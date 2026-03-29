/**
 * \file Gpt_Cfg.h
 * \brief Configuracao do modulo GPT12 para TC297B
 *
 * O TC297B possui o modulo GPT12 com 6 timers de 16 bits (T2..T6).
 * Neste projeto, quatro timers sao utilizados para o sincronismo
 * de ignicao e injecao:
 *
 *   T2 -> GPT_CH_IGN_TIMING    (inicio da ignicao)
 *   T3 -> GPT_CH_INJ_DURATION  (duracao do pulso de injecao)
 *   T4 -> GPT_CH_INJ_TIMING    (inicio da injecao)
 *   T6 -> GPT_CH_DWELL         (tempo de dwell da bobina)
 *
 * Todos os timers operam em modo one-shot (contagem regressiva).
 * A interrupcao e' gerada no underflow (contador chega a 0).
 *
 * Calculo de ticks:
 *   f_GPT12  = 100 MHz (FREQ_MHZ_TIMER)
 *   Prescaler = 4 (PRSC_T2) * 16 (PRSC_BPS1) = 64 (TOTAL_PRSC_TIMER)
 *   f_timer  = 100 MHz / 64 = 1.5625 MHz
 *   Resolucao = 0.64 us/tick
 *
 *   ticks = timeout_us * FREQ_MHZ_TIMER / TOTAL_PRSC_TIMER
 *         = timeout_us * 100 / 64
 *         = timeout_us * 25 / 16
 *
 * Plataforma: Infineon AURIX TC297B
 * Driver iLLD: IfxGpt12_T2, IfxGpt12_T3, IfxGpt12_T4, IfxGpt12_T6
 */

#ifndef GPT_CFG_H
#define GPT_CFG_H

/* ------------------------------------------------------------------ */
/* Parametros de clock do modulo GPT12                                 */
/* ------------------------------------------------------------------ */

/** Frequencia de entrada do modulo GPT12 em MHz (fGPT = fSPB = 100 MHz) */
#define GPT_FREQ_MHZ_TIMER          100u

/**
 * \brief Prescaler total aplicado ao clock antes do contador
 *
 * BPS1 (prescaler do bloco 1 - T2/T3/T4) = /16
 * T2/T3/T4 prescaler individual           = /4
 * Total: 16 * 4 = 64
 *
 * T6 pertence ao bloco 2 (BPS2), configurado com o mesmo divisor total.
 */
#define GPT_TOTAL_PRSC_TIMER        64u

/* ------------------------------------------------------------------ */
/* Mapeamento: canal logico -> timer fisico do GPT12                   */
/* ------------------------------------------------------------------ */

/**
 * \brief Tipo de identificador do timer fisico do GPT12
 *
 * Usado internamente pela implementacao Gpt.c para indexar a tabela
 * de configuracao de timers.
 */
typedef enum
{
    GPT_HW_TIMER_T2 = 0u,   /**< GPT_CH_IGN_TIMING    */
    GPT_HW_TIMER_T3 = 1u,   /**< GPT_CH_INJ_DURATION  */
    GPT_HW_TIMER_T4 = 2u,   /**< GPT_CH_INJ_TIMING    */
    GPT_HW_TIMER_T6 = 3u    /**< GPT_CH_DWELL         */
} Gpt_HwTimerId;

/**
 * \brief Mapeamento entre canal logico (GPT_CH_*) e timer fisico
 *
 * O indice deste array corresponde ao canal logico definido em Gpt.h:
 *   [0] GPT_CH_IGN_TIMING   -> T2
 *   [1] GPT_CH_INJ_DURATION -> T3
 *   [2] GPT_CH_INJ_TIMING   -> T4
 *   [3] GPT_CH_DWELL        -> T6
 */
extern const Gpt_HwTimerId Gpt_ChannelToHwTimer[];

/* ------------------------------------------------------------------ */
/* Vetores de interrupcao GPT12 (TriCore SRC)                         */
/* ------------------------------------------------------------------ */
/*
 * Os ISRs do GPT12 sao registrados via macros iLLD IFX_INTERRUPT().
 * A tabela abaixo documenta o mapeamento para referencia:
 *
 *   Timer T2  -> IFX_INTPRIO_GPT12_T2  (definir no Irq_Cfg.h do BSP)
 *   Timer T3  -> IFX_INTPRIO_GPT12_T3
 *   Timer T4  -> IFX_INTPRIO_GPT12_T4
 *   Timer T6  -> IFX_INTPRIO_GPT12_T6
 *
 * Os ISRs estao implementados em Gpt.c com a macro:
 *   IFX_INTERRUPT(Gpt_IsrT2, 0, IFX_INTPRIO_GPT12_T2) { ... }
 */

/* Prioridades de interrupcao GPT (ajustar conforme tabela de IRQ do projeto) */
#define GPT_IRQ_PRIO_T2             50u
#define GPT_IRQ_PRIO_T3             51u
#define GPT_IRQ_PRIO_T4             52u
#define GPT_IRQ_PRIO_T6             53u

#endif /* GPT_CFG_H */
