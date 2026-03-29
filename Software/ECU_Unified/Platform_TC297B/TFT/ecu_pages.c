#include "ecu_pages.h"
#include "tfthw.h"
#include "tft_touch.h"
#include "tft_graphics.h"
#include "hal_discrete_common.h"
#include "cdd_crankshaft.h"
#include "hal_discrete_inputs.h"
#include "rte_environment.h"
#include "rte_components.h"
#include "IfxPort.h"

/* ======================== Extern Variables ======================== */
extern unsigned long tps_filtered_value;

/* RTE variables defined in rte_environment.c and rte_components.c */
extern volatile short int S_RTE_T_AirTemp;
extern volatile short int S_RTE_T_CoolantTemp;
extern volatile short int S_RTE_deg_SPARKTiming;

/* ======================== Page Definitions ======================== */
#define NUM_PAGES           4
#define STRIP_CHART_SIZE    50  /* 50 samples = 5 seconds at 100ms */

typedef struct {
    const char *title;
    void (*draw)(void);
    void (*update)(void);
} ECU_Page;

/* ======================== Forward Declarations ======================== */
static void page0_draw(void);
static void page0_update(void);
static void page1_draw(void);
static void page1_update(void);
static void page2_draw(void);
static void page2_update(void);
static void page3_draw(void);
static void page3_update(void);

/* ======================== Page Table ======================== */
static const ECU_Page pages[NUM_PAGES] = {
    { "DASHBOARD",  page0_draw, page0_update },
    { "GRAFICOS",   page1_draw, page1_update },
    { "MOTOR",      page2_draw, page2_update },
    { "DEBUG",      page3_draw, page3_update },
};

/* ======================== State ======================== */
static uint8 current_page = 0;
static uint8 page_changed = 1;

/* Strip chart circular buffers for Page 1 */
static uint16 rpm_history[STRIP_CHART_SIZE];
static uint16 tps_history[STRIP_CHART_SIZE];
static uint16 chart_head = 0;
static uint16 chart_count = 0;

/* ======================== Helpers ======================== */

/* Right-align unsigned integer in buffer of given width, pad with spaces */
static void uint32_to_str(uint32 val, char *buf, uint8 width)
{
    int i;
    for (i = width - 1; i >= 0; i--) {
        if (val > 0 || i == (int)(width - 1)) {
            buf[i] = '0' + (char)(val % 10);
            val /= 10;
        } else {
            buf[i] = ' ';
        }
    }
    buf[width] = '\0';
}

/* Right-align signed integer in buffer of given width, pad with spaces */
static void sint32_to_str(sint32 val, char *buf, uint8 width)
{
    uint8 neg = 0;
    uint32 uval;

    if (val < 0) {
        neg = 1;
        uval = (uint32)(-val);
    } else {
        uval = (uint32)val;
    }

    int i;
    for (i = width - 1; i >= 0; i--) {
        if (uval > 0) {
            buf[i] = '0' + (char)(uval % 10);
            uval /= 10;
        } else if (neg) {
            buf[i] = '-';
            neg = 0;
        } else {
            buf[i] = ' ';
        }
    }
    buf[width] = '\0';
}

/* Build a 20-char line: "LABEL: VALUE UNIT   " padded to 20 chars */
static void format_line(char *out, const char *label, const char *value, const char *unit)
{
    int i = 0;
    int j;

    /* Copy label */
    for (j = 0; label[j] != '\0' && i < 20; j++) {
        out[i++] = label[j];
    }
    /* Separator */
    if (i < 20) out[i++] = ':';
    if (i < 20) out[i++] = ' ';
    /* Value */
    for (j = 0; value[j] != '\0' && i < 20; j++) {
        out[i++] = value[j];
    }
    /* Unit */
    if (unit[0] != '\0' && i < 20) {
        out[i++] = ' ';
        for (j = 0; unit[j] != '\0' && i < 20; j++) {
            out[i++] = unit[j];
        }
    }
    /* Pad to 20 chars */
    while (i < 20) {
        out[i++] = ' ';
    }
    out[20] = '\0';
}

