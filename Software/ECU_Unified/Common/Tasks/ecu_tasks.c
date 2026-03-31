/**
 * \file ecu_tasks.c
 * \brief Corpos de tarefa unificados da ECU - implementacao portavel
 *
 * Este arquivo contem TODA a logica de aplicacao da ECU, extraida do
 * Cpu0_Main.c original do TC297B e adaptada para usar exclusivamente
 * a MCAL AUTOSAR. Nenhuma chamada iLLD ou STM32 HAL e' permitida aqui.
 *
 * Plataformas suportadas (via MCAL):
 *   - Infineon AURIX TC297B  (TC297B/MCAL/Src/)
 *   - STM32H745              (STM32H7/MCAL/Src/)
 *
 * O escalonamento das tarefas e' de responsabilidade da camada de
 * plataforma (Cpu0_Main.c para TC297B, main_ecu.c para STM32H7).
 *
 * \note Fluxo do crankshaft:
 *   Borda fisica -> ICU ISR -> CDD_SYNC_Timing_Event()
 *                           -> CDD_TriggerWheel_Event() (interno ao CDD_SYNC)
 *   O callback registrado via Icu_SetEdgeCallback() aponta para
 *   CDD_SYNC_Timing_Event, que ja encapsula CDD_TriggerWheel_Event
 *   internamente. Nao e necessario um wrapper externo.
 */

#include "ecu_tasks.h"

/* MCAL AUTOSAR */
#include "Platform_Types.h"
#include "Dio.h"
#include "Mcal_Adc.h"
#include "Icu.h"
#include "Gpt.h"

/* Camada de abstracao de sensores */
#include "EcuAbs_Sensors.h"

/* Complex Device Drivers */
#include "cdd_crankshaft.h"
#include "cdd_synchronism.h"
#include "cdd_spark.h"
#include "cdd_injectors.h"
#include "cdd_tbi.h"
#include "cdd_fuelpump.h"

/* XCP - Protocolo de calibracao/medicao */
#include "xcp_can_if.h"
#include "XcpBasic.h"

/* Interfaces de aplicacao ASW */
#include "fuel_interface.h"
#include "mngmt_interface.h"
#include "spark_interface.h"
#include "tbi_interface.h"

/* Task0 gerada pelo ASCET (logica de aplicacao 10 ms) */
extern void Task0_Run(void);

/* ================================================================== */
/* Variaveis de debug para XCP/INCA (visibilidade global intencional)  */
/* ================================================================== */

/** Toggle a 5 ms - verificar presenca da tarefa via INCA/XCP */
volatile uint8 dbg_Task5ms_toggle  = 0u;

/** Toggle a 20 ms - verificar presenca da tarefa via INCA/XCP */
volatile uint8 dbg_Task20ms_toggle = 0u;

/** Toggle a 100 ms - verificar presenca da tarefa via INCA/XCP */
volatile uint8 dbg_Task100ms_toggle = 0u;

/* ================================================================== */
/* Variaveis do background - filtro TPS                                */
/* ================================================================== */

/**
 * Maior numero de amostras ADC registrado num janelamento de 10 ms.
 * Util para verificar taxa de amostragem real (XCP/INCA).
 */
static uint32 maior_adc_counter = 0u;

/** Contador de amostras na janela atual de 10 ms */
static unsigned long tps_sample_count = 0UL;

/** Soma acumulada das amostras da janela atual */
static unsigned long tps_sample_sum   = 0UL;

/**
 * Valor filtrado do TPS (media da janela de 10 ms).
 * Unidade: milivolts. Exportado para rte_components.c e tbi_calibration.c.
 */
unsigned long tps_filtered_value = 0UL;

/** Ultima leitura bruta do ADC do TPS (mV) - para debug */
static uint32 tps_raw_mv = 0u;

/* ================================================================== */
/* Animacoes de LED (non-blocking, chamadas a cada 100 ms)             */
/* ================================================================== */

