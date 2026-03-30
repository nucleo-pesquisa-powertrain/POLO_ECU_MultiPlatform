/**
 * \file rte_environment.c
 * \brief RTE - Leitura e processamento dos sinais de ambiente do motor
 *
 * Responsavel por:
 *   - Ler os canais ADC de temperatura (ar, agua), pressao (MAP) e bateria
 *     via MCAL Adc_ReadChannel_mV()
 *   - Ler as entradas digitais de ignicao e fase via MCAL Dio_ReadChannel()
 *   - Aplicar curvas de calibracao e limites de saturacao
 *   - Expor os sinais processados atraves da interface Get16s/Get16u/Get8u
 *
 * Dependencias MCAL (plataforma-independentes):
 *   - Adc.h  : Adc_ReadChannel_mV(), canais ADC_CH_*
 *   - Dio.h  : Dio_ReadChannel(), canais DIO_CH_*
 *
 * Dependencias de aplicacao:
 *   - cdd_crankshaft.h     : CDD_Get_EngineSpeed_RAW()
 *   - ECU_State_interface.h: Get_ECU_State()
 *
 * Historico:
 *   2026-03-28  Refatorado: substituidas chamadas iLLD/HAL diretas
 *               (hal_adc_inputs.h, hal_discrete_inputs.h) pela MCAL API
 *               (Adc.h, Dio.h). Curvas de calibracao e formulas de conversao
 *               preservadas identicas ao original.
 */

#include "rte_environment.h"
#include "Mcal_Adc.h"
#include "Dio.h"
#include "Platform_Types.h"
#include "cdd_crankshaft.h"
#include "ECU_State_interface.h"

/* ------------------------------------------------------------------ */
/* Constantes internas                                                  */
/* ------------------------------------------------------------------ */

/** Fator de filtro passa-baixa para rotacao do motor (nao utilizado ativamente) */
#define ENGINE_SPEED_FILTER_FACTOR      0.1f

/* ------------------------------------------------------------------ */
/* Prototipos das funcoes de atualizacao internas (privadas ao modulo) */
/* ------------------------------------------------------------------ */

static void Update_RTE_AirTemperature(void);
static void Update_RTE_CoolantTemperature(void);
static void Update_RTE_ManAirPress(void);
static void Update_RTE_p_EthanolPercent(void);
static void Update_RTE_rpm_EngineSpeed(void);
static void Update_RTE_b_IgnitionOn(void);
static void Update_RTE_b_CamshaftState(void);
static void Update_RTE_V_BatteryCharge(void);

/* ------------------------------------------------------------------ */
/* Sinais do RTE (variaveis de estado internas)                        */
/* ------------------------------------------------------------------ */

volatile short int          S_RTE_T_CoolantTemp;    /* Temperatura do liquido [degC] */
volatile short int          S_RTE_K_AirTemp;        /* Temperatura do ar [K]         */
volatile short int          S_RTE_T_AirTemp;        /* Temperatura do ar [degC]      */
volatile unsigned short int S_RTE_P_MAP;            /* Pressao MAP [kPa]             */
volatile unsigned short int S_RTE_p_EthanolPercent; /* Percentual de etanol (0-1000) */
volatile unsigned short int S_RTE_rpm_EngineSpeed;  /* Rotacao do motor [RPM]        */
volatile unsigned short int S_RTE_V_BatteryCharge;  /* Tensao da bateria [mV]        */
volatile unsigned char      S_RTE_b_IgnitionOn;     /* Flag chave de ignicao         */
volatile unsigned char      S_RTE_b_CamshaftState;  /* Flag sensor de fase           */

/* ------------------------------------------------------------------ */
/* Funcao principal de atualizacao                                      */
/* ------------------------------------------------------------------ */

/**
 * \brief Atualiza todos os sinais de ambiente num unico ciclo de task.
 *
 * Chamada periodicamente (ex.: 10 ms) pela task de controle principal.
 * A ordem de execucao reflete a dependencia entre sinais: analogicos
 * primeiro, depois discretos.
 */
