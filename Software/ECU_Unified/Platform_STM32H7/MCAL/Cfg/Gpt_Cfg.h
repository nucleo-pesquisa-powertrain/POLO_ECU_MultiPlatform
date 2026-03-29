/**
 * \file Gpt_Cfg.h
 * \brief Configuracao dos timers GPT para STM32H745
 *
 * Mapeia os canais logicos GPT (definidos em Gpt.h) para os timers
 * fisicos do STM32H745. Configuracao de prescaler e frequencia de clock.
 *
 * Resolucao alvo: 1 us por tick (simples e suficiente para ignicao/injecao).
 *
 * Estrategia de clock:
 *   - Clock APB timer:  200 MHz (PCLK1/PCLK2 com multiplicador x2)
 *   - Prescaler:        200 - 1 = 199
 *   - Frequencia timer: 200MHz / 200 = 1 MHz => 1 tick = 1 us
 *   - Periodo maximo:
 *       TIM2, TIM5 (32-bit): ~4295 segundos
 *       TIM3, TIM4 (16-bit): ~65 ms (suficiente para injecao/dwell)
 *
 * Plataforma : STM32H745 (Cortex-M7, Core1 domina timers APB1)
 * HAL        : stm32h7xx_hal_tim.h
 *
 * \note TODO: Confirmar frequencia do clock APB do timer no CubeMX.
 *             Se PCLK1 = 100 MHz com multiplicador, ajustar
 *             GPT_TIMER_CLOCK_HZ e GPT_PRESCALER_VALUE.
 */
#ifndef GPT_CFG_H
#define GPT_CFG_H

#include "stm32h7xx_hal.h"
#include "Gpt.h"

/* ------------------------------------------------------------------ */
/* Parametros de clock                                                */
/* ------------------------------------------------------------------ */

/**
 * Frequencia do clock que alimenta os timers APB (em Hz).
 * STM32H745 pode atingir 200 MHz no timer clock (TIMPRE bit ativo).
 * TODO: Verificar RCC_ClkInitTypeDef e TIMPRE no arquivo gerado pelo CubeMX.
 */
#define GPT_TIMER_CLOCK_HZ          200000000UL  /* 200 MHz */

/**
 * Prescaler aplicado a todos os timers GPT.
 * Valor carregado no registrador TIMx->PSC = GPT_PRESCALER_VALUE.
 * A frequencia efetiva do contador e': f = clock / (PSC + 1)
 *
 * GPT_TIMER_CLOCK_HZ / (GPT_PRESCALER_VALUE + 1)
 * = 200000000 / 200
 * = 1000000 Hz => 1 us por tick
 */
#define GPT_PRESCALER_VALUE         (199U)  /* PSC = 199 => divisao por 200 */

/**
 * Frequencia efetiva do contador apos prescaler (Hz).
 * Derivada dos dois defines acima - usada para conversao us -> ticks.
 */
#define GPT_COUNTER_FREQ_HZ         (GPT_TIMER_CLOCK_HZ / (GPT_PRESCALER_VALUE + 1U))

/**
 * Resolucao em microssegundos por tick (= 1 us).
 * Com GPT_COUNTER_FREQ_HZ = 1 MHz, a conversao e' trivial (ticks == us).
 */
#define GPT_US_PER_TICK             (1U)

/* ------------------------------------------------------------------ */
/* Handles dos timers (gerados pelo CubeMX em main.c / tim.c)        */
/* ------------------------------------------------------------------ */

/*
 * Declaracao extern dos handles HAL gerados pelo CubeMX.
 * As definicoes estao em tim.c (ou main.c dependendo da versao do CubeMX).
 * TODO: Confirmar nomes dos handles no arquivo gerado (htim2, htim3...).
 */
extern TIM_HandleTypeDef htim2;  /* GPT_CH_IGN_TIMING  - 32-bit */
extern TIM_HandleTypeDef htim3;  /* GPT_CH_INJ_DURATION - 16-bit */
extern TIM_HandleTypeDef htim4;  /* GPT_CH_INJ_TIMING  - 16-bit */
extern TIM_HandleTypeDef htim5;  /* GPT_CH_DWELL       - 32-bit */

/* ------------------------------------------------------------------ */
/* Mapeamento canal logico -> instancia fisica                        */
/* ------------------------------------------------------------------ */

/**
 * Estrutura de configuracao de um canal GPT.
 * Associa o handle HAL, o numero da instancia TIM e informacoes
 * sobre o tamanho do contador (16 ou 32 bits).
 */
