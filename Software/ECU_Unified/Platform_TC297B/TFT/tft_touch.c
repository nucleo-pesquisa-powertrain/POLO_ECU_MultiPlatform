/**
 * \file tft_touch.c
 * \brief XPT2046 resistive touch controller driver.
 *
 * Creates a second SPI channel on QSPI0 (shared with TFT) using SLSO9 (P20.3)
 * as chip select. The INT pin (P20.0, active low) is polled for press detection.
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "Ifx_Types.h"
#include "IfxPort.h"
#include "IfxQspi_SpiMaster.h"
#include "tft_config.h"
#include "tft_touch.h"

/******************************************************************************/
/*----------------------------------Defines-----------------------------------*/
/******************************************************************************/

/* XPT2046 control bytes (differential mode, 12-bit) */
#define XPT2046_CMD_READ_X      0xD0u
#define XPT2046_CMD_READ_Y      0x90u

/* Calibration: raw ADC range mapped to screen pixels */
#define TOUCH_RAW_X_MIN         300u
#define TOUCH_RAW_X_MAX         3800u
#define TOUCH_RAW_Y_MIN         300u
#define TOUCH_RAW_Y_MAX         3800u

#define TOUCH_SCREEN_W          320u
#define TOUCH_SCREEN_H          240u

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/

/* QSPI0 master - initialised in Qspi0.c */
extern IfxQspi_SpiMaster spi0Master;

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/

/* Place variables in the same CPU0 section as the rest of the TFT driver */
#if TOUCH_VAR_LOCATION == 0
    #if defined(__GNUC__)
    #pragma section ".bss_cpu0" awc0
    #endif
    #if defined(__TASKING__)
    #pragma section farbss "bss_cpu0"
    #pragma section fardata "data_cpu0"
    #endif
    #if defined(__DCC__)
    #pragma section DATA ".data_cpu0" ".bss_cpu0" far-absolute RW
    #endif
#endif

static IfxQspi_SpiMaster_Channel touchSpiChannel;

#if defined(__GNUC__)
#pragma section
#endif
#if defined(__TASKING__)
#pragma section farbss restore
#pragma section fardata restore
#endif
#if defined(__DCC__)
#pragma section DATA RW
#endif

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/

void TFT_Touch_Init(void)
{
    boolean interruptState = IfxCpu_disableInterrupts();

    /* Configure P20.0 (TOUCH_INT) as input with pull-up */
    IfxPort_setPinMode(TOUCH_USE_INT.port, TOUCH_USE_INT.pinIndex,
                       IfxPort_Mode_inputPullUp);

    /* Create SPI channel for the touch controller on the existing QSPI0 master */
    IfxQspi_SpiMaster_ChannelConfig chCfg;
    IfxQspi_SpiMaster_initChannelConfig(&chCfg, &spi0Master);

    chCfg.ch.baudrate                = 1000000;   /* 1 MHz for touch */
    chCfg.ch.mode.dataWidth          = 8;          /* 8-bit transfers */
    chCfg.ch.mode.csLeadDelay        = 2;
    chCfg.ch.mode.csTrailDelay       = 2;
    chCfg.ch.mode.csInactiveDelay    = 2;
    chCfg.ch.mode.shiftClock         = IfxQspi_ShiftClock_shiftTransmitDataOnLeadingEdge; /* SPI Mode 0 */

    const IfxQspi_SpiMaster_Output slsOutput = {
        &TOUCH_USE_CHIPSELECT,
        IfxPort_OutputMode_pushPull,
        IfxPort_PadDriver_cmosAutomotiveSpeed1
    };
    chCfg.sls.output.pin    = slsOutput.pin;
    chCfg.sls.output.mode   = slsOutput.mode;
    chCfg.sls.output.driver = slsOutput.driver;

    IfxQspi_SpiMaster_initChannel(&touchSpiChannel, &chCfg);

    IfxCpu_restoreInterrupts(interruptState);
}

uint8 TFT_Touch_IsPressed(void)
{
    /* INT pin is active low: LOW = touched */
    return (IfxPort_getPinState(TOUCH_USE_INT.port, TOUCH_USE_INT.pinIndex) == FALSE) ? 1u : 0u;
}

/**
 * \brief Read a single 12-bit value from the XPT2046.
 * \param cmd  Control byte (XPT2046_CMD_READ_X or XPT2046_CMD_READ_Y).
 * \return 12-bit ADC result (0..4095).
 */
static uint16 touch_read_channel(uint8 cmd)
{
    uint8 txBuf[3];
    uint8 rxBuf[3];

    txBuf[0] = cmd;
    txBuf[1] = 0x00u;
    txBuf[2] = 0x00u;
    rxBuf[0] = 0x00u;
    rxBuf[1] = 0x00u;
    rxBuf[2] = 0x00u;

    /* Wait for any previous transfer to finish */
    while (IfxQspi_SpiMaster_getStatus(&touchSpiChannel) == IfxQspi_Status_busy) {}

    IfxQspi_SpiMaster_exchange(&touchSpiChannel, txBuf, rxBuf, 3);

    while (IfxQspi_SpiMaster_getStatus(&touchSpiChannel) == IfxQspi_Status_busy) {}

    /* 12-bit result sits in bits [14:3] of the 16-bit response (bytes 1-2) */
    uint16 raw = (uint16)(((uint16)rxBuf[1] << 8) | rxBuf[2]);
    raw >>= 3;
    raw &= 0x0FFFu;

    return raw;
}

void TFT_Touch_ReadXY(uint16 *x, uint16 *y)
{
    uint16 rawX = touch_read_channel(XPT2046_CMD_READ_X);
    uint16 rawY = touch_read_channel(XPT2046_CMD_READ_Y);

    /* Clamp to calibration range */
    if (rawX < TOUCH_RAW_X_MIN) rawX = TOUCH_RAW_X_MIN;
    if (rawX > TOUCH_RAW_X_MAX) rawX = TOUCH_RAW_X_MAX;
    if (rawY < TOUCH_RAW_Y_MIN) rawY = TOUCH_RAW_Y_MIN;
    if (rawY > TOUCH_RAW_Y_MAX) rawY = TOUCH_RAW_Y_MAX;

    /* Linear mapping to screen coordinates */
    *x = (uint16)(((uint32)(rawX - TOUCH_RAW_X_MIN) * (TOUCH_SCREEN_W - 1u))
                  / (TOUCH_RAW_X_MAX - TOUCH_RAW_X_MIN));
    *y = (uint16)(((uint32)(rawY - TOUCH_RAW_Y_MIN) * (TOUCH_SCREEN_H - 1u))
                  / (TOUCH_RAW_Y_MAX - TOUCH_RAW_Y_MIN));
}

uint8 TFT_Touch_GetZone(void)
{
    if (!TFT_Touch_IsPressed())
    {
        return 0xFFu;  /* Not pressed */
    }

    uint16 sx, sy;
    TFT_Touch_ReadXY(&sx, &sy);

    /* Simple left/right split at midpoint */
    if (sx < (TOUCH_SCREEN_W / 2u))
    {
        return 0u;  /* Left half  -> page previous */
    }
    else
    {
        return 1u;  /* Right half -> page next */
    }
}
