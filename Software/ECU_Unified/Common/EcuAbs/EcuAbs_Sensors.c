/**
 * \file EcuAbs_Sensors.c
 * \brief Implementacao da camada de abstracao de sensores da ECU
 *
 * Le os sensores via MCAL (Adc, Dio) e CDD (crankshaft), aplica curvas
 * de calibracao e armazena os valores em variaveis estaticas. As funcoes
 * getter retornam os valores cacheados sem acesso ao hardware.
 *
 * Ciclo de operacao:
 *   1. EcuAbs_Init()   - chamada uma vez na inicializacao
 *   2. EcuAbs_Update() - chamada a cada 10 ms pela task principal
 *   3. EcuAbs_Get*()   - chamadas pelo RTE/ASW a qualquer momento
 *
 * Curvas de calibracao extraidas de rte_environment.c (coeficientes de
 * bancada - NAO alterar sem recalibracao).
 *
 * Dependencias:
 *   - Mcal_Adc.h       : Adc_ReadChannel_mV(), canais ADC_CH_*
 *   - Dio.h            : Dio_ReadChannel(), canais DIO_CH_*
 *   - cdd_crankshaft.h : CDD_Get_EngineSpeed_RAW()
 */

#include "EcuAbs_Sensors.h"
#include "Mcal_Adc.h"
#include "Dio.h"
#include "cdd_crankshaft.h"

/* ------------------------------------------------------------------ */
/* Variavel externa: TPS filtrado pela task de background              */
/* ------------------------------------------------------------------ */

extern unsigned long tps_filtered_value;

/* ------------------------------------------------------------------ */
/* Variaveis estaticas - valores cacheados dos sensores                */
/* ------------------------------------------------------------------ */

static sint16  s_airTemp_degC       = 0;
static sint16  s_coolantTemp_degC   = 0;
static uint16  s_map_kPa            = 0u;
static uint16  s_vbatt_mV           = 0u;
static uint16  s_pedalPos_pct       = 0u;
static uint16  s_tps_raw            = 0u;
static uint16  s_engineSpeed_rpm    = 0u;
static uint16  s_ethanolPercent     = 0u;
static uint8   s_phaseState         = 0u;
static uint8   s_ignitionOn         = 0u;

/* ------------------------------------------------------------------ */
/* Prototipos das funcoes internas de atualizacao                      */
/* ------------------------------------------------------------------ */

static void EcuAbs_UpdateAirTemp(void);
static void EcuAbs_UpdateCoolantTemp(void);
static void EcuAbs_UpdateMAP(void);
static void EcuAbs_UpdateVbatt(void);
static void EcuAbs_UpdatePedalPos(void);
static void EcuAbs_UpdateTPS(void);
static void EcuAbs_UpdateEngineSpeed(void);
static void EcuAbs_UpdateEthanolPercent(void);
static void EcuAbs_UpdatePhaseState(void);
static void EcuAbs_UpdateIgnitionOn(void);

/* ================================================================== */
/* Funcoes de atualizacao internas (privadas ao modulo)                */
/* ================================================================== */

/* ------------------------------------------------------------------ */
/* Temperatura do ar de admissao                                       */
/* Curva: T[degC] = 2.827 * V^2 - 43.44 * V + 135.1                  */
/* Saturacao: -10 .. 129.75 degC                                       */
/* ------------------------------------------------------------------ */
static void EcuAbs_UpdateAirTemp(void)
{
    float32 voltage = ((float32)Adc_ReadChannel_mV(ADC_CH_AIR_TEMP)) / 1000.0f;
    float32 temp    = (2.827f * voltage * voltage) - (43.44f * voltage) + 135.1f;

    if (temp > 129.75f)
    {
        temp = 129.75f;
    }
    else if (temp < -10.0f)
    {
        temp = -10.0f;
    }

    s_airTemp_degC = (sint16)temp;
}

/* ------------------------------------------------------------------ */
/* Temperatura do liquido de arrefecimento                             */
/* Curva: T[degC] = -32.24 * V + 133.43                               */
/* Saturacao: -5.25 .. 143.25 degC                                     */
/* ------------------------------------------------------------------ */
static void EcuAbs_UpdateCoolantTemp(void)
{
    float32 voltage = ((float32)Adc_ReadChannel_mV(ADC_CH_COOLANT_TEMP)) / 1000.0f;
    float32 temp    = (voltage * -32.24f) + 133.43f;

    if (temp > 143.25f)
    {
        temp = 143.25f;
    }
    else if (temp < -5.25f)
    {
        temp = -5.25f;
    }

    s_coolantTemp_degC = (sint16)temp;
}

/* ------------------------------------------------------------------ */
/* Pressao do coletor de admissao (MAP)                                */
/* Curva: P[kPa] = 25.16 * V - 3.87                                   */
/* Saturacao: 8.72 .. 121.96 kPa                                       */
/* ------------------------------------------------------------------ */
static void EcuAbs_UpdateMAP(void)
{
    float32 voltage = ((float32)Adc_ReadChannel_mV(ADC_CH_MAP)) / 1000.0f;
    float32 press   = (voltage * 25.16f) - 3.87f;

    if (press < 8.72f)
    {
        press = 8.72f;
    }
    else if (press > 121.96f)
    {
        press = 121.96f;
    }

    s_map_kPa = (uint16)press;
}

