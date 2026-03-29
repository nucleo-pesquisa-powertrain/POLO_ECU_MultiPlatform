/**
 * \file Dio.h
 * \brief MCAL Digital I/O Abstraction
 *
 * API para controle de I/O discreto. A implementacao e' feita
 * por plataforma (TC297B: IfxPort, STM32H7: HAL_GPIO).
 *
 * Canais logicos definidos em Dio_Cfg.h de cada plataforma.
 */
#ifndef DIO_H
#define DIO_H

#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Tipos                                                               */
/* ------------------------------------------------------------------ */

/** Canal logico de DIO (mapeado para pino fisico na Cfg) */
typedef uint8 Dio_ChannelType;

/** Nivel logico do pino */
#define DIO_LOW     0u
#define DIO_HIGH    1u

/* ------------------------------------------------------------------ */
/* Canais logicos (comum a ambas plataformas - mesmo carro)            */
/* ------------------------------------------------------------------ */

/* Saidas - Injetores */
#define DIO_CH_INJECTOR1            0u
#define DIO_CH_INJECTOR2            1u
#define DIO_CH_INJECTOR3            2u
#define DIO_CH_INJECTOR4            3u

/* Saidas - Bobinas de Ignicao */
#define DIO_CH_COIL1                4u
#define DIO_CH_COIL2                5u

/* Saidas - Atuadores */
#define DIO_CH_FUEL_PUMP            6u
#define DIO_CH_COLDSTART_RELAY      7u
#define DIO_CH_FAN_LOW              8u
#define DIO_CH_FAN_HIGH             9u
#define DIO_CH_LAMBDA_HEATER1       10u
#define DIO_CH_LAMBDA_HEATER2       11u

/* Saidas - MC33810 */
#define DIO_CH_MC33810_CS           12u
#define DIO_CH_MC33810_ENOUT        13u

/* Saidas - MC33186 (H-Bridge) */
#define DIO_CH_MC33186_DI1          14u
#define DIO_CH_MC33186_DI2          15u
#define DIO_CH_MC33186_COD          16u
#define DIO_CH_MC33186_IN1          17u

/* Saidas - GIN (MC33810 Spark) */
#define DIO_CH_GIN0                 18u
#define DIO_CH_GIN1                 19u
#define DIO_CH_GIN2                 20u
#define DIO_CH_GIN3                 21u

/* Entradas Digitais */
#define DIO_CH_IGNITION_ON          22u
#define DIO_CH_PHASE_STATE          23u
#define DIO_CH_BRAKE_SW1            24u
#define DIO_CH_BRAKE_SW2            25u
#define DIO_CH_CLUTCH_SW            26u
#define DIO_CH_AC_SW                27u
#define DIO_CH_MC33186_SF           28u

/* LEDs de Debug */
#define DIO_CH_LED1                 29u
#define DIO_CH_LED2                 30u
#define DIO_CH_LED3                 31u
#define DIO_CH_LED4                 32u
#define DIO_CH_LED_HIGH             33u
#define DIO_CH_LED_MID              34u
#define DIO_CH_LED_LOW              35u
#define DIO_CH_LED_GND              36u

#define DIO_NUM_CHANNELS            37u

/* ------------------------------------------------------------------ */
/* API                                                                 */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa todos os pinos digitais (direcao, estado inicial)
 */
void Dio_Init(void);

/**
 * \brief Escreve nivel logico num canal
 * \param ch    Canal logico (DIO_CH_*)
 * \param level DIO_HIGH ou DIO_LOW
 */
void Dio_WriteChannel(Dio_ChannelType ch, uint8 level);

/**
 * \brief Le nivel logico de um canal
 * \param ch Canal logico (DIO_CH_*)
 * \return DIO_HIGH ou DIO_LOW
 */
uint8 Dio_ReadChannel(Dio_ChannelType ch);

/**
 * \brief Inverte o nivel logico de um canal
 * \param ch Canal logico (DIO_CH_*)
 */
void Dio_ToggleChannel(Dio_ChannelType ch);

#endif /* DIO_H */