typedef struct
{
    TIM_HandleTypeDef* handle;      /**< Ponteiro para handle HAL TIM  */
    TIM_TypeDef*       instance;    /**< Instancia do periferico TIM   */
    uint8_t            is32bit;     /**< 1 se timer 32-bit, 0 se 16    */
    uint32_t           maxPeriod;   /**< Periodo maximo em ticks       */
} Gpt_ChannelConfigType;

/**
 * Tabela de configuracao dos canais GPT.
 * Indexada por GPT_CH_* definidos em Gpt.h.
 *
 * Canais 32-bit (TIM2, TIM5): periodo maximo ~4295 s  => ignicao e dwell
 * Canais 16-bit (TIM3, TIM4): periodo maximo ~65535 us => injecao
 *
 * TODO: No CubeMX, configurar TIM2..TIM5 em modo "One Pulse" ou
 *       "Up counting" com interrupcao de Update habilitada.
 *       Prescaler = GPT_PRESCALER_VALUE deve ser inserido manualmente.
 */
static const Gpt_ChannelConfigType Gpt_ChannelCfg[GPT_NUM_CHANNELS] =
{
    /*
     * [GPT_CH_IGN_TIMING = 0]: TIM2 - Timer de avanco de ignicao
     * Timer 32-bit no barramento APB1. Cobre angulos de ate' varios
     * segundos sem overflow (motor em baixa rotacao).
     * TODO: Habilitar interrupcao TIM2_IRQn com prioridade alta (2).
     */
    {
        &htim2,     /* handle HAL  */
        TIM2,       /* instancia   */
        1U,         /* 32-bit      */
        0xFFFFFFFFU /* ~4295 s max */
    },

    /*
     * [GPT_CH_INJ_DURATION = 1]: TIM3 - Duracao do pulso de injecao
     * Timer 16-bit. Duracao tipica de injecao: 1 ms a 20 ms (max ~65 ms).
     * TODO: Habilitar interrupcao TIM3_IRQn com prioridade alta (2).
     */
    {
        &htim3,     /* handle HAL  */
        TIM3,       /* instancia   */
        0U,         /* 16-bit      */
        0x0000FFFFU /* ~65535 us   */
    },

    /*
     * [GPT_CH_INJ_TIMING = 2]: TIM4 - Atraso ate' inicio da injecao
     * Timer 16-bit. Atraso tipico: < 20 ms.
     * TODO: Habilitar interrupcao TIM4_IRQn com prioridade alta (2).
     */
    {
        &htim4,     /* handle HAL  */
        TIM4,       /* instancia   */
        0U,         /* 16-bit      */
        0x0000FFFFU /* ~65535 us   */
    },

    /*
     * [GPT_CH_DWELL = 3]: TIM5 - Tempo de dwell da bobina
     * Timer 32-bit. Dwell tipico: 2 ms a 8 ms. 32-bit garante
     * margem para baixas rotacoes ou dwell aumentado.
     * TODO: Habilitar interrupcao TIM5_IRQn com prioridade alta (2).
     */
    {
        &htim5,     /* handle HAL  */
        TIM5,       /* instancia   */
        1U,         /* 32-bit      */
        0xFFFFFFFFU /* ~4295 s max */
    },
};

/* ------------------------------------------------------------------ */
/* Configuracao de prioridade das interrupcoes                        */
/* ------------------------------------------------------------------ */

/**
 * Prioridade NVIC dos timers GPT.
 * Timers de ignicao/injecao devem ter prioridade alta para garantir
 * jitter < 10 us conforme requisito do sistema.
 *
 * TODO: Ajustar conforme hierarquia de prioridades do projeto.
 *       Prioridade 2 e' alta mas abaixo de EXTI do crankshaft (prio 1).
 */
#define GPT_NVIC_PREEMPT_PRIORITY   2U
#define GPT_NVIC_SUB_PRIORITY       0U

/* ------------------------------------------------------------------ */
/* Macro auxiliar de conversao us -> ticks                           */
/* ------------------------------------------------------------------ */

/**
 * Converte microssegundos para ticks do contador.
 * Com prescaler de 200 e clock de 200 MHz, 1 tick = 1 us.
 * A macro e' trivial mas mantida para portabilidade.
 */
#define GPT_US_TO_TICKS(us)         ((uint32_t)(us))

/**
 * Converte ticks para microssegundos (inversa).
 */
#define GPT_TICKS_TO_US(ticks)      ((uint32_t)(ticks))

#endif /* GPT_CFG_H */