/* ------------------------------------------------------------------ */
/* Tensao da bateria                                                   */
/* Fator do divisor resistivo: Vbat = Vadc * 14 / 5                    */
/* ------------------------------------------------------------------ */
static void EcuAbs_UpdateVbatt(void)
{
    uint32 mV = Adc_ReadChannel_mV(ADC_CH_VBATT);

    s_vbatt_mV = (uint16)(((float32)mV) * (14.0f / 5.0f));
}

/* ------------------------------------------------------------------ */
/* Posicao do pedal do acelerador                                      */
/* Normalizacao: adc8 = mV * 255 / 5000                                */
/* Linearizacao: pct = (adc8 - 39) * 100 / (191 - 39)                 */
/* Saturacao: 0 .. 100 %                                               */
/* ------------------------------------------------------------------ */
static void EcuAbs_UpdatePedalPos(void)
{
    uint32  mV   = Adc_ReadChannel_mV(ADC_CH_PEDAL);
    uint32  adc8 = mV * 255u / 5000u;
    sint32  pct  = ((sint32)adc8 - 39) * 100 / (191 - 39);

    if (pct < 0)
    {
        pct = 0;
    }
    else if (pct > 100)
    {
        pct = 100;
    }

    s_pedalPos_pct = (uint16)pct;
}

/* ------------------------------------------------------------------ */
/* Posicao da borboleta (TPS) - valor filtrado pela task de background */
/* Retorna o valor raw em mV (ja filtrado externamente)                */
/* ------------------------------------------------------------------ */
static void EcuAbs_UpdateTPS(void)
{
    s_tps_raw = (uint16)tps_filtered_value;
}

/* ------------------------------------------------------------------ */
/* Rotacao do motor - leitura direta do CDD do virabrequim             */
/* ------------------------------------------------------------------ */
static void EcuAbs_UpdateEngineSpeed(void)
{
    s_engineSpeed_rpm = (uint16)CDD_Get_EngineSpeed_RAW();
}

/* ------------------------------------------------------------------ */
/* Percentual de etanol - valor fixo (sensor nao implementado)         */
/* ------------------------------------------------------------------ */
static void EcuAbs_UpdateEthanolPercent(void)
{
    /* TODO: implementar leitura do sensor de etanol quando disponivel */
    s_ethanolPercent = 275u;
}

/* ------------------------------------------------------------------ */
/* Estado do sensor de fase (arvore de cames)                          */
/* ------------------------------------------------------------------ */
static void EcuAbs_UpdatePhaseState(void)
{
    s_phaseState = (uint8)(Dio_ReadChannel(DIO_CH_PHASE_STATE) != 0u);
}

/* ------------------------------------------------------------------ */
/* Estado da chave de ignicao                                          */
/* ------------------------------------------------------------------ */
static void EcuAbs_UpdateIgnitionOn(void)
{
    s_ignitionOn = (uint8)(Dio_ReadChannel(DIO_CH_IGNITION_ON) != 0u);
}

/* ================================================================== */
/* API publica - Inicializacao e atualizacao ciclica                   */
/* ================================================================== */

/**
 * \brief Inicializa o modulo EcuAbs_Sensors.
 *
 * Atualmente vazio: os perifericos (ADC, DIO) sao inicializados pelo
 * MCAL antes desta chamada. Reservado para futuras calibracoes ou
 * configuracoes especificas da camada de abstracao.
 */
void EcuAbs_Init(void)
{
    /* Sensores inicializados pelo MCAL - nada a fazer aqui */
}

/**
 * \brief Atualiza todos os sensores num unico ciclo.
 *
 * Deve ser chamada a cada 10 ms pela task principal, ANTES do ciclo
 * do RTE. Garante que todos os getters retornam valores coerentes
 * de um mesmo instante de amostragem.
 */
void EcuAbs_Update(void)
{
    /* Sinais analogicos */
    EcuAbs_UpdateAirTemp();
    EcuAbs_UpdateCoolantTemp();
    EcuAbs_UpdateMAP();
    EcuAbs_UpdateVbatt();
    EcuAbs_UpdatePedalPos();
    EcuAbs_UpdateTPS();

    /* Sinais do motor */
    EcuAbs_UpdateEngineSpeed();
    EcuAbs_UpdateEthanolPercent();

    /* Sinais discretos */
    EcuAbs_UpdatePhaseState();
    EcuAbs_UpdateIgnitionOn();
}

/* ================================================================== */
/* API publica - Getters (retornam valores cacheados)                  */
/* ================================================================== */

sint16 EcuAbs_GetAirTemp_degC(void)
{
    return s_airTemp_degC;
}

sint16 EcuAbs_GetCoolantTemp_degC(void)
{
    return s_coolantTemp_degC;
}

uint16 EcuAbs_GetMAP_kPa(void)
{
    return s_map_kPa;
}

uint16 EcuAbs_GetVbatt_mV(void)
{
    return s_vbatt_mV;
}

uint16 EcuAbs_GetPedalPos_pct(void)
{
    return s_pedalPos_pct;
}

uint16 EcuAbs_GetTPS_raw(void)
{
    return s_tps_raw;
}

uint16 EcuAbs_GetEngineSpeed_rpm(void)
{
    return s_engineSpeed_rpm;
}

uint16 EcuAbs_GetEthanolPercent(void)
{
    return s_ethanolPercent;
}

uint8 EcuAbs_GetPhaseState(void)
{
    return s_phaseState;
}

uint8 EcuAbs_GetIgnitionOn(void)
{
    return s_ignitionOn;
}
