/**
 * \file tft_touch.h
 * \brief XPT2046 resistive touch controller driver.
 *
 * Shares QSPI0 with the TFT display using a separate chip select (SLSO9).
 */

#ifndef TFT_TOUCH_H
#define TFT_TOUCH_H

#include "Ifx_Types.h"

/**
 * \brief Initialise the touch controller SPI channel and INT pin.
 *
 * Must be called after tft_init() so that QSPI0 master is already running.
 */
void TFT_Touch_Init(void);

/**
 * \brief Check whether the touch panel is currently pressed.
 * \return 1 if pressed (INT pin LOW), 0 otherwise.
 */
uint8 TFT_Touch_IsPressed(void);

/**
 * \brief Read raw touch coordinates via SPI.
 * \param[out] x  Raw X value (0..4095).
 * \param[out] y  Raw Y value (0..4095).
 */
void TFT_Touch_ReadXY(uint16 *x, uint16 *y);

/**
 * \brief Map touch position to a screen zone for page navigation.
 * \return 0 = left half (page previous), 1 = right half (page next),
 *         2 = unused, 3 = unused, 0xFF = not pressed.
 */
uint8 TFT_Touch_GetZone(void);

#endif /* TFT_TOUCH_H */