/* Draw common title bar on LINE0 with blue background */
static void draw_title_bar(const char *text)
{
    GLCD_setBackColor(COLOR_BLUE);
    GLCD_setTextColor(COLOR_WHITE);
    GLCD_displayStringLn(LINE0, text);
    GLCD_setBackColor(COLOR_BLACK);
}

/* Draw navigation bar on LINE9 */
static void draw_nav_bar(const char *page_name)
{
    char nav[21];
    int i = 0;
    int j;

    /* "[<<] " = 5 chars */
    nav[i++] = '['; nav[i++] = '<'; nav[i++] = '<';
    nav[i++] = ']'; nav[i++] = ' ';

    /* Center page name */
    int name_len = 0;
    for (j = 0; page_name[j] != '\0'; j++) name_len++;

    int pad_left = (10 - name_len) / 2;
    int pad_right = 10 - name_len - pad_left;

    for (j = 0; j < pad_left && i < 15; j++) nav[i++] = ' ';
    for (j = 0; page_name[j] != '\0' && i < 15; j++) nav[i++] = page_name[j];
    for (j = 0; j < pad_right && i < 15; j++) nav[i++] = ' ';

    /* " [>>]" = 5 chars */
    nav[i++] = ' '; nav[i++] = '['; nav[i++] = '>';
    nav[i++] = '>'; nav[i++] = ']';
    nav[20] = '\0';

    GLCD_setBackColor(COLOR_DARKGREY);
    GLCD_setTextColor(COLOR_WHITE);
    GLCD_displayStringLn(LINE9, nav);
    GLCD_setBackColor(COLOR_BLACK);
}

/* ================================================================ */
/*                       PAGE 0: DASHBOARD                          */
/* ================================================================ */

static void page0_draw(void)
{
    GLCD_clear(COLOR_BLACK);
    draw_title_bar(" ECU TC297B-POLO 1/4");
    draw_nav_bar("DASHBOARD");

    GLCD_setBackColor(COLOR_BLACK);
    GLCD_setTextColor(COLOR_WHITE);
    GLCD_displayStringLn(LINE1, "RPM:                ");
    GLCD_displayStringLn(LINE2, "TPS:            %   ");
    GLCD_displayStringLn(LINE3, "T_AR:           C   ");
    GLCD_displayStringLn(LINE4, "T_AG:           C   ");
    GLCD_displayStringLn(LINE5, "IGN:            deg ");
    GLCD_displayStringLn(LINE6, "FUEL:               ");
    GLCD_displayStringLn(LINE7, "                    ");
    GLCD_displayStringLn(LINE8, "Estado:             ");
}

static void page0_update(void)
{
    char line[21];
    char val_str[8];
    uint32 rpm;
    uint32 tps_pct;
    sint32 air_temp;
    sint32 coolant_temp;
    sint32 spark;

    rpm = CDD_Get_EngineSpeed_RAW();
    tps_pct = (uint32)tps_filtered_value * 100UL / 4095UL;
    air_temp = (sint32)S_RTE_T_AirTemp / 10;
    coolant_temp = (sint32)S_RTE_T_CoolantTemp / 10;
    spark = (sint32)S_RTE_deg_SPARKTiming / 10;

    /* RPM */
    uint32_to_str(rpm, val_str, 5);
    format_line(line, "RPM", val_str, "rpm");
    GLCD_setTextColor(COLOR_GREEN);
    GLCD_displayStringLn(LINE1, line);

    /* TPS */
    uint32_to_str(tps_pct, val_str, 3);
    format_line(line, "TPS", val_str, "%");
    GLCD_setTextColor(COLOR_CYAN);
    GLCD_displayStringLn(LINE2, line);

    /* Air Temp */
    sint32_to_str(air_temp, val_str, 4);
    format_line(line, "T_AR", val_str, "C");
    GLCD_setTextColor(COLOR_YELLOW);
    GLCD_displayStringLn(LINE3, line);

    /* Coolant Temp */
    sint32_to_str(coolant_temp, val_str, 4);
    format_line(line, "T_AG", val_str, "C");
    if (coolant_temp > 100) {
        GLCD_setTextColor(COLOR_RED);
    } else {
        GLCD_setTextColor(COLOR_YELLOW);
    }
    GLCD_displayStringLn(LINE4, line);

    /* Spark */
    sint32_to_str(spark, val_str, 4);
    format_line(line, "IGN", val_str, "deg");
    GLCD_setTextColor(COLOR_WHITE);
    GLCD_displayStringLn(LINE5, line);

    /* Fuel pump status */
    if (HAL_DISCRETE_Get_IgnitionOn()) {
        format_line(line, "FUEL", "ON", "");
        GLCD_setTextColor(COLOR_GREEN);
    } else {
        format_line(line, "FUEL", "OFF", "");
        GLCD_setTextColor(COLOR_RED);
    }
    GLCD_displayStringLn(LINE6, line);

    /* Engine state */
    GLCD_setTextColor(COLOR_WHITE);
    if (rpm > 0) {
        format_line(line, "Estado", "RUNNING", "");
        GLCD_setTextColor(COLOR_GREEN);
    } else if (HAL_DISCRETE_Get_IgnitionOn()) {
        format_line(line, "Estado", "IGN ON", "");
        GLCD_setTextColor(COLOR_YELLOW);
    } else {
        format_line(line, "Estado", "STANDBY", "");
        GLCD_setTextColor(COLOR_DARKGREY);
    }
    GLCD_displayStringLn(LINE8, line);
}

