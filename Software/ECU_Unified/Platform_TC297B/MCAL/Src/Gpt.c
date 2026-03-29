/**
 * \file Gpt.c
 * \brief Implementacao MCAL GPT para Infineon AURIX TC297B
 *
 * Encapsula o modulo GPT12 do TC297B para fornecer timers one-shot
 * com callbacks na expiracao, usados pelo sincronismo de ignicao e
 * injecao.
 *
 * Mapeamento de canais logicos -> timers fisicos GPT12:
 *   GPT_CH_IGN_TIMING   (0) -> T2  (bloco 1)
 *   GPT_CH_INJ_DURATION (1) -> T3  (bloco 1)
 *   GPT_CH_INJ_TIMING   (2) -> T4  (bloco 1)
 *   GPT_CH_DWELL        (3) -> T6  (bloco 2)
 *
 * Calculo de ticks (conversao de us para contagem do timer):
 *   f_GPT12       = 100 MHz
 *   Prescaler     = 64 (BPS1=/16, T2/T3/T4 individual=/4)
 *   f_efetiva     = 100 MHz / 64 = 1.5625 MHz
 *   ticks         = timeout_us * 100 / 64
 *                 = timeout_us * 25 / 16
 *
 * Todos os timers operam em modo decrescente (T2/T3/T4/T6 como timers
 * independentes). A interrupcao e' gerada no underflow (T_xx = 0 ->
 * recarrega 0xFFFF e continua; o ISR deve parar o timer apos o callback).
 *
 * Mapeamento de vetores de interrupcao TriCore:
 *   T2 overflow -> ISR Gpt_IsrT2  (SRC_GPT120T2, prioridade GPT_IRQ_PRIO_T2)
 *   T3 overflow -> ISR Gpt_IsrT3  (SRC_GPT120T3, prioridade GPT_IRQ_PRIO_T3)
 *   T4 overflow -> ISR Gpt_IsrT4  (SRC_GPT120T4, prioridade GPT_IRQ_PRIO_T4)
 *   T6 overflow -> ISR Gpt_IsrT6  (SRC_GPT120T6, prioridade GPT_IRQ_PRIO_T6)
 *
 * Plataforma: Infineon AURIX TC297B
 * Driver iLLD: IfxGpt12_T2, IfxGpt12_T3, IfxGpt12_T4, IfxGpt12_T6
 * Timestamp:   IfxStm_getLower(&MODULE_STM0)
 */

/* ------------------------------------------------------------------ */
/* Includes                                                            */
/* ------------------------------------------------------------------ */

#include "Gpt.h"
#include "Gpt_Cfg.h"
#include "Mcal_Compiler.h"

/* iLLD - GPT12 */
#include "IfxGpt12.h"

/* iLLD - STM para timestamp de alta resolucao */
#include "IfxStm.h"
#include "IfxStm_reg.h"

/* ------------------------------------------------------------------ */
/* Tabela de mapeamento: canal logico -> timer fisico                  */
/* ------------------------------------------------------------------ */

const Gpt_HwTimerId Gpt_ChannelToHwTimer[GPT_NUM_CHANNELS] =
{
    GPT_HW_TIMER_T2,    /* GPT_CH_IGN_TIMING   */
    GPT_HW_TIMER_T3,    /* GPT_CH_INJ_DURATION */
    GPT_HW_TIMER_T4,    /* GPT_CH_INJ_TIMING   */
    GPT_HW_TIMER_T6     /* GPT_CH_DWELL        */
};

/* ------------------------------------------------------------------ */
/* Estado interno dos canais                                           */
/* ------------------------------------------------------------------ */

/**
 * \brief Estado de um canal de timer
 */
typedef struct
{
    Gpt_NotificationType callback;      /**< Callback registrado via Gpt_SetNotification()  */
    uint32               startStm;      /**< Valor do STM0 no momento de Gpt_StartTimer()   */
    uint32               timeout_us;    /**< Timeout configurado em us (para GetElapsed)     */
    boolean              running;       /**< TRUE enquanto o timer estiver ativo             */
} Gpt_ChannelState;

/** Array de estado: indice = canal logico GPT_CH_* */
static Gpt_ChannelState Gpt_State[GPT_NUM_CHANNELS];

/* ------------------------------------------------------------------ */
/* Funcoes auxiliares internas                                         */
/* ------------------------------------------------------------------ */

/**
 * \brief Converte timeout em us para ticks do timer GPT12
 *
 * Formula: ticks = timeout_us * FREQ_MHZ_TIMER / TOTAL_PRSC_TIMER
 *               = timeout_us * 100 / 64
 *               = timeout_us * 25 / 16
 *
 * Saturacao para 0xFFFF (max valor de 16 bits do timer).
 *
 * \param timeout_us Tempo desejado em microssegundos
 * \return Valor de pre-carga para o registrador do timer (16-bit)
 */
