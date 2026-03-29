/**
 * \file Pwm.h
 * \brief MCAL PWM Generation Abstraction
 *
 * API para geracao de sinais PWM.
 * TC297B: CCU6 / GTM ATOM
 * STM32H7: TIM PWM
 */
#ifndef PWM_H
#define PWM_H

#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Tipos                                                               */
/* ------------------------------------------------------------------ */

typedef uint8 Pwm_ChannelType;

/* ------------------------------------------------------------------ */
/* Canais logicos de PWM                                               */
/* ------------------------------------------------------------------ */

#define PWM_CH_THROTTLE             0u  /* Ponte H da borboleta (MC33186) */
#define PWM_CH_CANISTER             1u  /* Valvula do canister (carvao ativo) */

#define PWM_NUM_CHANNELS            2u

/* ------------------------------------------------------------------ */
/* API                                                                 */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo PWM
 */
void Pwm_Init(void);

/**
 * \brief Configura duty cycle de um canal PWM
 * \param ch   Canal (PWM_CH_*)
 * \param duty Duty cycle (0..10000 = 0.00%..100.00%)
 */
void Pwm_SetDutyCycle(Pwm_ChannelType ch, uint16 duty);

/**
 * \brief Inicia geracao de PWM num canal
 * \param ch Canal (PWM_CH_*)
 */
void Pwm_Start(Pwm_ChannelType ch);

/**
 * \brief Para geracao de PWM num canal
 * \param ch Canal (PWM_CH_*)
 */
void Pwm_Stop(Pwm_ChannelType ch);

#endif /* PWM_H */