/* ================================================================ */
/*                     PAGE 1: GRAFICOS (Charts)                    */
/* ================================================================ */

static void page1_draw(void)
{
    GLCD_clear(COLOR_BLACK);
    draw_title_bar(" GRAFICOS        2/4");
    draw_nav_bar("GRAFICOS");

    GLCD_setBackColor(COLOR_BLACK);
    GLCD_setTextColor(COLOR_GREEN);
    GLCD_displayStringLn(LINE1, "RPM                 ");

    GLCD_setTextColor(COLOR_CYAN);
    GLCD_displayStringLn(LINE5, "TPS                 ");
}

static void page1_update(void)
{
    uint32 rpm = CDD_Get_EngineSpeed_RAW();
    uint32 tps_pct = (uint32)tps_filtered_value * 100UL / 4095UL;

    /* Add to circular buffers */
    rpm_history[chart_head] = (uint16)((rpm > 8000) ? 8000 : rpm);
    tps_history[chart_head] = (uint16)((tps_pct > 100) ? 100 : tps_pct);
    chart_head = (chart_head + 1) % STRIP_CHART_SIZE;
    if (chart_count < STRIP_CHART_SIZE) chart_count++;

    /* RPM label + value above chart */
    {
        char line[21];
        char val_str[8];
        uint32_to_str(rpm, val_str, 5);
        format_line(line, "RPM", val_str, "");
        GLCD_setTextColor(COLOR_GREEN);
        GLCD_displayStringLn(LINE1, line);
    }

    /* RPM strip chart: y=48, h=72 */
    GFX_DrawStripChart(0, 48, 320, 72,
                       rpm_history, chart_count, chart_head,
                       8000, COLOR_GREEN, COLOR_BLACK);

    /* TPS label + value above chart */
    {
        char line[21];
        char val_str[8];
        uint32_to_str(tps_pct, val_str, 3);
        format_line(line, "TPS", val_str, "%");
        GLCD_setTextColor(COLOR_CYAN);
        GLCD_displayStringLn(LINE5, line);
    }

    /* TPS strip chart: y=144, h=72 */
    GFX_DrawStripChart(0, 144, 320, 72,
                       tps_history, chart_count, chart_head,
                       100, COLOR_CYAN, COLOR_BLACK);
}

/* ================================================================ */
/*                    PAGE 2: MOTOR (Engine Details)                 */
/* ================================================================ */