static uint16 Gpt_UsToTicks(uint32 timeout_us)
{
    uint32 ticks;

    /* Multiplicacao primeiro para manter precisao, depois divisao */
    ticks = (timeout_us * (uint32)GPT_FREQ_MHZ_TIMER) / (uint32)GPT_TOTAL_PRSC_TIMER;

    /* Saturacao: timer GPT12 e' 16-bit */
    if (ticks > 0xFFFFu)
    {
        ticks = 0xFFFFu;
    }

    return (uint16)ticks;
}

/**
 * \brief Le o contador STM0 em us
 *
 * STM0 opera a 100 MHz. Para converter para us: divide por 100.
 * Usa apenas o lower 32 bits (suficiente para ~42 segundos).
 *
 * \return Timestamp em us
 */
static uint32 Gpt_GetStm_us(void)
{
    return IfxStm_getLower(&MODULE_STM0) / 100u;
}

/* ------------------------------------------------------------------ */
/* Implementacao da API                                                */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo GPT12
 *
 * Configura o prescaler do bloco 1 (T2/T3/T4) e bloco 2 (T6).
 * Todos os timers sao parados inicialmente.
 * As interrupcoes sao habilitadas mas os timers so' iniciam com
 * Gpt_StartTimer().
 */
void Gpt_Init(void)
{
    uint8 ch;

    /* Habilita o clock do modulo GPT12 */
    IfxGpt12_enableModule(&MODULE_GPT120);

    /*
     * Prescaler do bloco 1 (T2, T3, T4): /16
     * Cada timer individual pode ter prescaler adicional de /1, /2, /4, /8
     * Configuramos /4 para cada um -> total /64 (GPT_TOTAL_PRSC_TIMER)
     */
    IfxGpt12_setGpt1BlockPrescaler(&MODULE_GPT120, IfxGpt12_Gpt1BlockPrescaler_16);

    /*
     * Prescaler do bloco 2 (T5, T6): /16
     * T6 tambem com prescaler individual /4 -> total /64
     */
    IfxGpt12_setGpt2BlockPrescaler(&MODULE_GPT120, IfxGpt12_Gpt2BlockPrescaler_16);

    /* --- Configuracao T2 (GPT_CH_IGN_TIMING) --- */
    IfxGpt12_T2_setMode(&MODULE_GPT120, IfxGpt12_Mode_timer);
    IfxGpt12_T2_setTimerDirection(&MODULE_GPT120, IfxGpt12_TimerDirection_down);
    IfxGpt12_T2_setTimerPrescaler(&MODULE_GPT120, IfxGpt12_TimerInputPrescaler_4);
    IfxGpt12_T2_setTimerValue(&MODULE_GPT120, 0xFFFFu);
    /* Habilita interrupcao T2 overflow no servico de CPU0 */
    IfxGpt12_T2_setInterruptEnable(&MODULE_GPT120, TRUE);
    volatile Ifx_SRC_SRCR* src_t2 = IfxGpt12_T2_getSrc(&MODULE_GPT120);
    IfxSrc_init(src_t2, IfxSrc_Tos_cpu0, GPT_IRQ_PRIO_T2);
    IfxSrc_enable(src_t2);

    /* --- Configuracao T3 (GPT_CH_INJ_DURATION) --- */
    IfxGpt12_T3_setMode(&MODULE_GPT120, IfxGpt12_Mode_timer);
    IfxGpt12_T3_setTimerDirection(&MODULE_GPT120, IfxGpt12_TimerDirection_down);
    IfxGpt12_T3_setTimerPrescaler(&MODULE_GPT120, IfxGpt12_TimerInputPrescaler_4);
    IfxGpt12_T3_setTimerValue(&MODULE_GPT120, 0xFFFFu);
    IfxGpt12_T3_setInterruptEnable(&MODULE_GPT120, TRUE);
    volatile Ifx_SRC_SRCR* src_t3 = IfxGpt12_T3_getSrc(&MODULE_GPT120);
    IfxSrc_init(src_t3, IfxSrc_Tos_cpu0, GPT_IRQ_PRIO_T3);
    IfxSrc_enable(src_t3);

    /* --- Configuracao T4 (GPT_CH_INJ_TIMING) --- */
    IfxGpt12_T4_setMode(&MODULE_GPT120, IfxGpt12_Mode_timer);
    IfxGpt12_T4_setTimerDirection(&MODULE_GPT120, IfxGpt12_TimerDirection_down);
    IfxGpt12_T4_setTimerPrescaler(&MODULE_GPT120, IfxGpt12_TimerInputPrescaler_4);
    IfxGpt12_T4_setTimerValue(&MODULE_GPT120, 0xFFFFu);
    IfxGpt12_T4_setInterruptEnable(&MODULE_GPT120, TRUE);
    volatile Ifx_SRC_SRCR* src_t4 = IfxGpt12_T4_getSrc(&MODULE_GPT120);
    IfxSrc_init(src_t4, IfxSrc_Tos_cpu0, GPT_IRQ_PRIO_T4);
    IfxSrc_enable(src_t4);

    /* --- Configuracao T6 (GPT_CH_DWELL) --- */
    IfxGpt12_T6_setMode(&MODULE_GPT120, IfxGpt12_Mode_timer);
    IfxGpt12_T6_setTimerDirection(&MODULE_GPT120, IfxGpt12_TimerDirection_down);
    IfxGpt12_T6_setTimerPrescaler(&MODULE_GPT120, IfxGpt12_TimerInputPrescaler_4);
    IfxGpt12_T6_setTimerValue(&MODULE_GPT120, 0xFFFFu);
    IfxGpt12_T6_setInterruptEnable(&MODULE_GPT120, TRUE);
    volatile Ifx_SRC_SRCR* src_t6 = IfxGpt12_T6_getSrc(&MODULE_GPT120);
    IfxSrc_init(src_t6, IfxSrc_Tos_cpu0, GPT_IRQ_PRIO_T6);
    IfxSrc_enable(src_t6);

    /* Inicializa estado interno de todos os canais */
    for (ch = 0u; ch < GPT_NUM_CHANNELS; ch++)
    {
        Gpt_State[ch].callback   = (Gpt_NotificationType)0;
        Gpt_State[ch].startStm   = 0u;
        Gpt_State[ch].timeout_us = 0u;
        Gpt_State[ch].running    = FALSE;
    }
}

