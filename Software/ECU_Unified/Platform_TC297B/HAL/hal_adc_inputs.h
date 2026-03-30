#ifndef _HAL_ADC_INPUTS_H
#define _HAL_ADC_INPUTS_H

#include "ADC_Background_Scan.h"

#define SUPPLY_VOLTAGE  5000
#define MAX_ADC_VALUE   4095

#define TBIPOS_CH       g_vadcChannel0,AN0_CHID     //Group 0, Ch 0 OK
#define TBIPOS_RED_CH   g_vadcChannel0,AN2_CHID     //Group 0, Ch 2 OK
#define MAP_CH          g_vadcChannel0,AN3_CHID     //Group 0, Ch 3 OK
#define COOLTEMP_CH     g_vadcChannel1,AN8_CHID     //Group 1, Ch 0 ?
#define PEDAL_RED_CH    g_vadcChannel2,AN16_CHID    //Group 2, Ch 0 ?
#define VBAT_CH         g_vadcChannel2,AN17_CHID    //Group 2, Ch 1 OK
#define LAMBDA2_CH      g_vadcChannel2,AN20_CHID    //Group 2, Ch 4 OK
#define AIRTEMP_CH      g_vadcChannel2,AN21_CHID    //Group 2, Ch 5 OK
#define PEDAL_CH        g_vadcChannel3,AN24_CHID    //Group 3, Ch 0 ?
#define ACPRESS_CH      g_vadcChannel3,AN25_CHID    //Group 3, Ch 1 ?
#define LAMBDA1_CH      g_vadcChannel4,AN32_CHID    //Group 4, Ch 0 ?
#define KNOCK_CH        g_vadcChannel4,AN33_CHID    //Group 4, Ch 1 ?
#define GENERATOR_CH    g_vadcChannel5,AN44_CHID    //Group 5, Ch 4 ?


#define HAL_ADC_Get_V_ThrottlePedal1()            ((uint32)readADCValue(PEDAL_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)
#define HAL_ADC_Get_V_ThrottlePedalRed()          ((uint32)readADCValue(PEDAL_RED_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)
#define HAL_ADC_Get_V_MAP()                       ((uint32)readADCValue(MAP_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)
#define HAL_ADC_Get_V_CoolantTemp()               ((uint32)readADCValue(COOLTEMP_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)
#define HAL_ADC_Get_V_TBIPositionRed()            ((uint32)readADCValue(TBIPOS_RED_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)
#define HAL_ADC_Get_V_Vbatt()                     ((uint32)readADCValue(VBAT_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)
#define HAL_ADC_Get_V_Lambda2()                   ((uint32)readADCValue(LAMBDA2_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)
#define HAL_ADC_Get_V_AirTemp()                   ((uint32)readADCValue(AIRTEMP_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)
#define HAL_ADC_Get_V_TBIPosition()               ((uint32)readADCValue(TBIPOS_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)
#define HAL_ADC_Get_V_ACPress()                   ((uint32)readADCValue(ACPRESS_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)
#define HAL_ADC_Get_V_Lambda1()                   ((uint32)readADCValue(LAMBDA1_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)
#define HAL_ADC_Get_V_Knock()                     ((uint32)readADCValue(KNOCK_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)
#define HAL_ADC_Get_V_Generator()                 ((uint32)readADCValue(GENERATOR_CH)*SUPPLY_VOLTAGE/MAX_ADC_VALUE)

extern unsigned long adc_tps_average;
extern unsigned long adc_counter;

unsigned long TEST_ADC_Get_TBIPosition();

void HAL_ANALOG_ADC_Init(void);


#endif