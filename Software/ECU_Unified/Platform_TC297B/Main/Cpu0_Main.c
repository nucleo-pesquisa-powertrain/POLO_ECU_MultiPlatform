/**
 * \file Cpu0_Main.c
 * \brief Main da plataforma TC297B - wrapper fino sobre ecu_tasks.c
 *
 * Este arquivo e especifico da plataforma Infineon AURIX TC297B.
 * Responsabilidades DESTE arquivo:
 *   - Sincronizacao de CPUs (IfxCpu_syncEvent)
 *   - Desabilitar watchdogs durante desenvolvimento
 *   - Configuracao do tick de 1 ms via STM0 (iLLD permitido aqui)
 *   - Animacao de startup nos LEDs (opcional, estetica)
 *   - Inicializacao do display TFT (opcional, plataforma-especifica)
 *   - Loop principal: polling de flags e chamada das EcuTask_*ms()
 *   - Loop de background: EcuTask_Background() + XcpCanIf_Handler()
 *
 * O que NAO pertence a este arquivo:
 *   - Logica de aplicacao (esta em ecu_tasks.c)
 *   - Chamadas iLLD para perifericos de aplicacao (GPIO app, ADC app, GPT app)
 *   - Qualquer referencia a injetores, bobinas, TPS, CAN XCP
 *
 * iLLD permitidos APENAS neste arquivo:
 *   - IfxCpu_* (sincronizacao)
 *   - IfxScuWdt_* (watchdog)
 *   - IfxStm_* (tick timer STM0 - init + ISR)
 *   - IfxPort_setPinMode/High/Low para os LEDs no startup e animacoes
 *
 * \copyright Copyright (c) 2017 Infineon Technologies AG. All rights reserved.
 */

/* ------------------------------------------------------------------ */
/* Includes iLLD (uso restrito a init de plataforma e STM tick)        */
/* ------------------------------------------------------------------ */
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "IfxStm.h"
#include "IfxPort.h"
#include "Bsp.h"
#include "hal_discrete_common.h"  /* LED_HIGH, LED_1, etc. para animacao de startup */

/* ------------------------------------------------------------------ */
/* Display TFT (especifico TC297B, opcional)                           */
/* ------------------------------------------------------------------ */
#include "tfthw.h"
#include "tft_touch.h"
#include "ecu_pages.h"

/* ------------------------------------------------------------------ */
/* MCAL e tarefas unificadas                                            */
/* ------------------------------------------------------------------ */
#include "Dio.h"          /* Para mapeamento de DIO_CH_LED* */
#include "xcp_can_if.h"   /* XcpCanIf_Handler() no loop principal */
#include "ecu_tasks.h"    /* EcuTask_Init / _5ms / _10ms / etc. */

/* ================================================================== */
/* Sincronizacao de CPUs (core0 aguarda core1 se necessario)           */
/* ================================================================== */
IfxCpu_syncEvent cpuSyncEvent = 0;

/* ================================================================== */
/* Stubs OSEK (RTAOS removido, substituido por bare-metal)             */
/* ================================================================== */
void SuspendAllInterrupts(void) { __disable(); }
void ResumeAllInterrupts(void)  { __enable(); }

/* ================================================================== */
/* Configuracao do tick STM                                             */
/* ================================================================== */

/** Periodo do tick base em milissegundos */
#define STM_TICK_MS         1u

/** Prioridade da ISR do STM0 */
#define STM_ISR_PRIORITY    10u

/* ================================================================== */
/* Flags de tarefas (setadas pela ISR, limpas pelo loop principal)     */
/* ================================================================== */
static volatile uint32 tick_1ms   = 0u;
static volatile uint8  flag_5ms   = 0u;
static volatile uint8  flag_10ms  = 0u;
static volatile uint8  flag_20ms  = 0u;
static volatile uint8  flag_100ms = 0u;