void Update_RTE_EnvironmentSignals(void)
{
    /* Sinais analogicos (nao booleanos) */
    Update_RTE_AirTemperature();
    Update_RTE_CoolantTemperature();
    Update_RTE_ManAirPress();
    Update_RTE_p_EthanolPercent();
    Update_RTE_rpm_EngineSpeed();
    Update_RTE_V_BatteryCharge();

    /* Sinais digitais (booleanos) */
    Update_RTE_b_IgnitionOn();
    Update_RTE_b_CamshaftState();
}

/* ------------------------------------------------------------------ */
/* Temperatura do ar de admissao                                        */
/* ------------------------------------------------------------------ */

/**
 * \brief Le o canal ADC_CH_AIR_TEMP e aplica a curva quadratica de calibracao.
 *
 * Curva: T[degC] = 2.827*V^2 - 43.44*V + 135.1
 * Faixa valida: 0.4 V .. 4.9 V
 * Saturacao: -10 degC .. 129.75 degC
 *
 * Adc_ReadChannel_mV() retorna milivolts (uint32); a divisao por 1000.0f
 * converte para volts, identico ao comportamento anterior de HAL_ADC_Get_V_AirTemp().
 */
static void Update_RTE_AirTemperature(void)
{
    float32 L_RTE_T_AirTemp;
    float32 voltage = ((float32) Adc_ReadChannel_mV(ADC_CH_AIR_TEMP)) / 1000.0f;

    /* Deteccao de falha por faixa de tensao (a tratar com diagnostico futuro) */
    if (voltage > 4.9f)
    {
        /* Tensao acima do esperado - possivel curto com 5V ou sensor aberto */
    }
    else if (voltage < 0.4f)
    {
        /* Tensao abaixo do esperado - possivel curto com GND ou sensor aberto */
    }

    /* Curva de calibracao quadratica (coeficientes fixos de calibracao de bancada) */
    L_RTE_T_AirTemp = ((2.827f * voltage * voltage) - (43.44f * voltage) + 135.1f);

    /* Saturacao dos limites fisicos do sensor */
    if (L_RTE_T_AirTemp > 129.75f)
    {
        L_RTE_T_AirTemp = 129.75f;
    }
    else if (L_RTE_T_AirTemp < -10.0f)
    {
        L_RTE_T_AirTemp = -10.0f;
    }

    S_RTE_T_AirTemp = (short int) L_RTE_T_AirTemp;
}

/**
 * \brief Retorna a temperatura do ar de admissao em Kelvin.
 *
 * Conversao: K = degC + 273
 */
short int Get16s_RTE_K_AirTemperature(void)
{
    S_RTE_K_AirTemp = S_RTE_T_AirTemp + 273;
    return S_RTE_K_AirTemp;
}

/**
 * \brief Retorna a temperatura do ar de admissao em graus Celsius.
 */
short int Get16s_RTE_T_AirTemperature(void)
{
    return S_RTE_T_AirTemp;
}

/* ------------------------------------------------------------------ */
/* Temperatura do liquido de arrefecimento                             */
/* ------------------------------------------------------------------ */

/**
 * \brief Le o canal ADC_CH_COOLANT_TEMP e aplica a curva linear de calibracao.
 *
 * Curva: T[degC] = -32.24*V + 133.43
 * Faixa valida: 0.333 V .. 4.4 V
 * Saturacao: -5.25 degC .. 143.25 degC
 *
 * Adc_ReadChannel_mV() retorna milivolts; divisao por 1000.0f converte
 * para volts, identico ao comportamento anterior de HAL_ADC_Get_V_CoolantTemp().
 */
static void Update_RTE_CoolantTemperature(void)
{
    float32 L_RTE_T_CoolantTemp;
    float32 voltage = ((float32) Adc_ReadChannel_mV(ADC_CH_COOLANT_TEMP)) / 1000.0f;

    /* Deteccao de falha por faixa de tensao */
    if (voltage > 4.4f)
    {
        /* Tensao acima do esperado */
    }
    else if (voltage < 0.333f)
    {
        /* Tensao abaixo do esperado */
    }

    /* Curva de calibracao linear */
    L_RTE_T_CoolantTemp = ((voltage * -32.24f) + 133.43f);

    /* Saturacao dos limites fisicos do sensor */
    if (L_RTE_T_CoolantTemp > 143.25f)
    {
        L_RTE_T_CoolantTemp = 143.25f;
    }
    else if (L_RTE_T_CoolantTemp < -5.25f)
    {
        L_RTE_T_CoolantTemp = -5.25f;
    }

    S_RTE_T_CoolantTemp = (short int) L_RTE_T_CoolantTemp;
}