static void page2_draw(void)
{
    GLCD_clear(COLOR_BLACK);
    draw_title_bar(" MOTOR           3/4");
    draw_nav_bar("MOTOR");

    GLCD_setBackColor(COLOR_BLACK);
    GLCD_setTextColor(COLOR_WHITE);
    GLCD_displayStringLn(LINE1, "Dente atual:        ");
    GLCD_displayStringLn(LINE2, "Tempo dente:      us");
    GLCD_displayStringLn(LINE3, "Tempo filtr:      us");
    GLCD_displayStringLn(LINE4, "Eventos cil:        ");
    GLCD_displayStringLn(LINE5, "RPM:                ");
    GLCD_displayStringLn(LINE6, "Phase:              ");
    GLCD_displayStringLn(LINE7, "                    ");
    GLCD_displayStringLn(LINE8, "                    ");
}

static void page2_update(void)
{
    char line[21];
    char val_str[8];

    /* Dente atual */
    uint32_to_str((uint32)CDD_Get_CurrentTooth(), val_str, 3);
    format_line(line, "Dente atual", val_str, "");
    GLCD_setTextColor(COLOR_WHITE);
    GLCD_displayStringLn(LINE1, line);

    /* Tempo dente */
    uint32_to_str(CDD_Get_LastToothTime(), val_str, 6);
    format_line(line, "Tempo dente", val_str, "us");
    GLCD_displayStringLn(LINE2, line);

    /* Tempo filtrado */
    uint32_to_str((uint32)CDD_Get_LastToothTime_us_Filtered(), val_str, 6);
    format_line(line, "Tempo filtr", val_str, "us");
    GLCD_displayStringLn(LINE3, line);

    /* Eventos cilindro */
    uint32_to_str((uint32)CDD_Get_CylinderEvents(), val_str, 6);
    format_line(line, "Eventos cil", val_str, "");
    GLCD_displayStringLn(LINE4, line);

    /* RPM */
    uint32_to_str(CDD_Get_EngineSpeed_RAW(), val_str, 5);
    format_line(line, "RPM", val_str, "rpm");
    GLCD_setTextColor(COLOR_GREEN);
    GLCD_displayStringLn(LINE5, line);

    /* Phase */
    if (HAL_DISCRETE_Get_PhaseState()) {
        format_line(line, "Phase", "HIGH", "");
        GLCD_setTextColor(COLOR_GREEN);
    } else {
        format_line(line, "Phase", "LOW", "");
        GLCD_setTextColor(COLOR_YELLOW);
    }
    GLCD_displayStringLn(LINE6, line);
}

/* ================================================================ */
/*                         PAGE 3: DEBUG                            */
/* ================================================================ */

static void page3_draw(void)
{
    GLCD_clear(COLOR_BLACK);
    draw_title_bar(" DEBUG           4/4");
    draw_nav_bar("DEBUG");

    GLCD_setBackColor(COLOR_BLACK);
    GLCD_setTextColor(COLOR_WHITE);
    GLCD_displayStringLn(LINE1, "ADC TPS:            ");
    GLCD_displayStringLn(LINE2, "RPM raw:            ");
    GLCD_displayStringLn(LINE3, "Chave IGN:          ");
    GLCD_displayStringLn(LINE4, "Fase:               ");
    GLCD_displayStringLn(LINE5, "Dente:              ");
    GLCD_displayStringLn(LINE6, "Eventos:            ");
    GLCD_displayStringLn(LINE7, "Spark:          deg ");
    GLCD_displayStringLn(LINE8, "                    ");
}