/* ================================================================== */
/* ISR do STM0: tick de 1 ms                                           */
/*                                                                      */
/* Unico uso de iLLD no caminho de execucao periodico. Reprograma o    */
/* comparador do STM para o proximo periodo e seta as flags de tarefa.  */
/* ================================================================== */
IFX_INTERRUPT(STM_TickISR, 0, STM_ISR_PRIORITY)
{
    /* Reprograma comparador para daqui a 1 ms */
    uint32 stmTicks = (uint32)(0.001f * IfxStm_getFrequency(&MODULE_STM0));
    IfxStm_updateCompare(&MODULE_STM0, IfxStm_Comparator_0,
        IfxStm_getCompare(&MODULE_STM0, IfxStm_Comparator_0) + stmTicks);

    tick_1ms++;

    /* Contadores estaticos: sem custo de init, sem acesso externo */
    static uint32 cnt_5ms   = 0u;
    static uint32 cnt_10ms  = 0u;
    static uint32 cnt_20ms  = 0u;
    static uint32 cnt_100ms = 0u;

    cnt_5ms++;
    cnt_10ms++;
    cnt_20ms++;
    cnt_100ms++;

    if (cnt_5ms   >= 5u)   { cnt_5ms   = 0u; flag_5ms   = 1u; }
    if (cnt_10ms  >= 10u)  { cnt_10ms  = 0u; flag_10ms  = 1u; }
    if (cnt_20ms  >= 20u)  { cnt_20ms  = 0u; flag_20ms  = 1u; }
    if (cnt_100ms >= 100u) { cnt_100ms = 0u; flag_100ms = 1u; }
}

/* ================================================================== */
/* initStmTick: configura e arma o STM0 para interrupcao periodica     */
/* ================================================================== */
static void initStmTick(void)
{
    IfxStm_CompareConfig stmCfg;

    /* Suspende contagem durante depuracao (OCDS) */
    IfxStm_enableOcdsSuspend(&MODULE_STM0);

    IfxStm_initCompareConfig(&stmCfg);

    /* Primeira interrupcao rapida para verificar se o STM esta rodando */
    stmCfg.ticks           = 100u;
    stmCfg.triggerPriority = STM_ISR_PRIORITY;
    stmCfg.typeOfService    = IfxSrc_Tos_cpu0;

    IfxStm_initCompare(&MODULE_STM0, &stmCfg);
}

/* ================================================================== */
/* Animacoes de LED no startup (estetica, opcional)                    */
/*                                                                      */
/* IfxPort usado SOMENTE aqui no startup bloqueante pre-MCAL.          */
/* Apos EcuTask_Init() todos os LEDs sao acessados via Dio_WriteChannel */
/* ================================================================== */

/** Apaga todos os LEDs da placa de aplicacao e do conector X102 */
static void LEDs_AllOff(void)
{
    /* LEDs da Application Kit (P13.x, anodo comum - acende com LOW) */
    IfxPort_setPinHigh(LED_HIGH);
    IfxPort_setPinHigh(LED_MID);
    IfxPort_setPinHigh(LED_LOW);
    IfxPort_setPinHigh(LED_GND);

    /* LEDs customizados no conector X102 (anodo no pino - acende com HIGH) */
    IfxPort_setPinLow(LED_1);
    IfxPort_setPinLow(LED_2);
    IfxPort_setPinLow(LED_3);
    IfxPort_setPinLow(LED_4);
}

/**
 * Animacao "Knight Rider / bounce": 1->2->3->4->3->2
 * Chamada a cada 100 ms no modo chave desligada.
 * Usa Dio_WriteChannel para compatibilidade com MCAL apos init.
 */
static void Anim_Bounce_Step(void)
{
    static uint8 step = 0u;

    /* Apaga tudo antes de acender o LED do passo atual */
    Dio_WriteChannel(DIO_CH_LED_HIGH, DIO_HIGH); /* anodo comum: HIGH = apagado */
    Dio_WriteChannel(DIO_CH_LED_MID,  DIO_HIGH);
    Dio_WriteChannel(DIO_CH_LED_LOW,  DIO_HIGH);
    Dio_WriteChannel(DIO_CH_LED_GND,  DIO_HIGH);
    Dio_WriteChannel(DIO_CH_LED1, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED2, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED3, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED4, DIO_LOW);

    switch (step)
    {
        case 0u:
            Dio_WriteChannel(DIO_CH_LED_HIGH, DIO_LOW);
            Dio_WriteChannel(DIO_CH_LED1,     DIO_HIGH);
            break;
        case 1u:
            Dio_WriteChannel(DIO_CH_LED_MID,  DIO_LOW);
            Dio_WriteChannel(DIO_CH_LED2,     DIO_HIGH);
            break;
        case 2u:
            Dio_WriteChannel(DIO_CH_LED_LOW,  DIO_LOW);
            Dio_WriteChannel(DIO_CH_LED3,     DIO_HIGH);
            break;
        case 3u:
            Dio_WriteChannel(DIO_CH_LED_GND,  DIO_LOW);
            Dio_WriteChannel(DIO_CH_LED4,     DIO_HIGH);
            break;
        case 4u:
            Dio_WriteChannel(DIO_CH_LED_LOW,  DIO_LOW);
            Dio_WriteChannel(DIO_CH_LED3,     DIO_HIGH);
            break;
        case 5u:
            Dio_WriteChannel(DIO_CH_LED_MID,  DIO_LOW);
            Dio_WriteChannel(DIO_CH_LED2,     DIO_HIGH);
            break;
        default:
            step = 0u;
            return;
    }

    step = (step >= 5u) ? 0u : (step + 1u);
}

