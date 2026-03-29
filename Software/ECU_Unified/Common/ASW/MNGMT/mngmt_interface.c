#include "mngmt_interface.h"
#include "Management.h"

#include "rte_environment.h"


/*--------------------- INPUTS ---------------------*/
static unsigned short S_MNGT_rpm_EngSpeed;
static unsigned short S_MNGT_p_Pedal;

/*--------------------- OUTPUTS ---------------------*/
static unsigned short int S_MNGT_s_ManagementState;


void MNGT_MainTask20ms(void)
{
    /* INPUTS UPDATE */
    S_MNGT_rpm_EngSpeed = Get16u_RTE_rpm_EngineSpeeed();
    S_MNGT_p_Pedal = 0;

    /* STEP FUNCTION */
    Management_step();


}

/*---------------------------------------------------*/
/*--------------------- INPUTS ---------------------*/

unsigned short int Get_MNGT_rpm_EngSpeed(void)
{
    return S_MNGT_rpm_EngSpeed;
}

unsigned short int Get_MNGT_p_Pedal(void)
{
    return S_MNGT_p_Pedal;
}

/*----------------------------------------------------*/
/*--------------------- OUTPUTS ---------------------*/

void Set_MNGT_s_ManagementState(unsigned short int value)
{
    S_MNGT_s_ManagementState = value;
}
unsigned short int Get_MNGT_s_ManagementState(void)
{
    return S_MNGT_s_ManagementState;
}
/*-----------------------------------------------------*/