static void page3_update(void)
{
    char line[21];
    char val_str[8];

    /* ADC TPS raw */
    uint32_to_str((uint32)tps_filtered_value, val_str, 5);
    format_line(line, "ADC TPS", val_str, "");
    GLCD_setTextColor(COLOR_WHITE);
    GLCD_displayStringLn(LINE1, line);

    /* RPM raw */
    uint32_to_str(CDD_Get_EngineSpeed_RAW(), val_str, 5);
    format_line(line, "RPM raw", val_str, "");
    GLCD_displayStringLn(LINE2, line);

    /* Ignition key */
    if (HAL_DISCRETE_Get_IgnitionOn()) {
        format_line(line, "Chave IGN", "ON", "");
        GLCD_setTextColor(COLOR_GREEN);
    } else {
        format_line(line, "Chave IGN", "OFF", "");
        GLCD_setTextColor(COLOR_RED);
    }
    GLCD_displayStringLn(LINE3, line);

    /* Phase */
    GLCD_setTextColor(COLOR_WHITE);
    if (HAL_DISCRETE_Get_PhaseState()) {
        format_line(line, "Fase", "HIGH", "");
    } else {
        format_line(line, "Fase", "LOW", "");
    }
    GLCD_displayStringLn(LINE4, line);

    /* Current tooth */
    uint32_to_str((uint32)CDD_Get_CurrentTooth(), val_str, 3);
    format_line(line, "Dente", val_str, "");
    GLCD_displayStringLn(LINE5, line);

    /* Cylinder events */
    uint32_to_str((uint32)CDD_Get_CylinderEvents(), val_str, 6);
    format_line(line, "Eventos", val_str, "");
    GLCD_displayStringLn(LINE6, line);

    /* Spark timing */
    {
        sint32 spark = (sint32)S_RTE_deg_SPARKTiming / 10;
        sint32_to_str(spark, val_str, 4);
        format_line(line, "Spark", val_str, "deg");
        GLCD_displayStringLn(LINE7, line);
    }

    /* Spark timing already shown on LINE7 above */
}

/* ================================================================ */
/*                         PUBLIC API                               */
/* ================================================================ */

void ECU_Pages_Init(void)
{
    /* Configure button pins as inputs with pull-up */
    IfxPort_setPinModeInput(BUTTON_1, IfxPort_InputMode_pullUp);
    IfxPort_setPinModeInput(BUTTON_2, IfxPort_InputMode_pullUp);

    /* Splash screen */
    GLCD_clear(COLOR_BLACK);
    GLCD_setBackColor(COLOR_BLUE);
    GLCD_setTextColor(COLOR_WHITE);
    GLCD_displayStringLn(LINE4, "  ECU TC297B - POLO ");
    GLCD_displayStringLn(LINE5, "   Initializing...  ");
    GLCD_setBackColor(COLOR_BLACK);

    /* Initialize chart buffers */
    {
        uint16 i;
        for (i = 0; i < STRIP_CHART_SIZE; i++) {
            rpm_history[i] = 0;
            tps_history[i] = 0;
        }
    }
    chart_head = 0;
    chart_count = 0;

    current_page = 0;
    page_changed = 1;
}

void ECU_Pages_Update(void)
{
    /* Navigation debounce (300ms = 3 calls at 100ms) */
    static uint8 nav_debounce = 0;
    static uint8 btn1_prev = 0;
    static uint8 btn2_prev = 0;

    if (nav_debounce > 0) {
        nav_debounce--;
    }

    if (nav_debounce == 0) {
        /* Physical buttons: active low (pressed = 0) */
        uint8 btn1 = IfxPort_getPinState(BUTTON_1) ? 0u : 1u;
        uint8 btn2 = IfxPort_getPinState(BUTTON_2) ? 0u : 1u;

        /* BUTTON_1: previous page (rising edge) */
        if (btn1 && !btn1_prev) {
            current_page = (current_page == 0) ? (NUM_PAGES - 1) : (current_page - 1);
            page_changed = 1;
            nav_debounce = 3;
        }
        /* BUTTON_2: next page (rising edge) */
        else if (btn2 && !btn2_prev) {
            current_page = (current_page + 1) % NUM_PAGES;
            page_changed = 1;
            nav_debounce = 3;
        }
        /* Touch fallback */
        else if (TFT_Touch_IsPressed()) {
            uint8 zone = TFT_Touch_GetZone();
            if (zone == 0) {
                current_page = (current_page == 0) ? (NUM_PAGES - 1) : (current_page - 1);
                page_changed = 1;
            } else if (zone == 1) {
                current_page = (current_page + 1) % NUM_PAGES;
                page_changed = 1;
            }
            nav_debounce = 3;
        }

        btn1_prev = btn1;
        btn2_prev = btn2;
    }

    if (page_changed) {
        pages[current_page].draw();
        page_changed = 0;
    }

    pages[current_page].update();
}
