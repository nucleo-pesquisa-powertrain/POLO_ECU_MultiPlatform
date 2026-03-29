/**
 * \file Icu.h
 * \brief MCAL Input Capture / Timestamp Abstraction
 *
 * API para captura de eventos do sensor de rotacao (crankshaft)
 * e timestamp de alta resolucao.
 *
 * TC297B: ERU (edge detection) + STM (free-running timestamp)
 * STM32H7: EXTI/TIM Input Capture + TIM free-running
 */
#ifndef ICU_H
#define ICU_H

#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Tipos                                                               */
/* ------------------------------------------------------------------ */

/** Callback chamado na deteccao de borda (dente do crankshaft) */
typedef void (*Icu_EdgeCallbackType)(void);

/* ------------------------------------------------------------------ */
/* API                                                                 */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo ICU (input capture + free-running timer)
 */
void Icu_Init(void);

/**
 * \brief Retorna timestamp livre em microssegundos
 *
 * Timer free-running de alta resolucao. Nao reseta.
 * TC297B: STM0 @ 100MHz
 * STM32H7: TIM dedicado @ freq alta
 *
 * \return Timestamp em us (overflow a cada ~4295 segundos em 32-bit)
 */
uint32 Icu_GetTimestamp_us(void);

/**
 * \brief Registra callback para borda do crankshaft
 * \param callback Funcao chamada a cada dente (ISR context)
 *
 * O callback deve ser CDD_SYNC_Timing_Event() que por sua vez
 * chama CDD_TriggerWheel_Event() internamente.
 */
void Icu_SetEdgeCallback(Icu_EdgeCallbackType callback);

/**
 * \brief Habilita a interrupcao de borda do crankshaft
 */
void Icu_EnableEdgeDetection(void);

/**
 * \brief Desabilita a interrupcao de borda do crankshaft
 */
void Icu_DisableEdgeDetection(void);

#endif /* ICU_H */
