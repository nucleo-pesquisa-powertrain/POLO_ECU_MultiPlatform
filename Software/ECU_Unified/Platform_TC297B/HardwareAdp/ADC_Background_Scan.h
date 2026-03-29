/*
 * ADC_Background_Scan.h
 *
 *  Created on: 22 de nov de 2020
 *      Author: davic
 */

#ifndef ADC_BACKGROUND_SCAN_H_
#define ADC_BACKGROUND_SCAN_H_

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "Ifx_Types.h"
#include "IfxVadc_Adc.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
#define CHN_0   0                                       /* Position of channel 0 (AN0) on the array of ADC channels */
#define CHN_1   1                                       /* Position of channel 1 (AN1) on the array of ADC channels */
#define CHN_2   2                                       /* Position of channel 2 (AN2) on the array of ADC channels */
#define CHN_3   3                                       /* Position of channel 3 (AN3) on the array of ADC channels */


#define CHANNELS_PER_GROUP  8                                   /* Maximum number of channels per group             */

#define AN0_CHID            0
#define AN2_CHID            2
#define AN3_CHID            3
#define AN8_CHID            0
#define AN16_CHID           0
#define AN17_CHID           1               /* Channel ID for pin AN17, used also as index in the g_adcChannel array*/
#define AN20_CHID           4               /* Channel ID for pin AN20, used also as index in the g_adcChannel array*/
#define AN21_CHID           5               /* Channel ID for pin AN21, used also as index in the g_adcChannel array*/                                
#define AN24_CHID           0
#define AN25_CHID           1
#define AN32_CHID           0
#define AN33_CHID           1
#define AN44_CHID           4



/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
void initADC(void);                                     /* Function to initialize the VADC module  */   

uint16 readADCValue(IfxVadc_Adc_Channel* chn_adc, uint8 channel);

extern IfxVadc_Adc_Channel g_vadcChannel0[CHANNELS_PER_GROUP];            /* Global variable for configuring the VADC channels    */
extern IfxVadc_Adc_Channel g_vadcChannel1[CHANNELS_PER_GROUP];
extern IfxVadc_Adc_Channel g_vadcChannel2[CHANNELS_PER_GROUP];
extern IfxVadc_Adc_Channel g_vadcChannel3[CHANNELS_PER_GROUP];
extern IfxVadc_Adc_Channel g_vadcChannel4[CHANNELS_PER_GROUP];
extern IfxVadc_Adc_Channel g_vadcChannel5[CHANNELS_PER_GROUP];



#endif /* ADC_BACKGROUND_SCAN_H_ */
