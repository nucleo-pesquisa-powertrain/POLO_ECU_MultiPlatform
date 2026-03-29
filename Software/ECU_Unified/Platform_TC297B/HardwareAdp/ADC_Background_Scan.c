/*
 * ADC_Background_Scan.c
 *
 *  Created on: 22 de nov de 2020
 *      Author: davic
 */

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "ADC_Background_Scan.h"


/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/


/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
void initVADCModule(void);                      /* Function to initialize the VADC module with default parameters   */
void initVADCGroup(IfxVadc_Adc_Group*  g_vadcGroup, IfxVadc_GroupId Group_ID);                       /* Function to initialize the VADC group                            */
void initVADCChannels(IfxVadc_Adc_Group*  g_vadcGroup, IfxVadc_Adc_Channel* g_vadcChannel, uint8 channels_num, uint8* chn_ids);


/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
IfxVadc_Adc g_vadc;                                         /* Global variable for configuring the VADC module      */


IfxVadc_Adc_Group g_vadcGroup0;                              /* Global variable for configuring the VADC group       */
IfxVadc_Adc_Channel g_vadcChannel0[CHANNELS_PER_GROUP];            /* Global variable for configuring the VADC channels    */
uint8 g_grpChannelsId0[] = {AN0_CHID, AN2_CHID, AN3_CHID};    /* AN17, AN20, AN21                                 */

IfxVadc_Adc_Group g_vadcGroup1;                              /* Global variable for configuring the VADC group       */
IfxVadc_Adc_Channel g_vadcChannel1[CHANNELS_PER_GROUP];            /* Global variable for configuring the VADC channels    */
uint8 g_grpChannelsId1[] = {AN8_CHID};

                                         /* Global variable for configuring the VADC module      */
IfxVadc_Adc_Group g_vadcGroup2;                              /* Global variable for configuring the VADC group       */
IfxVadc_Adc_Channel g_vadcChannel2[CHANNELS_PER_GROUP];
uint8 g_grpChannelsId2[] = {AN16_CHID, AN17_CHID, AN20_CHID, AN21_CHID};

IfxVadc_Adc_Group g_vadcGroup3;                              /* Global variable for configuring the VADC group       */
IfxVadc_Adc_Channel g_vadcChannel3[CHANNELS_PER_GROUP];            /* Global variable for configuring the VADC channels    */
uint8 g_grpChannelsId3[] = {AN24_CHID, AN25_CHID};                          

IfxVadc_Adc_Group g_vadcGroup4;                              /* Global variable for configuring the VADC group       */
IfxVadc_Adc_Channel g_vadcChannel4[CHANNELS_PER_GROUP];            /* Global variable for configuring the VADC channels    */
uint8 g_grpChannelsId4[] = {AN32_CHID, AN33_CHID};                          

IfxVadc_Adc_Group g_vadcGroup5;                              /* Global variable for configuring the VADC group       */
IfxVadc_Adc_Channel g_vadcChannel5[CHANNELS_PER_GROUP];            /* Global variable for configuring the VADC channels    */
uint8 g_grpChannelsId5[] = {AN44_CHID};


/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
/* Function to initialize the VADC module */
void initADC(void)
{
    initVADCModule();                                                   /* Initialize the VADC module               */
    
    initVADCGroup(&g_vadcGroup0, IfxVadc_GroupId_0);                                                    /* Initialize the VADC group                */
    initVADCChannels(&g_vadcGroup0, g_vadcChannel0, 3, g_grpChannelsId0);                                                 /* Initialize the used channels             */
    
    initVADCGroup(&g_vadcGroup1, IfxVadc_GroupId_1);                                                    /* Initialize the VADC group                */
    initVADCChannels(&g_vadcGroup1, g_vadcChannel1, 1, g_grpChannelsId1);                                                 /* Initialize the used channels             */
    
    initVADCGroup(&g_vadcGroup2, IfxVadc_GroupId_2);                                                    /* Initialize the VADC group                */
    initVADCChannels(&g_vadcGroup2, g_vadcChannel2, 4, g_grpChannelsId2);                                                 /* Initialize the used channels             */
    
    initVADCGroup(&g_vadcGroup3, IfxVadc_GroupId_3);                                                    /* Initialize the VADC group                */
    initVADCChannels(&g_vadcGroup3, g_vadcChannel3, 2, g_grpChannelsId3);                                                 /* Initialize the used channels             */
    
    initVADCGroup(&g_vadcGroup4, IfxVadc_GroupId_4);                                                    /* Initialize the VADC group                */
    initVADCChannels(&g_vadcGroup4, g_vadcChannel4, 2, g_grpChannelsId4);                                                 /* Initialize the used channels             */

    initVADCGroup(&g_vadcGroup5, IfxVadc_GroupId_5);                                                    /* Initialize the VADC group                */
    initVADCChannels(&g_vadcGroup5, g_vadcChannel5, 1, g_grpChannelsId5);                                                 /* Initialize the used channels             */
    

    /* Start the scan */
    IfxVadc_Adc_startBackgroundScan(&g_vadc);
}