/**
 * \brief Registra callback para expiracao do timer
 *
 * \param ch       Canal do timer (GPT_CH_*)
 * \param callback Funcao chamada na expiracao (contexto ISR)
 */
void Gpt_SetNotification(Gpt_ChannelType ch, Gpt_NotificationType callback)
{
    if (ch >= GPT_NUM_CHANNELS)
    {
        return;
    }

    Mcal_DisableAllInterrupts();
    Gpt_State[ch].callback = callback;
    Mcal_EnableAllInterrupts();
}

/**
 * \brief Inicia timer one-shot com timeout em microssegundos
 *
 * O timer e' pre-carregado com o valor de ticks calculado e imediatamente
 * habilitado. Se ja estiver rodando, e' reiniciado com o novo valor.
 * O timestamp STM e' capturado para permitir Gpt_GetElapsedTime_us().
 *
 * \param ch         Canal do timer (GPT_CH_*)
 * \param timeout_us Tempo ate expiracao em us
 */
void Gpt_StartTimer(Gpt_ChannelType ch, uint32 timeout_us)
{
    uint16 ticks;

    if (ch >= GPT_NUM_CHANNELS)
    {
        return;
    }

    ticks = Gpt_UsToTicks(timeout_us);

    Mcal_DisableAllInterrupts();

    /* Registra o timestamp do inicio para GetElapsedTime */
    Gpt_State[ch].startStm   = IfxStm_getLower(&MODULE_STM0);
    Gpt_State[ch].timeout_us = timeout_us;
    Gpt_State[ch].running    = TRUE;

    /* Carrega valor e inicia o timer correspondente */
    switch (ch)
    {
        case GPT_CH_IGN_TIMING:     /* T2 */
            IfxGpt12_T2_setTimerValue(&MODULE_GPT120, ticks);
            IfxGpt12_T2_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;

        case GPT_CH_INJ_DURATION:   /* T3 */
            IfxGpt12_T3_setTimerValue(&MODULE_GPT120, ticks);
            IfxGpt12_T3_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;

        case GPT_CH_INJ_TIMING:     /* T4 */
            IfxGpt12_T4_setTimerValue(&MODULE_GPT120, ticks);
            IfxGpt12_T4_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;

        case GPT_CH_DWELL:          /* T6 */
            IfxGpt12_T6_setTimerValue(&MODULE_GPT120, ticks);
            IfxGpt12_T6_run(&MODULE_GPT120, IfxGpt12_TimerRun_start);
            break;

        default:
            Gpt_State[ch].running = FALSE;
            break;
    }

    Mcal_EnableAllInterrupts();
}

/**
 * \brief Para o timer
 *
 * \param ch Canal do timer (GPT_CH_*)
 */
void Gpt_StopTimer(Gpt_ChannelType ch)
{
    if (ch >= GPT_NUM_CHANNELS)
    {
        return;
    }

    Mcal_DisableAllInterrupts();

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

    Mcal_EnableAllInterrupts();
}