/** Bounce/Knight Rider: 1->2->3->4->3->2 (chave OFF, sem rotacao) */
static void Anim_Bounce_Step(void)
{
    static uint8 step = 0u;

    /* Apaga todos */
    Dio_WriteChannel(DIO_CH_LED_HIGH, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED_MID,  DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED_LOW,  DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED_GND,  DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED1, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED2, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED3, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED4, DIO_LOW);

    switch (step)
    {
        case 0u: Dio_WriteChannel(DIO_CH_LED_HIGH, DIO_HIGH); Dio_WriteChannel(DIO_CH_LED1, DIO_HIGH); break;
        case 1u: Dio_WriteChannel(DIO_CH_LED_MID,  DIO_HIGH); Dio_WriteChannel(DIO_CH_LED2, DIO_HIGH); break;
        case 2u: Dio_WriteChannel(DIO_CH_LED_LOW,  DIO_HIGH); Dio_WriteChannel(DIO_CH_LED3, DIO_HIGH); break;
        case 3u: Dio_WriteChannel(DIO_CH_LED_GND,  DIO_HIGH); Dio_WriteChannel(DIO_CH_LED4, DIO_HIGH); break;
        case 4u: Dio_WriteChannel(DIO_CH_LED_LOW,  DIO_HIGH); Dio_WriteChannel(DIO_CH_LED3, DIO_HIGH); break;
        case 5u: Dio_WriteChannel(DIO_CH_LED_MID,  DIO_HIGH); Dio_WriteChannel(DIO_CH_LED2, DIO_HIGH); break;
        default: step = 0u; return;
    }
    step = (step + 1u >= 6u) ? 0u : step + 1u;
}

/** Pares alternados: 1+3 <-> 2+4 (chave ON, sem rotacao) */
static void Anim_AlternatePairs_Step(void)
{
    static uint8 toggle = 0u;

    /* Apaga todos */
    Dio_WriteChannel(DIO_CH_LED_HIGH, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED_MID,  DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED_LOW,  DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED_GND,  DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED1, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED2, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED3, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED4, DIO_LOW);

    if (toggle != 0u)
    {
        Dio_WriteChannel(DIO_CH_LED_HIGH, DIO_HIGH);
        Dio_WriteChannel(DIO_CH_LED1, DIO_HIGH);
        Dio_WriteChannel(DIO_CH_LED_LOW,  DIO_HIGH);
        Dio_WriteChannel(DIO_CH_LED3, DIO_HIGH);
    }
    else
    {
        Dio_WriteChannel(DIO_CH_LED_MID,  DIO_HIGH);
        Dio_WriteChannel(DIO_CH_LED2, DIO_HIGH);
        Dio_WriteChannel(DIO_CH_LED_GND,  DIO_HIGH);
        Dio_WriteChannel(DIO_CH_LED4, DIO_HIGH);
    }
    toggle ^= 1u;
}

/* ================================================================== */
/* EcuTask_Init                                                         */
/* ================================================================== */