/* Function to initialize the VADC module with default parameters */
void initVADCModule(void)
{
    IfxVadc_Adc_Config adcConf;                                         /* Define a configuration structure         */
    IfxVadc_Adc_initModuleConfig(&adcConf, &MODULE_VADC);               /* Fill it with default values              */
    IfxVadc_Adc_initModule(&g_vadc, &adcConf);                          /* Apply the configuration                  */
}

/* Function to initialize the VADC group */
void initVADCGroup(IfxVadc_Adc_Group*  g_vadcGroup, IfxVadc_GroupId Group_ID)
{
    IfxVadc_Adc_GroupConfig adcGroupConf;                               /* Define a configuration structure         */
    IfxVadc_Adc_initGroupConfig(&adcGroupConf, &g_vadc);                /* Fill it with default values              */

    adcGroupConf.groupId = Group_ID;                           /* Select the Group 0                       */
    adcGroupConf.master = adcGroupConf.groupId;                         /* Set the same group as master group       */

    /* Enable the background scan source and the background auto scan functionality */
    adcGroupConf.arbiter.requestSlotBackgroundScanEnabled = TRUE;
    adcGroupConf.backgroundScanRequest.autoBackgroundScanEnabled = TRUE;

    /* Enable the gate in "always" mode (no edge detection) */
    adcGroupConf.backgroundScanRequest.triggerConfig.gatingMode = IfxVadc_GatingMode_always;

    IfxVadc_Adc_initGroup(g_vadcGroup, &adcGroupConf);                 /* Apply the configuration                  */
}


/* Function to initialize the VADC used channels */
void initVADCChannels(IfxVadc_Adc_Group*  g_vadcGroup, IfxVadc_Adc_Channel* g_vadcChannel, uint8 channels_num, uint8* chn_ids)
{
    IfxVadc_Adc_ChannelConfig adcChannelConf[channels_num];             /* Array of configuration structures        */

    uint16 chn;
    for(chn = 0; chn < channels_num; chn++)                             /* The channels 0..3 are initialized        */
    {
        /* Fill the configuration with default values */
        IfxVadc_Adc_initChannelConfig(&adcChannelConf[chn], g_vadcGroup);

        /* Set the channel ID and the corresponding result register */
        adcChannelConf[chn].channelId = (IfxVadc_ChannelId)(chn_ids[chn]);
        adcChannelConf[chn].resultRegister = (IfxVadc_ChannelResult)(chn_ids[chn]);
        adcChannelConf[chn].backgroundChannel = TRUE;                   /* Enable background scan for the channel   */

        /* Apply the channel configuration */
        IfxVadc_Adc_initChannel(&g_vadcChannel[chn_ids[chn]], &adcChannelConf[chn]);

        /* Add the channel to background scan */
        unsigned chnEnableBit = (1 << adcChannelConf[chn].channelId);   /* Set the the corresponding input channel  */
        unsigned mask = chnEnableBit;                                   /* of the respective group to be added in   */
        IfxVadc_Adc_setBackgroundScan(&g_vadc, g_vadcGroup, chnEnableBit, mask); /* the background scan sequence.  */
    }
}




/* Function to read the VADC measurement */
uint16 readADCValue(IfxVadc_Adc_Channel* chn_adc, uint8 channel)
{
    Ifx_VADC_RES conversionResult;
    do
    {
        conversionResult = IfxVadc_Adc_getResult(&chn_adc[channel]);
    } while(!conversionResult.B.VF);

    return conversionResult.B.RESULT;
}
