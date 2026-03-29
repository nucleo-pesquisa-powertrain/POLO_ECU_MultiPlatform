/**
 * \file tft_config.h
 * \brief TFT display configuration for ECU TC297B project.
 *
 * Pin assignments, ISR priorities and DMA channels for QSPI0 (TFT + Touch).
 * Extracted from the reference project Configuration.h / ConfigurationIsr.h.
 */

#ifndef TFT_CONFIG_H
#define TFT_CONFIG_H

#include "Ifx_Cfg.h"
#include <_PinMap/IfxQspi_PinMap.h>
#include <_PinMap/IfxPort_PinMap.h>
#include "Qspi0.h"

/* ======================== QSPI0 Pin Configuration ======================== */
#define QSPI0_MAX_BAUDRATE          50000000
#define QSPI0_SCLK_PIN              IfxQspi0_SCLK_P20_11_OUT
#define QSPI0_MTSR_PIN              IfxQspi0_MTSR_P20_14_OUT
#define QSPI0_MRST_PIN              IfxQspi0_MRSTA_P20_12_IN
#define QSPI0_USE_DMA

/* ======================== DMA Channels ======================== */
#define TFT_DMA_CH_TXBUFF_TO_TXFIFO     0
#define TFT_DMA_CH_RXBUFF_FROM_RXFIFO   1
#define DMA_CH_QSPI0_TX                 TFT_DMA_CH_TXBUFF_TO_TXFIFO
#define DMA_CH_QSPI0_RX                 TFT_DMA_CH_RXBUFF_FROM_RXFIFO

/* ======================== TFT Display (ILI9341) ======================== */
#define TFT_QSPI_INIT               qspi0_init
#define TFT_USE_CHIPSELECT           IfxQspi0_SLSO8_P20_6_OUT
#define TFT_USE_SCLK                 QSPI0_SCLK_PIN
#define TFT_DISPLAY_VAR_LOCATION     0   /* CPU0 */

/* ======================== Touch (XPT2046) ======================== */
#define TOUCH_QSPI_INIT             qspi0_init
#define TOUCH_USE_CHIPSELECT         IfxQspi0_SLSO9_P20_3_OUT
#define TOUCH_USE_INT                IfxPort_P20_0
#define TOUCH_VAR_LOCATION           0   /* CPU0 */

/* ======================== Backlight ======================== */
#define BGL_PORT                     &MODULE_P20
#define BGL_PIN                      13

/* ======================== ISR Priorities ======================== */
/*
 * Current project ISR usage:
 *   STM tick        = 10
 *   GPT12 T6        = 6
 *   ATOM (injectors)= 20
 *   QSPI2 (MC33810) = 50-55
 *   ERU (crankshaft) = 40
 *
 * QSPI0 priorities chosen to avoid conflicts:
 */
#define ISR_PRIORITY_QSPI0_TX       31
#define ISR_PRIORITY_QSPI0_RX       41
#define ISR_PRIORITY_QSPI0_ER       12
#define ISR_PROVIDER_QSPI0          0    /* CPU0 */

/* ======================== Callback ======================== */
#define QSPI0_TRANSMIT_CALLBACK     tft_transmit_callback

/* ======================== ISR Helper Macros ======================== */
#define ISR_ASSIGN(no, cpu)          ((no << 8) + cpu)
#define ISR_PRIORITY(no_cpu)         (no_cpu >> 8)
#define ISR_PROVIDER(no_cpu)         (no_cpu % 8)

#endif /* TFT_CONFIG_H */