/**
 * \brief Retorna a temperatura do liquido de arrefecimento em graus Celsius.
 */
short int Get16s_RTE_T_CoolantTemperature(void)
{
    return S_RTE_T_CoolantTemp;
}

/* ------------------------------------------------------------------ */
/* Pressao do coletor de admissao (MAP)                                */
/* ------------------------------------------------------------------ */

/**
 * \brief Le o canal ADC_CH_MAP e aplica a curva linear de calibracao.
 *
 * Curva: P[kPa] = 25.16*V - 3.87
 * Faixa valida: 0.5 V .. 4.5 V
 * Saturacao: 8.72 kPa .. 121.96 kPa
 *
 * Adc_ReadChannel_mV() retorna milivolts; divisao por 1000.0f converte
 * para volts, identico ao comportamento anterior de HAL_ADC_Get_V_MAP().
 */
static void Update_RTE_ManAirPress(void)
{
    float32 L_RTE_P_MAP;
    float32 voltage = ((float32) Adc_ReadChannel_mV(ADC_CH_MAP)) / 1000.0f;

    /* Deteccao de falha por faixa de tensao */
    if (voltage > 4.5f)
    {
        /* Tensao acima do esperado */
    }
    else if (voltage < 0.5f)
    {
        /* Tensao abaixo do esperado */
    }

    /* Curva de calibracao linear */
    L_RTE_P_MAP = ((voltage * 25.16f) - 3.87f);

    /* Saturacao dos limites fisicos do sensor */
    if (L_RTE_P_MAP < 8.72f)
    {
        L_RTE_P_MAP = 8.72f;
    }
    else if (L_RTE_P_MAP > 121.96f)
    {
        L_RTE_P_MAP = 121.96f;
    }

    S_RTE_P_MAP = (unsigned short int) L_RTE_P_MAP;
}

/**
 * \brief Retorna a pressao do coletor de admissao em kPa.
 */
unsigned short int Get16u_RTE_P_ManAirPress(void)
{
    return S_RTE_P_MAP;
}

/* ------------------------------------------------------------------ */
/* Percentual de etanol                                                */
/* ------------------------------------------------------------------ */

/**
 * \brief Atualiza o percentual de etanol no combustivel.
 *
 * Valor fixo de 275 (escala 0-1000, representa ~27.5%). A leitura do
 * sensor de etanol nao esta implementada nesta revisao.
 */
static void Update_RTE_p_EthanolPercent(void)
{
    /* TODO: implementar leitura do sensor de etanol quando disponivel */
    S_RTE_p_EthanolPercent = 275u;
}

/**
 * \brief Retorna o percentual de etanol (escala 0-1000, onde 1000 = 100%).
 */
unsigned short int Get16u_RTE_p_EthanolPercent(void)
{
    return S_RTE_p_EthanolPercent;
}

/* ------------------------------------------------------------------ */
/* Rotacao do motor                                                     */
/* ------------------------------------------------------------------ */

/**
 * \brief Atualiza a rotacao do motor lida pelo CDD do virabrequim.
 *
 * A filtragem passa-baixa (ENGINE_SPEED_FILTER_FACTOR) esta comentada
 * intencionalmente: o valor raw e' utilizado diretamente para evitar
 * atraso na malha de controle.
 */