/**
 * Animacao "pares alternados": 1+3 <-> 2+4
 * Chamada a cada 100 ms no modo chave ligada sem partida.
 * Usa Dio_WriteChannel para compatibilidade com MCAL apos init.
 */
static void Anim_AlternatePairs_Step(void)
{
    static uint8 toggle = 0u;

    /* Apaga tudo */
    Dio_WriteChannel(DIO_CH_LED_HIGH, DIO_HIGH);
    Dio_WriteChannel(DIO_CH_LED_MID,  DIO_HIGH);
    Dio_WriteChannel(DIO_CH_LED_LOW,  DIO_HIGH);
    Dio_WriteChannel(DIO_CH_LED_GND,  DIO_HIGH);
    Dio_WriteChannel(DIO_CH_LED1, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED2, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED3, DIO_LOW);
    Dio_WriteChannel(DIO_CH_LED4, DIO_LOW);

    if (toggle != 0u)
    {
        /* Par 1+3 */
        Dio_WriteChannel(DIO_CH_LED_HIGH, DIO_LOW);
        Dio_WriteChannel(DIO_CH_LED1,     DIO_HIGH);
        Dio_WriteChannel(DIO_CH_LED_LOW,  DIO_LOW);
        Dio_WriteChannel(DIO_CH_LED3,     DIO_HIGH);
    }
    else
    {
        /* Par 2+4 */
        Dio_WriteChannel(DIO_CH_LED_MID,  DIO_LOW);
        Dio_WriteChannel(DIO_CH_LED2,     DIO_HIGH);
        Dio_WriteChannel(DIO_CH_LED_GND,  DIO_LOW);
        Dio_WriteChannel(DIO_CH_LED4,     DIO_HIGH);
    }

    toggle ^= 1u;
}

/* ================================================================== */
/* Hook do display TFT - implementacao real para TC297B                */
/*                                                                      */
/* Sobrescreve a versao fraca declarada em ecu_tasks.c.                 */
/* Chamada por EcuTask_100ms() via EcuTask_Hook_DisplayUpdate().        */
/* ================================================================== */
void EcuTask_Hook_DisplayUpdate(void)
{
    ECU_Pages_Update();
}

