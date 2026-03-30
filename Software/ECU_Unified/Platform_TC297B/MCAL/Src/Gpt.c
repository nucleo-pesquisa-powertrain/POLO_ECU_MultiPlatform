/**
 * \file Gpt.c
 * \brief Implementacao MCAL GPT para Infineon AURIX TC297B
 *
 * Delega a inicializacao do GPT12 para initGpt12Timer() em
 * HardwareAdp/GPT12_Timer_Interrupt.c, que ja contem as ISRs
 * funcionais chamando os callbacks do CDD diretamente.
 *
 * Este modulo fornece:
 * - Gpt_StartTimer: carrega e inicia um timer one-shot (us -> ticks)
 * - Gpt_StopTimer: para o timer
 * - Gpt_GetElapsedTime_us: tempo decorrido desde o inicio
 *
 * Mapeamento:
 *   GPT_CH_IGN_TIMING   (0) -> T2
 *   GPT_CH_INJ_DURATION (1) -> T3
 *   GPT_CH_INJ_TIMING   (2) -> T4
 *   GPT_CH_DWELL        (3) -> T6
 *
 * Plataforma: Infineon AURIX TC297B
 */

#include "Gpt.h"
#include "Gpt_Cfg.h"
#include "Mcal_Compiler.h"

/* iLLD - GPT12 */
#include "IfxGpt12.h"

/* iLLD - STM para timestamp */
#include "IfxStm.h"
#include "IfxStm_reg.h"

/* HardwareAdp - init e ISRs ja configurados */
#include "GPT12_Timer_Interrupt.h"

/* ------------------------------------------------------------------ */
/* Estado interno                                                      */
/* ------------------------------------------------------------------ */

typedef struct
{
    Gpt_NotificationType callback;
    uint32               startStm;
    uint32               timeout_us;
    boolean              running;
} Gpt_ChannelState;

static Gpt_ChannelState Gpt_State[GPT_NUM_CHANNELS];

/* ------------------------------------------------------------------ */
/* Funcoes auxiliares                                                   */
/* ------------------------------------------------------------------ */

static uint16 Gpt_UsToTicks(uint32 timeout_us)
{
    uint32 ticks = (timeout_us * (uint32)GPT_FREQ_MHZ_TIMER) / (uint32)GPT_TOTAL_PRSC_TIMER;
    if (ticks > 0xFFFFu) ticks = 0xFFFFu;
    if (ticks == 0u) ticks = 1u;
    return (uint16)ticks;
}

/* ------------------------------------------------------------------ */
/* API                                                                 */
/* ------------------------------------------------------------------ */

void Gpt_Init(void)
{
    uint8 ch;

    /* Delega toda a config de GPT12 para o codigo existente em HardwareAdp.
     * Ele configura prescalers, modos, direcao, e registra as ISRs
     * (GPT12_T2_Int0_Handler, etc.) que chamam os CDDs diretamente. */
    initGpt12Timer();

    for (ch = 0u; ch < GPT_NUM_CHANNELS; ch++)
    {
        Gpt_State[ch].callback   = (Gpt_NotificationType)0;
        Gpt_State[ch].startStm   = 0u;
        Gpt_State[ch].timeout_us = 0u;
        Gpt_State[ch].running    = FALSE;
    }
}

void Gpt_SetNotification(Gpt_ChannelType ch, Gpt_NotificationType callback)
{
    if (ch >= GPT_NUM_CHANNELS) return;

    /* No TC297B, os callbacks sao hardcoded nas ISRs de GPT12_Timer_Interrupt.c.
     * Armazena para compatibilidade com a API. */
    Gpt_State[ch].callback = callback;
}

void Gpt_StartTimer(Gpt_ChannelType ch, uint32 timeout_us)
{
    uint16 ticks;

    if (ch >= GPT_NUM_CHANNELS) return;

    ticks = Gpt_UsToTicks(timeout_us);

    Mcal_DisableAllInterrupts();

    Gpt_State[ch].startStm   = IfxStm_getLower(&MODULE_STM0);
    Gpt_State[ch].timeout_us = timeout_us;
    Gpt_State[ch].running    = TRUE;

    switch (ch)
    {
        case GPT_CH_IGN_TIMING:
            IfxGpt12_T2_setTimerValue(&MODULE_GPT120, ticks);
            IfxGpt12_T2_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;
        case GPT_CH_INJ_DURATION:
            IfxGpt12_T3_setTimerValue(&MODULE_GPT120, ticks);
            IfxGpt12_T3_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;
        case GPT_CH_INJ_TIMING:
            IfxGpt12_T4_setTimerValue(&MODULE_GPT120, ticks);
            IfxGpt12_T4_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;
        case GPT_CH_DWELL:
            IfxGpt12_T6_setTimerValue(&MODULE_GPT120, ticks);
            IfxGpt12_T6_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;
        default:
            Gpt_State[ch].running = FALSE;
            break;
    }

    Mcal_EnableAllInterrupts();
}

void Gpt_StopTimer(Gpt_ChannelType ch)
{
    if (ch >= GPT_NUM_CHANNELS) return;

    switch (ch)
    {
        case GPT_CH_IGN_TIMING:
            IfxGpt12_T2_run(&MODULE_GPT120, IfxGpt12_TimerRun_stop);
            break;
        case GPT_CH_INJ_DURATION:
            IfxGpt12_T3_run(&MODULE_GPT120, IfxGpt12_TimerRun_stop);
            break;
        case GPT_CH_INJ_TIMING:
            IfxGpt12_T4_run(&MODULE_GPT120, IfxGpt12_TimerRun_stop);
            break;
        case GPT_CH_DWELL:
            IfxGpt12_T6_run(&MODULE_GPT120, IfxGpt12_TimerRun_stop);
            break;
        default:
            break;
    }

    Gpt_State[ch].running = FALSE;
}

uint32 Gpt_GetElapsedTime_us(Gpt_ChannelType ch)
{
    uint32 currentStm;
    uint32 deltaStm;

    if (ch >= GPT_NUM_CHANNELS) return 0u;
    if (Gpt_State[ch].running == FALSE) return 0u;

    currentStm = IfxStm_getLower(&MODULE_STM0);
    deltaStm = currentStm - Gpt_State[ch].startStm;

    return deltaStm / 100u;
}
