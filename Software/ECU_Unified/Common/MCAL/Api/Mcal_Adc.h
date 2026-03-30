/**
 * \file Adc.h
 * \brief MCAL ADC Abstraction
 *
 * API para leitura de canais analogicos. A implementacao e' feita
 * por plataforma (TC297B: DSADC/VADC, STM32H7: HAL_ADC).
 */
#ifndef ADC_H
#define ADC_H

#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Tipos                                                               */
/* ------------------------------------------------------------------ */

typedef uint8 Adc_ChannelType;

/* ------------------------------------------------------------------ */
/* Canais logicos (comum - mesmo carro, mesmos sensores)               */
/* ------------------------------------------------------------------ */

#define ADC_CH_TBI_POS              0u  /* Posicao borboleta TPS1 */
#define ADC_CH_TBI_POS_RED          1u  /* Posicao borboleta TPS2 (redundante) */
#define ADC_CH_MAP                  2u  /* Pressao do coletor (MAP) */
#define ADC_CH_COOLANT_TEMP         3u  /* Temperatura da agua */
#define ADC_CH_AIR_TEMP             4u  /* Temperatura do ar */
#define ADC_CH_PEDAL                5u  /* Pedal do acelerador */
#define ADC_CH_PEDAL_RED            6u  /* Pedal redundante */
#define ADC_CH_VBATT                7u  /* Tensao da bateria */
#define ADC_CH_LAMBDA1              8u  /* Sonda lambda 1 */
#define ADC_CH_LAMBDA2              9u  /* Sonda lambda 2 */
#define ADC_CH_KNOCK                10u /* Sensor de detonacao */
#define ADC_CH_AC_PRESS             11u /* Pressao do A/C */
#define ADC_CH_GENERATOR            12u /* Tensao do alternador */

#define ADC_NUM_CHANNELS            13u

/* ------------------------------------------------------------------ */
/* Constantes                                                          */
/* ------------------------------------------------------------------ */

#define ADC_SUPPLY_VOLTAGE_MV       5000u
#define ADC_MAX_RAW_VALUE           4095u   /* 12-bit ADC */

/* ------------------------------------------------------------------ */
/* API                                                                 */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo ADC e configura os canais
 */
void Adc_Init(void);

/**
 * \brief Le o valor bruto (raw) de um canal ADC (0..4095)
 * \param ch Canal logico (ADC_CH_*)
 * \return Valor ADC raw (12-bit)
 */
uint16 Adc_ReadChannel_Raw(Adc_ChannelType ch);

/**
 * \brief Le o valor de um canal ADC convertido para milivolts
 * \param ch Canal logico (ADC_CH_*)
 * \return Tensao em mV (0..5000)
 */
uint32 Adc_ReadChannel_mV(Adc_ChannelType ch);

#endif /* ADC_H */
