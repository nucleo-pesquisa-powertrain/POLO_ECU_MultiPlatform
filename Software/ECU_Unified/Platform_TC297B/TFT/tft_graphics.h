/**
 * \file tft_graphics.h
 * \brief Graphics primitives and widgets for the ILI9341 TFT display.
 *
 * Built on top of the low-level TFT driver (tfthw.h).
 */

#ifndef TFT_GRAPHICS_H
#define TFT_GRAPHICS_H

#include "Ifx_Types.h"

/* ======================== Primitives ======================== */

void GFX_FillRect(uint16 x, uint16 y, uint16 w, uint16 h, uint16 color);
void GFX_DrawRect(uint16 x, uint16 y, uint16 w, uint16 h, uint16 color);
void GFX_DrawHLine(uint16 x, uint16 y, uint16 w, uint16 color);
void GFX_DrawVLine(uint16 x, uint16 y, uint16 h, uint16 color);

/* ======================== Widgets ======================== */

/**
 * \brief Draw a horizontal bar gauge.
 * \param x, y       Top-left corner.
 * \param w, h       Outer dimensions.
 * \param value      Current value.
 * \param max_value  Full-scale value.
 * \param bar_color  Fill colour.
 * \param bg_color   Background colour.
 */
void GFX_DrawBarH(uint16 x, uint16 y, uint16 w, uint16 h,
                  uint16 value, uint16 max_value,
                  uint16 bar_color, uint16 bg_color);

/**
 * \brief Draw a strip chart (history graph with circular buffer).
 * \param x, y       Top-left corner.
 * \param w, h       Chart dimensions.
 * \param data       Circular buffer of sample values.
 * \param count      Number of valid samples in the buffer.
 * \param head       Write index into the circular buffer.
 * \param max_val    Full-scale value for Y axis.
 * \param line_color Colour for the data trace.
 * \param bg_color   Background colour.
 */
void GFX_DrawStripChart(uint16 x, uint16 y, uint16 w, uint16 h,
                        uint16 *data, uint16 count, uint16 head,
                        uint16 max_val,
                        uint16 line_color, uint16 bg_color);

/* ======================== Formatted numbers ======================== */

/**
 * \brief Display "LABEL: VALUE UNIT" on a text line (integer).
 */
void GFX_DrawValue(uint16 line, const char *label, uint32 value, const char *unit);

/**
 * \brief Display "LABEL: VALUE UNIT" on a text line (float with decimals).
 */
void GFX_DrawValueFloat(uint16 line, const char *label, float value, uint8 decimals, const char *unit);

#endif /* TFT_GRAPHICS_H */