void EcuTask_Init(void)
{
    /* -------------------------------------------------------------- */
    /* 1. Inicializacao MCAL                                           */
    /* Ordem importa: DIO deve estar pronto antes dos CDDs que o usam  */
    /* -------------------------------------------------------------- */
    Dio_Init();
    Adc_Init();
    Icu_Init();
    Gpt_Init();

    /* Camada de abstracao de sensores (sobre MCAL Adc/Dio/CDD) */
    EcuAbs_Init();

    /* -------------------------------------------------------------- */
    /* 2. Complex Device Drivers                                       */
    /* -------------------------------------------------------------- */

    /* Sensor de rotacao (roda dentada 60-2) */
    CDD_CrankshaftPosition_Init();

    /* Injetores de combustivel + driver MC33810 via SPI */
    CDD_INJ_Init();

    /* Borboleta eletronica - driver MC33186 via PWM + DIO */
    CDD_TBI_Init();

    /* Bobinas de ignicao: estado inicial (OFF), sem Init separado      */
    /* CDD_SPARK_Init() e chamado internamente se necessario            */
    /* O Dio_Init() ja garante os pinos das bobinas em nivel seguro     */

    /* -------------------------------------------------------------- */
    /* 3. Protocolo XCP sobre CAN                                      */
    /* Sequencia obrigatoria: XcpCanIf_Init antes de XcpInit           */
    /* -------------------------------------------------------------- */
    XcpCanIf_Init();
    XcpInit();

    /* -------------------------------------------------------------- */
    /* 4. Registro de callbacks GPT (timers one-shot de ignicao/injecao)*/
    /*                                                                  */
    /* GPT_CH_IGN_TIMING  -> inicia dwell da bobina no angulo calculado */
    /* GPT_CH_INJ_DURATION-> encerra pulso de injecao                  */
    /* GPT_CH_INJ_TIMING  -> inicia pulso de injecao no angulo calculado*/
    /* GPT_CH_DWELL       -> dispara a faısca (descarrega bobina)       */
    /* -------------------------------------------------------------- */
    Gpt_SetNotification(GPT_CH_IGN_TIMING,   CDD_SYNC_StartIgnition_Event);
    Gpt_SetNotification(GPT_CH_INJ_DURATION, CDD_INJ_StopFuelInjEvent);
    Gpt_SetNotification(GPT_CH_INJ_TIMING,   CDD_SYNC_StartInjection_Event);
    Gpt_SetNotification(GPT_CH_DWELL,        CDD_SPARK_SparkEvent);

    /* -------------------------------------------------------------- */
    /* 5. Callback ICU: borda do crankshaft                            */
    /*                                                                  */
    /* CDD_SYNC_Timing_Event e o ponto de entrada publico. Ele chama   */
    /* CDD_TriggerWheel_Event internamente para atualizar o modelo da  */
    /* roda dentada antes de calcular os angulos de sincronismo.        */
    /* Nao e necessario um wrapper externo.                             */
    /* -------------------------------------------------------------- */
    Icu_SetEdgeCallback(CDD_SYNC_Timing_Event);

    /* Habilita deteccao de borda - a partir daqui o motor pode rodar  */
    Icu_EnableEdgeDetection();
}

/* ================================================================== */
/* EcuTask_5ms                                                          */
/* ================================================================== */

void EcuTask_5ms(void)
{
    /* Toggle de presenca da tarefa - visivel via XCP/INCA */
    dbg_Task5ms_toggle ^= 1u;

    /* LED4: espelha estado do sensor de fase do comando de valvulas */
    if (EcuAbs_GetPhaseState() != 0u)
    {
        Dio_WriteChannel(DIO_CH_LED4, DIO_HIGH);
    }
    else
    {
        Dio_WriteChannel(DIO_CH_LED4, DIO_LOW);
    }
}

/* ================================================================== */
/* EcuTask_10ms                                                         */
/* ================================================================== */

void EcuTask_10ms(void)
{
    /* Atualiza leituras dos sensores via EcuAbs (antes do RTE/ASW) */
    EcuAbs_Update();

    /* Logica de aplicacao ASCET: XCP, RTE, ECU_State, TBI, THROTTLE   */
    Task0_Run();

    /* Calculo de tempo de injecao (Simulink/ASCET Fuel Control)        */
    FUEL_MainTask10ms();

    /* Background do protocolo XCP (DAQ, calibracao) */
    XcpBackground();
}

/* ================================================================== */
/* EcuTask_20ms                                                         */
/* ================================================================== */

void EcuTask_20ms(void)
{
    /* Toggle de presenca da tarefa - visivel via XCP/INCA */
    dbg_Task20ms_toggle ^= 1u;

    /* Gerenciamento do motor (estado, ventiladores, bomba, etc.) */
    MNGT_MainTask20ms();

    /* Calculo do avanco de ignicao */
    SPARK_MainTask20ms();
}

/* ================================================================== */
/* EcuTask_100ms                                                        */
/* ================================================================== */

