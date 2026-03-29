/**
 * \file tft_graphics.c
 * \brief Graphics primitives and widgets for the ILI9341 TFT display.
 *
 * Uses tft_display_setxy() and tft_flush_row_buff() from tfthw for fast
 * rectangle fills, and the GLCD_* API for text rendering.
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "Ifx_Types.h"
#include "tfthw.h"
#include "tft_graphics.h"

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/

/* Row buffer for fast rectangle fills (one row, max 320 pixels) */
static uint16 gfx_row_buf[320];

/******************************************************************************/
/*---------------------------Helper functions---------------------------------*/
/******************************************************************************/

/**
 * \brief Wait until tft_status indicates the display is idle.
 *
 * tft_flush_row_buff() sets tft_status = 1 while a DMA transfer is in
 * progress. We must wait for completion before issuing the next command.
 */
static void gfx_wait_tft_ready(void)
{
    while (tft_status != 0u) {}
}

/**
 * \brief Reset the ILI9341 address window to full screen.
 *
 * tft_display_setxy() restricts the column/page window. GLCD text functions
 * only set the START address, so the END stays restricted. We must restore
 * the full window after any GFX operation to avoid corrupting text output.
 */
static void gfx_reset_window(void)
{
    gfx_wait_tft_ready();
    tft_display_setxy(0, 0, TFT_XSIZE - 1u, TFT_YSIZE - 1u);
}

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/

/* ---------- Primitives --------------------------------------------------- */

void GFX_FillRect(uint16 x, uint16 y, uint16 w, uint16 h, uint16 color)
{
    uint16 row;
    uint16 i;

    if ((w == 0u) || (h == 0u))
    {
        return;
    }

    /* Cap to screen bounds */
    if (x + w > TFT_XSIZE) w = TFT_XSIZE - x;
    if (y + h > TFT_YSIZE) h = TFT_YSIZE - y;

    /* Fill the row buffer with the requested colour */
    for (i = 0u; i < w; i++)
    {
        gfx_row_buf[i] = color;
    }

    /* Set the pixel window */
    gfx_wait_tft_ready();
    tft_display_setxy(x, y, x + w - 1u, y + h - 1u);

    /* Flush row-by-row */
    for (row = 0u; row < h; row++)
    {
        gfx_wait_tft_ready();
        tft_flush_row_buff(w, gfx_row_buf);
    }

    gfx_wait_tft_ready();
    tft_status = 0;

    /* Restore full-screen window so GLCD text functions work correctly */
    gfx_reset_window();
}

void GFX_DrawHLine(uint16 x, uint16 y, uint16 w, uint16 color)
{
    GFX_FillRect(x, y, w, 1u, color);
}

void GFX_DrawVLine(uint16 x, uint16 y, uint16 h, uint16 color)
{
    GFX_FillRect(x, y, 1u, h, color);
}

void GFX_DrawRect(uint16 x, uint16 y, uint16 w, uint16 h, uint16 color)
{
    if ((w < 2u) || (h < 2u))
    {
        GFX_FillRect(x, y, w, h, color);
        return;
    }

    GFX_DrawHLine(x, y, w, color);                  /* top    */
    GFX_DrawHLine(x, y + h - 1u, w, color);         /* bottom */
    GFX_DrawVLine(x, y + 1u, h - 2u, color);        /* left   */
    GFX_DrawVLine(x + w - 1u, y + 1u, h - 2u, color); /* right  */
}

/* ---------- Horizontal bar gauge ----------------------------------------- */

void GFX_DrawBarH(uint16 x, uint16 y, uint16 w, uint16 h,
                  uint16 value, uint16 max_value,
                  uint16 bar_color, uint16 bg_color)
{
    uint16 fill_w;

    if (max_value == 0u)
    {
        return;
    }

    if (value > max_value)
    {
        value = max_value;
    }

    fill_w = (uint16)(((uint32)value * (uint32)w) / (uint32)max_value);

    /* Filled portion */
    if (fill_w > 0u)
    {
        GFX_FillRect(x, y, fill_w, h, bar_color);
    }

    /* Empty portion */
    if (fill_w < w)
    {
        GFX_FillRect(x + fill_w, y, w - fill_w, h, bg_color);
    }

    /* Border */
    GFX_DrawRect(x, y, w, h, bar_color);
}

/* ---------- Strip chart -------------------------------------------------- */