/**
 * \brief Retorna o tempo decorrido desde o inicio do timer em us
 *
 * Calcula a diferenca entre o STM atual e o STM capturado em
 * Gpt_StartTimer(). Robusto a overflow do STM de 32 bits devido
 * a subtracao sem sinal.
 *
 * \param ch Canal do timer (GPT_CH_*)
 * \return Tempo decorrido em us, ou 0 se o timer nao estiver rodando
 */
uint32 Gpt_GetElapsedTime_us(Gpt_ChannelType ch)
{
    uint32 currentStm;
    uint32 deltaStm;

    if (ch >= GPT_NUM_CHANNELS)
    {
        return 0u;
    }

    if (Gpt_State[ch].running == FALSE)
    {
        return 0u;
    }

    /* Leitura atomica do STM (32-bit read e' atomica no TriCore) */
    currentStm = IfxStm_getLower(&MODULE_STM0);

    /*
     * Subtracao sem sinal: funciona corretamente mesmo com overflow
     * do contador STM de 32 bits (ex: currentStm < startStm).
     */
    deltaStm = currentStm - Gpt_State[ch].startStm;

    /* STM0 @ 100 MHz -> dividir por 100 para obter us */
    return deltaStm / 100u;
}

/* ------------------------------------------------------------------ */
/* Rotinas de servico de interrupcao (ISR) do GPT12                   */
/* ------------------------------------------------------------------ */
/*
 * Os ISRs sao registrados usando a macro IFX_INTERRUPT() do iLLD.
 * Cada ISR:
 *   1. Para o timer (comportamento one-shot)
 *   2. Marca o canal como nao-rodando
 *   3. Chama o callback registrado (se nao-nulo)
 *
 * ATENCAO: os ISRs executam no contexto de interrupcao. Os callbacks
 * devem ser curtos e nao podem chamar funcoes bloqueantes.
 */

/**
 * \brief ISR do timer T2 (GPT_CH_IGN_TIMING - inicio da ignicao)
 *
 * Vetor: SRC_GPT120T2
 * Prioridade: GPT_IRQ_PRIO_T2
 */
IFX_INTERRUPT(Gpt_IsrT2, 0, GPT_IRQ_PRIO_T2)
{
    /* Para o timer imediatamente (one-shot) */
    IfxGpt12_T2_run(&MODULE_GPT120, IfxGpt12_TimerRun_stop);
    Gpt_State[GPT_CH_IGN_TIMING].running = FALSE;

    /* Chama callback se registrado */
    if (Gpt_State[GPT_CH_IGN_TIMING].callback != (Gpt_NotificationType)0)
    {
        Gpt_State[GPT_CH_IGN_TIMING].callback();
    }
}

/**
 * \brief ISR do timer T3 (GPT_CH_INJ_DURATION - duracao da injecao)
 *
 * Vetor: SRC_GPT120T3
 * Prioridade: GPT_IRQ_PRIO_T3
 */
IFX_INTERRUPT(Gpt_IsrT3, 0, GPT_IRQ_PRIO_T3)
{
    IfxGpt12_T3_run(&MODULE_GPT120, IfxGpt12_TimerRun_stop);
    Gpt_State[GPT_CH_INJ_DURATION].running = FALSE;

    if (Gpt_State[GPT_CH_INJ_DURATION].callback != (Gpt_NotificationType)0)
    {
        Gpt_State[GPT_CH_INJ_DURATION].callback();
    }
}

/**
 * \brief ISR do timer T4 (GPT_CH_INJ_TIMING - inicio da injecao)
 *
 * Vetor: SRC_GPT120T4
 * Prioridade: GPT_IRQ_PRIO_T4
 */
IFX_INTERRUPT(Gpt_IsrT4, 0, GPT_IRQ_PRIO_T4)
{
    IfxGpt12_T4_run(&MODULE_GPT120, IfxGpt12_TimerRun_stop);
    Gpt_State[GPT_CH_INJ_TIMING].running = FALSE;

    if (Gpt_State[GPT_CH_INJ_TIMING].callback != (Gpt_NotificationType)0)
    {
        Gpt_State[GPT_CH_INJ_TIMING].callback();
    }
}

/**
 * \brief ISR do timer T6 (GPT_CH_DWELL - dwell time da bobina)
 *
 * Vetor: SRC_GPT120T6
 * Prioridade: GPT_IRQ_PRIO_T6
 */
IFX_INTERRUPT(Gpt_IsrT6, 0, GPT_IRQ_PRIO_T6)
{
    IfxGpt12_T6_run(&MODULE_GPT120, IfxGpt12_TimerRun_stop);
    Gpt_State[GPT_CH_DWELL].running = FALSE;

    if (Gpt_State[GPT_CH_DWELL].callback != (Gpt_NotificationType)0)
    {
        Gpt_State[GPT_CH_DWELL].callback();
    }
}