void EcuTask_100ms(void)
{
    /* Toggle de presenca da tarefa - visivel via XCP/INCA */
    dbg_Task100ms_toggle ^= 1u;

    /* -------------------------------------------------------------- */
    /* Animacao dos LEDs quando motor parado (RPM = 0)                 */
    /*                                                                  */
    /*   Chave OFF (IGNITION_ON = 0): Bounce Knight Rider              */
    /*   Chave ON  (IGNITION_ON = 1): Pares alternados                 */
    /*   Motor rodando: LEDs controlados pelos CDDs (injecao/ignicao)  */
    /* -------------------------------------------------------------- */
    if (EcuAbs_GetEngineSpeed_rpm() == 0u)
    {
        if (EcuAbs_GetIgnitionOn() != 0u)
        {
            Anim_AlternatePairs_Step();
        }
        else
        {
            Anim_Bounce_Step();
        }
    }

    /* -------------------------------------------------------------- */
    /* Hook de atualizacao do display (somente TC297B)                 */
    /* Em outras plataformas a versao fraca (sem operacao) e usada     */
    /* -------------------------------------------------------------- */
    EcuTask_Hook_DisplayUpdate();
}

/* ================================================================== */
/* EcuTask_Background                                                   */
/* ================================================================== */

void EcuTask_Background(void)
{
    /*
     * Timestamps em us via MCAL Icu_GetTimestamp_us().
     * Substitui o par elapsed()/now() + IfxStm_getFrequency() do original.
     *
     * O timer ICU free-running (32-bit) transborda a cada ~4295 s;
     * a subtracao com uint32 trata o rollover corretamente desde que
     * o intervalo medido seja menor que o periodo de transbordamento.
     */
    static uint32 last_ts_10ms = 0u;
    static uint32 last_ts_adc  = 0u;
    static uint8  first_run    = 1u;

    if (first_run != 0u)
    {
        last_ts_10ms = Icu_GetTimestamp_us();
        last_ts_adc  = Icu_GetTimestamp_us();
        first_run    = 0u;
    }

    uint32 now_us = Icu_GetTimestamp_us();

    /* -------------------------------------------------------------- */
    /* Janela de 10 ms: calcula media das amostras acumuladas           */
    /* -------------------------------------------------------------- */
    if ((now_us - last_ts_10ms) >= 10000u)
    {
        last_ts_10ms = now_us;

        if (tps_sample_count > 0UL)
        {
            tps_filtered_value = tps_sample_sum / tps_sample_count;
        }
        else
        {
            tps_filtered_value = 0UL;
        }

        /* Registra pico de taxa de amostragem para diagnostico */
        if (tps_sample_count > (unsigned long)maior_adc_counter)
        {
            maior_adc_counter = (uint32)tps_sample_count;
        }

        /* Reinicia acumuladores para a proxima janela */
        tps_sample_count = 0UL;
        tps_sample_sum   = 0UL;
    }

    /* -------------------------------------------------------------- */
    /* Amostragem ADC a cada 200 us                                    */
    /* Substitui HAL_ADC_Get_V_TBIPositionRed() por Adc_ReadChannel_mV */
    /* -------------------------------------------------------------- */
    if ((now_us - last_ts_adc) >= 200u)
    {
        last_ts_adc = now_us;

        tps_raw_mv = Adc_ReadChannel_mV(ADC_CH_TBI_POS_RED);
        tps_sample_sum += (unsigned long)tps_raw_mv;
        tps_sample_count++;
    }
}

/* ================================================================== */
/* EcuTask_Hook_DisplayUpdate - implementacao fraca (default)          */
/* ================================================================== */

/**
 * Implementacao fraca: sem operacao em plataformas sem display TFT.
 * O TC297B sobrescreve esta funcao em Cpu0_Main.c chamando ECU_Pages_Update().
 *
 * O atributo __attribute__((weak)) e reconhecido pelo GCC/Clang/TASKING.
 * Para compiladores sem suporte, use #pragma weak ou deixe sem atributo
 * e garanta que a plataforma forneca a implementacao via objeto externo.
 */
__attribute__((weak)) void EcuTask_Hook_DisplayUpdate(void)
{
    /* Sem display: nenhuma operacao necessaria */
}