/* ================================================================== */
/* core0_main: ponto de entrada do core0 do TC297B                     */
/* ================================================================== */
int core0_main(void)
{
    /* Habilita interrupcoes do core0 */
    IfxCpu_enableInterrupts();

    /* Desabilita watchdog do CPU0 e watchdog de segurança (desenvolvimento) */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* ---------------------------------------------------------------- */
    /* Configuracao inicial dos pinos de LED                             */
    /* Feita antes do Dio_Init() para a animacao de startup bloqueante.  */
    /* Apos EcuTask_Init(), o Dio_Init() reassume o controle dos pinos.  */
    /* ---------------------------------------------------------------- */

    /* LEDs da Application Kit (P13.0-3, anodo comum - nivel alto = apagado) */
    IfxPort_setPinMode(LED_HIGH, IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinMode(LED_MID,  IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinMode(LED_LOW,  IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinMode(LED_GND,  IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinHigh(LED_HIGH);
    IfxPort_setPinHigh(LED_MID);
    IfxPort_setPinHigh(LED_LOW);
    IfxPort_setPinHigh(LED_GND);

    /* LEDs customizados no conector X102 (anodo no pino - nivel baixo = apagado) */
    IfxPort_setPinMode(LED_1, IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinMode(LED_2, IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinMode(LED_3, IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinMode(LED_4, IfxPort_Mode_outputPushPullGeneral);
    IfxPort_setPinLow(LED_1);
    IfxPort_setPinLow(LED_2);
    IfxPort_setPinLow(LED_3);
    IfxPort_setPinLow(LED_4);

    /* ---------------------------------------------------------------- */
    /* Fase 1: Animacao de startup bloqueante (2 varridas rapidas)       */
    /* Executada antes do EcuTask_Init() pois usa IfxStm_waitTicks().    */
    /* ---------------------------------------------------------------- */
    {
        uint32 stmFreq   = (uint32)IfxStm_getFrequency(&MODULE_STM0);
        uint32 delay80ms  = stmFreq / 12u;   /* ~83 ms */
        uint32 delay250ms = stmFreq / 4u;    /* 250 ms */
        uint32 delay150ms = (stmFreq * 3u) / 20u; /* 150 ms */
        int i;

        for (i = 0; i < 2; i++)
        {
            IfxPort_setPinLow(LED_HIGH);  IfxPort_setPinHigh(LED_1);
            IfxStm_waitTicks(&MODULE_STM0, delay80ms);
            IfxPort_setPinHigh(LED_HIGH); IfxPort_setPinLow(LED_1);

            IfxPort_setPinLow(LED_MID);   IfxPort_setPinHigh(LED_2);
            IfxStm_waitTicks(&MODULE_STM0, delay80ms);
            IfxPort_setPinHigh(LED_MID);  IfxPort_setPinLow(LED_2);

            IfxPort_setPinLow(LED_LOW);   IfxPort_setPinHigh(LED_3);
            IfxStm_waitTicks(&MODULE_STM0, delay80ms);
            IfxPort_setPinHigh(LED_LOW);  IfxPort_setPinLow(LED_3);

            IfxPort_setPinLow(LED_GND);   IfxPort_setPinHigh(LED_4);
            IfxStm_waitTicks(&MODULE_STM0, delay80ms);
            IfxPort_setPinHigh(LED_GND);  IfxPort_setPinLow(LED_4);
        }

        /* Flash final: todos acendem juntos */
        IfxPort_setPinLow(LED_HIGH); IfxPort_setPinLow(LED_MID);
        IfxPort_setPinLow(LED_LOW);  IfxPort_setPinLow(LED_GND);
        IfxPort_setPinHigh(LED_1);   IfxPort_setPinHigh(LED_2);
        IfxPort_setPinHigh(LED_3);   IfxPort_setPinHigh(LED_4);
        IfxStm_waitTicks(&MODULE_STM0, delay250ms);

        /* Apaga tudo */
        IfxPort_setPinHigh(LED_HIGH); IfxPort_setPinHigh(LED_MID);
        IfxPort_setPinHigh(LED_LOW);  IfxPort_setPinHigh(LED_GND);
        IfxPort_setPinLow(LED_1);     IfxPort_setPinLow(LED_2);
        IfxPort_setPinLow(LED_3);     IfxPort_setPinLow(LED_4);
        IfxStm_waitTicks(&MODULE_STM0, delay150ms);
    }

    /* ---------------------------------------------------------------- */
    /* Inicializacao unificada de todos os modulos da ECU               */
    /* (MCAL, CDDs, XCP, callbacks GPT/ICU)                             */
    /* ---------------------------------------------------------------- */
    EcuTask_Init();

    /* ---------------------------------------------------------------- */
    /* Inicializacao do display TFT (especifico TC297B)                  */
    /* Feito apos EcuTask_Init() para garantir que SPI/DIO estejam ok.  */
    /* ---------------------------------------------------------------- */
    tft_init();
    TFT_Touch_Init();
    ECU_Pages_Init();

    /* ---------------------------------------------------------------- */
    /* Arma o tick de 1 ms via STM0 (unico uso de iLLD no caminho ciclico)*/
    /* Deve ser o ultimo passo antes do loop para evitar ISR prematura.  */
    /* ---------------------------------------------------------------- */
    initStmTick();

    /* ================================================================ */
    /* Loop principal - bare-metal, sem RTOS                            */
    /*                                                                   */
    /* Prioridade de execucao (maior para menor):                        */
    /*   1. XcpCanIf_Handler() - sub-1 ms, cada iteracao do loop        */
    /*   2. Flags periodicas (5/10/20/100 ms) - setadas pela ISR STM    */
    /*   3. EcuTask_Background() - ADC averaging, sem periodicidade fixa */
    /* ================================================================ */
    while (1)
    {
        /* XCP CAN polling - tempo de resposta < 1 ms exigido pelo protocolo */
        XcpCanIf_Handler();

        /* Despacha tarefas periodicas assim que a flag chegar */
        if (flag_5ms != 0u)
        {
            flag_5ms = 0u;
            EcuTask_5ms();
        }

        if (flag_10ms != 0u)
        {
            flag_10ms = 0u;
            EcuTask_10ms();
        }

        if (flag_20ms != 0u)
        {
            flag_20ms = 0u;
            EcuTask_20ms();
        }

        if (flag_100ms != 0u)
        {
            flag_100ms = 0u;
            EcuTask_100ms();
        }

        /* Background: amostragem ADC do TPS e filtragem */
        EcuTask_Background();
    }

    /* Linha nunca alcancada - suprime warning de return em funcao int */
    return 1;
}
