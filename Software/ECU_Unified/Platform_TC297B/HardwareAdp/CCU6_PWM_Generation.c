/*
 * CCU6_PWM_Generation.c
 *
 *  Created on: 18 de nov de 2020
 *      Author: davic
 */


/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "IfxCcu6_PwmHl.h"
#include "CCU6_PWM_Generation.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
#define CCU6_BASE_FREQUENCY	    100000000                                   /* CCU6 base frequency, in Hertz        */
#define PWM_FREQUENCY           10000                                       /* PWM signal frequency, in Hertz       */
#define PWM_PERIOD              (CCU6_BASE_FREQUENCY / PWM_FREQUENCY)       /* PWM signal period, in ticks          */

#define NUMBER_OF_CHANNELS      3

#define CHANNEL1_DUTY_CYCLE     5                                          /* PWM Signal 1 Duty cycle, in percent  */
#define CHANNEL2_DUTY_CYCLE     50                                          /* PWM Signal 2 Duty cycle, in percent  */
#define CHANNEL3_DUTY_CYCLE     75                                          /* PWM Signal 3 Duty cycle, in percent  */

#define CHANNEL1_COMPARE_VALUE  ((PWM_PERIOD / 100) * (100 - CHANNEL1_DUTY_CYCLE))
#define CHANNEL2_COMPARE_VALUE  ((PWM_PERIOD / 100) * (100 - CHANNEL2_DUTY_CYCLE))
#define CHANNEL3_COMPARE_VALUE  ((PWM_PERIOD / 100) * (100 - CHANNEL3_DUTY_CYCLE))

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
IfxCcu6_TimerWithTrigger g_timer;
IfxCcu6_PwmHl g_driver;

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
/* Function to initialize the CCU6 module to generate PWM signals */
void initCCU6(void)
{
    boolean interruptState = IfxCpu_disableInterrupts();            /* Disable global interrupts                    */

    /* Timer configuration: timer used as counter */
    IfxCcu6_TimerWithTrigger_Config timerConf;
    IfxCcu6_TimerWithTrigger_initConfig(&timerConf, &MODULE_CCU61); /* Initialize the timer configuration with
                                                                     * default values                               */
    /* User timer configuration */
    timerConf.base.frequency = PWM_FREQUENCY;                       /* Set the desired frequency for the PWM signal */
    timerConf.base.countDir = IfxStdIf_Timer_CountDir_upAndDown;    /* Configure the timer to count up and down, in
                                                                     * order to generate center-aligned PWM signals */
    /* Initialize the timer driver */
    IfxCcu6_TimerWithTrigger_init(&g_timer, &timerConf);

    /* PWM High/Low driver configuration */
    IfxCcu6_PwmHl_Config pwmHlConf;
    IfxCcu6_PwmHl_initConfig(&pwmHlConf);                           /* Initialize the PwmHl configuration with
                                                                     * default values                               */
    /* User PWM High/Low driver configuration */
    pwmHlConf.timer = &g_timer;                                     /* Use the already configured timer             */
    pwmHlConf.base.channelCount = 1;               /* Configure the driver to use use all three
                                                                     * compare modules available in T12             */
    /* Assign output pins */
    //pwmHlConf.cc0 = &IfxCcu60_CC60_P02_0_OUT;IfxCcu61_CC62_P00_9_OUT
    //pwmHlConf.cc1 = &IfxCcu60_CC61_P02_2_OUT;
    pwmHlConf.cc2 = &IfxCcu61_CC62_P00_9_OUT;                   //Pino P00.9, IN2 do MCP33186 para controlar a TBI
    //pwmHlConf.cout0 = &IfxCcu60_COUT60_P02_1_OUT;
    //pwmHlConf.cout1 = &IfxCcu60_COUT61_P02_3_OUT;
 //   pwmHlConf.cout2 = &IfxDsadc_COUT1_P00_9_OUT;                 //Pino P00.9, IN2 do MCP33186 para controlar a TBI

    /* Initialize the PwmHl driver */
    IfxCcu6_PwmHl_init(&g_driver, &pwmHlConf);

    /* Instruct the driver to generate center aligned PWM signals */
    IfxCcu6_PwmHl_setMode(&g_driver, Ifx_Pwm_Mode_centerAligned);

    /* Set the duty cycles for the three channels */
    //Ifx_TimerValue cmpValues[NUMBER_OF_CHANNELS];
    //cmpValues[0] = CHANNEL1_COMPARE_VALUE;                          /* Set the compare value for channel 1          */
    //cmpValues[1] = CHANNEL2_COMPARE_VALUE;                          /* Set the compare value for channel 2          */
    //cmpValues[2] = CHANNEL3_COMPARE_VALUE;                          /* Set the compare value for channel 3          */
    Ifx_TimerValue cmpValues;
    g_driver.update(&g_driver, &cmpValues);                          /* Apply the compare values                     */

    /* Update the timer.
     * This instruction enables the shadow transfer of the compare values, copying the compare values to the
     * compare registers */
    IfxCcu6_TimerWithTrigger_applyUpdate(g_driver.timer);

    /* Restore interrupts to their initial state */
    IfxCpu_restoreInterrupts(interruptState);
}

/* Function that starts the timer and thus the generation of the PWM signals */
void startPWMGeneration(void)
{
    IfxCcu6_TimerWithTrigger_run(&g_timer);
}

void UpdatePWM (int Set_DutyCyle)
{
	 /* Set the duty cycles for the three channels */
	 //Ifx_TimerValue cmpValues[NUMBER_OF_CHANNELS];
	 //cmpValues[0] = ((PWM_PERIOD / 100) * (100 - Set_DutyCyle));     /* Set the compare value for channel 1          */
	 //cmpValues[1] = CHANNEL2_COMPARE_VALUE;                          /* Set the compare value for channel 2          */
	 //cmpValues[2] = CHANNEL3_COMPARE_VALUE;                          /* Set the compare value for channel 3          */
    
     Ifx_TimerValue cmpValues;
	 cmpValues = ((PWM_PERIOD / 1000) * (1000 - Set_DutyCyle));     /* Set the compare value for channel 1          */
	 g_driver.update(&g_driver, &cmpValues);                          /* Apply the compare values                     */
	 /* Update the timer.
	      * This instruction enables the shadow transfer of the compare values, copying the compare values to the
	      * compare registers */
	     IfxCcu6_TimerWithTrigger_applyUpdate(g_driver.timer);
}


