/**
 * \file Gpt.h
 * \brief MCAL General Purpose Timer Abstraction
 *
 * API para timers one-shot usados no sincronismo de ignicao e injecao.
 * TC297B: GPT12 (T2, T3, T4, T6)
 * STM32H7: TIM (a definir)
 *
 * Todos os tempos sao em microssegundos (us). A conversao para ticks
 * do timer e' feita internamente pela implementacao de cada plataforma.
 */
#ifndef GPT_H
#define GPT_H

#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Tipos                                                               */
/* ------------------------------------------------------------------ */

typedef uint8 Gpt_ChannelType;

/** Callback chamado quando o timer expira (overflow) */
typedef void (*Gpt_NotificationType)(void);

/* ------------------------------------------------------------------ */
/* Canais logicos de timer                                             */
/* ------------------------------------------------------------------ */

/** Timer para inicio da ignicao (TC297B: T2) */
#define GPT_CH_IGN_TIMING           0u

/** Timer para duracao da injecao (TC297B: T3) */
#define GPT_CH_INJ_DURATION         1u

/** Timer para inicio da injecao (TC297B: T4) */
#define GPT_CH_INJ_TIMING           2u

/** Timer para tempo de dwell da bobina (TC297B: T6) */
#define GPT_CH_DWELL                3u

#define GPT_NUM_CHANNELS            4u

/* ------------------------------------------------------------------ */
/* API                                                                 */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo GPT (timers, prescalers, interrupcoes)
 */
void Gpt_Init(void);

/**
 * \brief Registra callback para quando o timer expirar
 * \param ch       Canal do timer (GPT_CH_*)
 * \param callback Funcao a ser chamada na expiracao (ISR context)
 */
void Gpt_SetNotification(Gpt_ChannelType ch, Gpt_NotificationType callback);

/**
 * \brief Inicia timer one-shot com timeout em microssegundos
 * \param ch         Canal do timer (GPT_CH_*)
 * \param timeout_us Tempo ate expiracao em us
 *
 * Quando o timer expirar, o callback registrado sera chamado.
 * Se o timer ja estiver rodando, sera reiniciado com o novo valor.
 */
void Gpt_StartTimer(Gpt_ChannelType ch, uint32 timeout_us);

/**
 * \brief Para o timer
 * \param ch Canal do timer (GPT_CH_*)
 */
void Gpt_StopTimer(Gpt_ChannelType ch);

/**
 * \brief Retorna o tempo decorrido desde o inicio do timer em us
 * \param ch Canal do timer (GPT_CH_*)
 * \return Tempo decorrido em us
 *
 * Usado no CDD de injecao para calcular o tempo restante
 * quando ha fila de injetores.
 */
uint32 Gpt_GetElapsedTime_us(Gpt_ChannelType ch);

#endif /* GPT_H */