void GFX_DrawStripChart(uint16 x, uint16 y, uint16 w, uint16 h,
                        uint16 *data, uint16 count, uint16 head,
                        uint16 max_val,
                        uint16 line_color, uint16 bg_color)
{
    uint16 col;
    uint16 samples_to_draw;
    uint16 bar_h;
    uint16 idx;
    uint16 col_w;    /* pixel width per sample */
    uint16 px;       /* pixel x position */

    if ((w == 0u) || (h == 0u) || (max_val == 0u))
    {
        return;
    }

    /* Clear background */
    GFX_FillRect(x, y, w, h, bg_color);

    samples_to_draw = count;
    if (samples_to_draw == 0u)
    {
        GFX_DrawRect(x, y, w, h, line_color);
        return;
    }

    /* Scale: each sample occupies w/count pixels (e.g. 320/160 = 2px) */
    col_w = w / samples_to_draw;
    if (col_w == 0u)
    {
        col_w = 1u;
        samples_to_draw = w;  /* more samples than pixels: show newest */
    }

    for (col = 0u; col < samples_to_draw; col++)
    {
        /* Read from circular buffer: oldest sample first */
        idx = (head + count - samples_to_draw + col) % count;

        if (data[idx] > max_val)
        {
            bar_h = h;
        }
        else
        {
            bar_h = (uint16)(((uint32)data[idx] * (uint32)(h - 1u)) / (uint32)max_val);
        }

        if (bar_h > 0u)
        {
            px = x + (col * col_w);
            if (col_w == 1u)
            {
                GFX_DrawVLine(px, y + h - bar_h, bar_h, line_color);
            }
            else
            {
                GFX_FillRect(px, y + h - bar_h, col_w, bar_h, line_color);
            }
        }
    }

    /* Border */
    GFX_DrawRect(x, y, w, h, line_color);
}

/* ---------- Formatted number display ------------------------------------- */

/**
 * \brief Append a NUL-terminated string to buf at position *pos.
 */
static void gfx_str_append(char *buf, uint16 *pos, const char *src)
{
    while (*src != '\0')
    {
        buf[*pos] = *src;
        (*pos)++;
        src++;
    }
}

/**
 * \brief Append an unsigned 32-bit integer (decimal) to buf at position *pos.
 */
static void gfx_uint32_to_str(char *buf, uint16 *pos, uint32 val)
{
    char tmp[11]; /* max 10 digits + NUL */
    sint16 i = 0;

    if (val == 0u)
    {
        buf[*pos] = '0';
        (*pos)++;
        return;
    }

    while (val > 0u)
    {
        tmp[i] = (char)('0' + (val % 10u));
        val /= 10u;
        i++;
    }

    /* Reverse into output buffer */
    while (i > 0)
    {
        i--;
        buf[*pos] = tmp[i];
        (*pos)++;
    }
}

/**
 * \brief Pad buffer with spaces up to total width and NUL-terminate.
 */
static void gfx_pad(char *buf, uint16 *pos, uint16 width)
{
    while (*pos < width)
    {
        buf[*pos] = ' ';
        (*pos)++;
    }
    buf[*pos] = '\0';
}

void GFX_DrawValue(uint16 line, const char *label, uint32 value, const char *unit)
{
    char buf[21];
    uint16 pos = 0u;

    gfx_str_append(buf, &pos, label);
    gfx_str_append(buf, &pos, ": ");
    gfx_uint32_to_str(buf, &pos, value);
    buf[pos] = ' ';
    pos++;
    gfx_str_append(buf, &pos, unit);
    gfx_pad(buf, &pos, 20u);

    GLCD_displayStringLn(line, buf);
}

void GFX_DrawValueFloat(uint16 line, const char *label, float value, uint8 decimals, const char *unit)
{
    char buf[21];
    uint16 pos = 0u;
    uint8 d;
    uint32 int_part;
    uint32 frac_part;
    float frac;
    uint32 mult;

    gfx_str_append(buf, &pos, label);
    gfx_str_append(buf, &pos, ": ");

    /* Handle negative */
    if (value < 0.0f)
    {
        buf[pos] = '-';
        pos++;
        value = -value;
    }

    int_part = (uint32)value;
    frac = value - (float)int_part;

    gfx_uint32_to_str(buf, &pos, int_part);

    if (decimals > 0u)
    {
        buf[pos] = '.';
        pos++;

        /* Compute fractional digits */
        mult = 1u;
        for (d = 0u; d < decimals; d++)
        {
            mult *= 10u;
        }
        frac_part = (uint32)(frac * (float)mult + 0.5f);

        /* Leading zeros for fractional part */
        {
            uint32 threshold = mult / 10u;
            while ((threshold > 0u) && (frac_part < threshold))
            {
                buf[pos] = '0';
                pos++;
                threshold /= 10u;
            }
        }

        if (frac_part > 0u)
        {
            gfx_uint32_to_str(buf, &pos, frac_part);
        }
    }

    buf[pos] = ' ';
    pos++;
    gfx_str_append(buf, &pos, unit);
    gfx_pad(buf, &pos, 20u);

    GLCD_displayStringLn(line, buf);
}
