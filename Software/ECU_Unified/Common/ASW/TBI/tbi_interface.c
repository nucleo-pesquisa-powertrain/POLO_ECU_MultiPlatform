/*
 * tbi_interface.c
 *
 *  Created on: Aug 16, 2022
 *      Author: henri
 */

/* Este modulo relacionado */
#include "tbi_interface.h"
#include "TBI_PositionControl.h"

/* Inputs relacionados */
#include "rte_components.h"
#include "rte_environment.h"
#include "cdd_tbi.h"
#include "tbi_calibration.h"

/*--------------------- INPUTS ---------------------*/
static uint16_t TBI_deg_SetPoint;   /* Graus multiplicado por 10 */
static uint16_t TBI_deg_Feedback;   /* Graus multiplicado por 10 */
static uint8_t  TBI_IgnitionOn;
static uint8_t TBI_type;
static int16_t TBI_PWMOut;

/* Debug */
uint8_t teste_ign = 0;
uint16_t Debug_Setpoint = 87;
short int Debug_PWMOut = 0;
uint16_t Debug_Type;
uint16_t Debug_TBIPos;
uint8_t OpenLoopMode = 0;
uint8_t CalibrateMode = 0;

void TBI_MainTask10ms(void)
{
    TBI_deg_Feedback = Get16u_RTE_deg_TPSAnglePosition()/10;
    TBI_IgnitionOn = Get8u_RTE_b_IgnitionOn();
    TBI_deg_SetPoint = Debug_Setpoint;
    Debug_TBIPos = TBI_deg_Feedback;

    TBI_PositionControl_step();
    Debug_Type = TBI_type;

    if(OpenLoopMode)
    {
        CDD_TBI_SetPWM(Debug_PWMOut);
    }
    else if(CalibrateMode)
    {
        TBI_Calibration_Step();
    }
    else
    {
        CDD_TBI_SetPWM(TBI_PWMOut);
    }

}

/*---------------------------------------------------*/
/*--------------------- INPUTS ---------------------*/
void Set_TBI_deg_SetPoint(uint16_t setpoint)
{
    TBI_deg_SetPoint = setpoint;
}
uint16_t Get_TBI_deg_SetPoint(void)
{
    return TBI_deg_SetPoint;
}
uint16_t Get_TBI_deg_Feedback(void)
{
    return TBI_deg_Feedback;
}
uint8_t Get_IgnitionOn(void)
{
    return TBI_IgnitionOn | teste_ign;
}

/*----------------------------------------------------*/
/*--------------------- OUTPUTS ---------------------*/
void Set_TBI_PWM_Out(int16_t pwmout)
{
    TBI_PWMOut = pwmout;
}
int16_t Get_TBI_PWM_Out(void)
{
    return TBI_type;
}

void Set_TBI_type(uint8_t type)
{
    TBI_type = type;
}
uint8_t Get_TBI_type(void)
{
    return TBI_type;
}
/*-----------------------------------------------------*/
