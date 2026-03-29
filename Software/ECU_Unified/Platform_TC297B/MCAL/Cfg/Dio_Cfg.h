/**
 * \file Dio_Cfg.h
 * \brief Mapeamento de canais logicos de DIO para pinos fisicos do TC297B
 *
 * Cada entrada da tabela associa um canal logico (DIO_CH_*) definido em
 * Dio.h ao modulo de porta (Ifx_P*) e ao indice do pino dentro daquele
 * modulo. O array Dio_PinMap[] e' declarado aqui e definido em Dio.c.
 *
 * Plataforma: Infineon AURIX TC297B
 * Driver iLLD: IfxPort_setPinState / IfxPort_getPinState
 */

#ifndef DIO_CFG_H
#define DIO_CFG_H

/* ------------------------------------------------------------------ */
/* Includes                                                            */
/* ------------------------------------------------------------------ */

#include "Platform_Types.h"

/* iLLD - definicoes de Ifx_P e IfxPort_PinMode */
#include "IfxPort.h"
#include "IfxPort_reg.h"

/* ------------------------------------------------------------------ */
/* Tipo: entrada da tabela de mapeamento de pino                       */
/* ------------------------------------------------------------------ */

/**
 * \brief Associacao entre canal logico e pino fisico do TC297B
 *
 * \note Manter a mesma ordem do enum DIO_CH_* definido em Dio.h para
 *       que o canal logico seja usado diretamente como indice do array.
 */
typedef struct
{
    Ifx_P*  port;       /**< Ponteiro para o registrador do modulo de porta (ex: &MODULE_P02) */
    uint8   pinIndex;   /**< Numero do pino dentro do modulo (0..15 tipicamente)               */
    uint8   isOutput;   /**< 1 = saida, 0 = entrada (usado em Dio_Init)                        */
} Dio_PinMapEntry;

/* ------------------------------------------------------------------ */
/* Declaracao do array de mapeamento (definido em Dio.c)               */
/* ------------------------------------------------------------------ */

/**
 * \brief Tabela de mapeamento: indice = canal logico DIO_CH_*
 *
 * Tamanho: DIO_NUM_CHANNELS (37 entradas)
 */
extern const Dio_PinMapEntry Dio_PinMap[];

/* ------------------------------------------------------------------ */
/* Mapeamento fisico - TC297B                                          */
/* ------------------------------------------------------------------ */
/*
 * Canal logico           Modulo      Pino  Direcao
 * -------------------------------------------------------
 * DIO_CH_INJECTOR1       P02         1     Saida
 * DIO_CH_INJECTOR2       P02         3     Saida
 * DIO_CH_INJECTOR3       P10         6     Saida
 * DIO_CH_INJECTOR4       P10         4     Saida
 * DIO_CH_COIL1           P33         10    Saida
 * DIO_CH_COIL2           P00         4     Saida
 * DIO_CH_FUEL_PUMP       P22         2     Saida
 * DIO_CH_COLDSTART_RELAY P00         12    Saida
 * DIO_CH_FAN_LOW         P00         8     Saida
 * DIO_CH_FAN_HIGH        P00         10    Saida
 * DIO_CH_LAMBDA_HEATER1  P00         1     Saida
 * DIO_CH_LAMBDA_HEATER2  P02         8     Saida
 * DIO_CH_MC33810_CS      P22         3     Saida
 * DIO_CH_MC33810_ENOUT   P10         7     Saida
 * DIO_CH_MC33186_DI1     P02         6     Saida
 * DIO_CH_MC33186_DI2     P00         7     Saida
 * DIO_CH_MC33186_COD     P02         4     Saida
 * DIO_CH_MC33186_IN1     P00         11    Saida
 * DIO_CH_GIN0            P00         0     Saida
 * DIO_CH_GIN1            P00         2     Saida
 * DIO_CH_GIN2            P00         4     Saida  (compartilha com COIL2 - verificar HW)
 * DIO_CH_GIN3            P00         6     Saida
 * DIO_CH_IGNITION_ON     P23         4     Entrada
 * DIO_CH_PHASE_STATE     P14         8     Entrada
 * DIO_CH_BRAKE_SW1       P14         4     Entrada
 * DIO_CH_BRAKE_SW2       P15         4     Entrada
 * DIO_CH_CLUTCH_SW       P15         6     Entrada
 * DIO_CH_AC_SW           P14         7     Entrada
 * DIO_CH_MC33186_SF      P00         7     Entrada (compartilha com DI2 - verificar HW)
 * DIO_CH_LED1            P33         4     Saida
 * DIO_CH_LED2            P33         2     Saida
 * DIO_CH_LED3            P33         12    Saida
 * DIO_CH_LED4            P33         6     Saida
 * DIO_CH_LED_HIGH        P13         0     Saida
 * DIO_CH_LED_MID         P13         1     Saida
 * DIO_CH_LED_LOW         P13         2     Saida
 * DIO_CH_LED_GND         P13         3     Saida
 */

#endif /* DIO_CFG_H */
