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
 * Dependencias:
 *   - EcuAbs_Sensors.h     : camada de abstracao de sensores (EcuAbs_Get*())
 *   - Dio.h                : Dio_ReadChannel(), canais DIO_CH_*
 *   - ECU_State_interface.h: Get_ECU_State()
 *
 * Historico:
 *   2026-03-28  Refatorado: substituidas chamadas iLLD/HAL diretas
 *               (hal_adc_inputs.h, hal_discrete_inputs.h) pela MCAL API
 *               (Adc.h, Dio.h). Curvas de calibracao e formulas de conversao
 *               preservadas identicas ao original.
 *   2026-03-30  Refatorado: Update_* delegam para EcuAbs_Sensors (camada
 *               de abstracao). Curvas de calibracao movidas para EcuAbs.
 */

#include "rte_environment.h"
#include "EcuAbs_Sensors.h"
#include "Dio.h"
#include "Platform_Types.h"
#include "ECU_State_interface.h"

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
 * \brief Obtem a temperatura do ar de admissao via EcuAbs [degC].
 *
 * A curva de calibracao e saturacao sao aplicadas internamente pelo EcuAbs.
 */
static void Update_RTE_AirTemperature(void)
{
    S_RTE_T_AirTemp = EcuAbs_GetAirTemp_degC();
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
 * \brief Obtem a temperatura do liquido de arrefecimento via EcuAbs [degC].
 *
 * A curva de calibracao e saturacao sao aplicadas internamente pelo EcuAbs.
 */
static void Update_RTE_CoolantTemperature(void)
{
    S_RTE_T_CoolantTemp = EcuAbs_GetCoolantTemp_degC();
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
 * \brief Obtem a pressao do coletor de admissao via EcuAbs [kPa].
 *
 * A curva de calibracao e saturacao sao aplicadas internamente pelo EcuAbs.
 */
static void Update_RTE_ManAirPress(void)
{
    S_RTE_P_MAP = EcuAbs_GetMAP_kPa();
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
 * \brief Obtem o percentual de etanol no combustivel via EcuAbs.
 *
 * Escala 0-1000, onde 1000 = 100%.
 */
static void Update_RTE_p_EthanolPercent(void)
{
    S_RTE_p_EthanolPercent = EcuAbs_GetEthanolPercent();
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
 * \brief Obtem a rotacao do motor via EcuAbs [RPM].
 */
static void Update_RTE_rpm_EngineSpeed(void)
{
    S_RTE_rpm_EngineSpeed = EcuAbs_GetEngineSpeed_rpm();
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
 * \brief Obtem a tensao da bateria via EcuAbs [mV].
 *
 * O fator do divisor resistivo e aplicado internamente pelo EcuAbs.
 */
static void Update_RTE_V_BatteryCharge(void)
{
    S_RTE_V_BatteryCharge = (unsigned short int)EcuAbs_GetVbatt_mV();
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
