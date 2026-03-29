#include "tbi_setpoint_interface.h"
#include "SetPoint_TBI.h"

#include "rte_environment.h"
#include "rte_operator.h"
#include "rte_components.h"

/*--------------------- INPUTS ---------------------*/

static unsigned short S_SPTBI_p_Pedal;
static unsigned short S_SPTBI_T_Water;
static unsigned short S_SPTBI_RPM;
static unsigned char  S_SPTBI_IgnitionOn;

/*--------------------- OUTPUTS ---------------------*/
unsigned short int S_SPTBI_deg_Setpoint = 0;


void THROTTLE_MainTask10ms(void)
{
    /* INPUTS UPDATE */
    S_SPTBI_p_Pedal = Get16u_RTE_p_ThrottlePedal()/10;
    S_SPTBI_T_Water = Get16s_RTE_T_CoolantTemperature();
    S_SPTBI_RPM = Get16u_RTE_rpm_EngineSpeeed();
    S_SPTBI_IgnitionOn = Get8u_RTE_b_IgnitionOn();

    /* STEP FUNCTION */
    SetPoint_TBI_step();

    Set16u_RTE_deg_TBIPositionSetPoint(S_SPTBI_deg_Setpoint);

}

/*---------------------------------------------------*/
/*--------------------- INPUTS ---------------------*/


unsigned short int Get_SPTBI_p_Pedal(void)
{
    return S_SPTBI_p_Pedal;
}
unsigned short int Get_SPTBI_T_Water(void)
{
    return S_SPTBI_T_Water;
}
unsigned short int Get_SPTBI_RPM(void)
{
    return S_SPTBI_RPM;
}
unsigned short int Get_SPTBI_IgnitionOn(void)
{
    return S_SPTBI_IgnitionOn;
}

/*----------------------------------------------------*/
/*--------------------- OUTPUTS ---------------------*/

void Set_SPTBI_deg_Setpoint(unsigned short int value)
{

    S_SPTBI_deg_Setpoint = value;

}
unsigned short int Get_SPTBI_deg_Setpoint(void)
{
    return S_SPTBI_deg_Setpoint;
}

/*-----------------------------------------------------*/