static void Update_RTE_rpm_EngineSpeed(void)
{
    static unsigned short int L_RTE_rpm_EngineSpeed_prev = 0u;
    unsigned short int L_RTE_rpm_EngineSpeedFiltered;

    /* Filtro desabilitado - usando valor RAW diretamente */
    /* L_RTE_rpm_EngineSpeedFiltered = L_RTE_rpm_EngineSpeed_prev +
       ((L_RTE_rpm_EngineSpeed_prev - CDD_Get_EngineSpeed_RAW()) * ENGINE_SPEED_FILTER_FACTOR); */
    L_RTE_rpm_EngineSpeedFiltered = (unsigned short int) CDD_Get_EngineSpeed_RAW();
    L_RTE_rpm_EngineSpeed_prev    = L_RTE_rpm_EngineSpeedFiltered;
    S_RTE_rpm_EngineSpeed         = L_RTE_rpm_EngineSpeedFiltered;
}

/**
 * \brief Retorna a rotacao do motor em RPM.
 *
 * Nota: o nome 'EngineSpeeed' (tres 'e') e' preservado intencionalmente
 * para manter compatibilidade com o restante do codigo da aplicacao.
 */
unsigned short int Get16u_RTE_rpm_EngineSpeeed(void)
{
    return S_RTE_rpm_EngineSpeed;
}

/* ------------------------------------------------------------------ */
/* Tensao da bateria                                                    */
/* ------------------------------------------------------------------ */

/**
 * \brief Le o canal ADC_CH_VBATT e aplica o fator de escala do divisor resistivo.
 *
 * O sinal de bateria e' atenuado por um divisor resistivo antes do ADC.
 * O fator (14.0 / 5.0) reconstroi a tensao original da bateria (0..14 V)
 * a partir da tensao no ADC (0..5 V), mantendo a unidade em mV.
 *
 * Adc_ReadChannel_mV() retorna milivolts (uint32), identico ao
 * comportamento anterior de HAL_ADC_Get_V_Vbatt().
 */
static void Update_RTE_V_BatteryCharge(void)
{
    unsigned short int voltage_mV = (unsigned short int) Adc_ReadChannel_mV(ADC_CH_VBATT);

    /* Reconstroi a tensao da bateria aplicando o fator do divisor resistivo */
    S_RTE_V_BatteryCharge = (unsigned short int)(((float32) voltage_mV) * (14.0f / 5.0f));
}

/**
 * \brief Retorna a tensao da bateria em mV.
 */
unsigned short int Get16u_RTE_V_BatteryCharge(void)
{
    return S_RTE_V_BatteryCharge;
}

/* ------------------------------------------------------------------ */
/* Entradas digitais                                                    */
/* ------------------------------------------------------------------ */

/**
 * \brief Le o canal DIO_CH_IGNITION_ON via MCAL Dio_ReadChannel().
 *
 * Substitui HAL_DISCRETE_Get_IgnitionOn(). Retorna DIO_HIGH (1u) quando
 * a chave de ignicao esta acionada.
 */
static void Update_RTE_b_IgnitionOn(void)
{
    S_RTE_b_IgnitionOn = Dio_ReadChannel(DIO_CH_IGNITION_ON);
}

/**
 * \brief Retorna o estado da chave de ignicao: 1 = ligada, 0 = desligada.
 */
unsigned char Get8u_RTE_b_IgnitionOn(void)
{
    return S_RTE_b_IgnitionOn;
}

/**
 * \brief Le o canal DIO_CH_PHASE_STATE via MCAL Dio_ReadChannel().
 *
 * Substitui HAL_DISCRETE_Get_PhaseState(). Reflete o estado atual do
 * sensor de fase (arvore de cames) para determinacao do ciclo do motor.
 */
static void Update_RTE_b_CamshaftState(void)
{
    S_RTE_b_CamshaftState = Dio_ReadChannel(DIO_CH_PHASE_STATE);
}

/**
 * \brief Retorna o estado do sensor de fase: 1 = ativo, 0 = inativo.
 */
unsigned char Get8u_RTE_b_CamshaftState(void)
{
    return S_RTE_b_CamshaftState;
}

/* ------------------------------------------------------------------ */
/* Estado da ECU                                                        */
/* ------------------------------------------------------------------ */

/**
 * \brief Retorna o estado atual da ECU via interface de estado.
 */
ECU_State_t Get_RTE_ECU_State(void)
{
    return Get_ECU_State();
}
