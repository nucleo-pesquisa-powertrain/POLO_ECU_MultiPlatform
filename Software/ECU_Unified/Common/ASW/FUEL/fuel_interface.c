#include "fuel_interface.h"
#include "FUEL_FuelControl.h"
#include "rte_environment.h"
#include "rte_operator.h"
#include <stdbool.h>

/*--------------------- INPUTS ---------------------*/
static unsigned short S_FUEL_mg_AirMass;
static unsigned short S_FUEL_p_EthanolPct;
static unsigned short S_FUEL_LambdaControl;
static unsigned short S_FUEL_p_Pedal;
static unsigned short S_FUEL_T_CoolTemp;
static unsigned short S_FUEL_RPM_EngSpeed;
static unsigned char  S_FUEL_IgnitionOn;
static ECU_State_t FUEL_ECUStateMachine;
volatile bool B_FUEL_Select_TINJ_CALC = 1;
static unsigned short S_FUEL_KPa_MAP;
/*--------------------- OUTPUTS ---------------------*/
short int S_FUEL_deg_InjAdvance = 0;
unsigned short int S_FUEL_us_InjectionTime = 0;

void FUEL_MainTask10ms(void)
{
    unsigned short int airPress = Get16u_RTE_P_ManAirPress();
    short int airTemp = Get16s_RTE_K_AirTemperature();

    if (airTemp == 0)
    {
        airTemp = 1; //Para nao dividir por zero
    }

    /* INPUTS UPDATE */
    S_FUEL_mg_AirMass = (float)((1397 * airPress) / airTemp);
    S_FUEL_p_EthanolPct = Get16u_RTE_p_EthanolPercent() / 10;
    S_FUEL_LambdaControl = 0;
    S_FUEL_p_Pedal = 0;//Get16u_RTE_p_ThrottlePedal()/10;
    S_FUEL_T_CoolTemp = Get16s_RTE_T_CoolantTemperature();
    S_FUEL_RPM_EngSpeed = Get16u_RTE_rpm_EngineSpeeed();
    S_FUEL_IgnitionOn = Get8u_RTE_b_IgnitionOn();
    FUEL_ECUStateMachine = Get_RTE_ECU_State();
    S_FUEL_KPa_MAP = Get16u_RTE_P_ManAirPress();

    /* STEP FUNCTION */
    FUEL_FuelControl_step();
}

/*---------------------------------------------------*/
/*--------------------- INPUTS ---------------------*/

unsigned short int Get_FUEL_mg_AirMass(void)
{
    return S_FUEL_mg_AirMass;
}

unsigned short int Get_FUEL_p_EthanolPct(void)
{
    return S_FUEL_p_EthanolPct;
}

unsigned short int Get_FUEL_LambdaControl(void)
{
    return S_FUEL_LambdaControl;
}

unsigned short int Get_FUEL_p_Pedal(void)
{
    return S_FUEL_p_Pedal;
}

unsigned short int Get_FUEL_T_CoolTemp(void)
{
    return S_FUEL_T_CoolTemp;
}

unsigned short int Get_FUEL_RPM_EngSpeed(void)
{
    return S_FUEL_RPM_EngSpeed;
}

unsigned short int Get_FUEL_IgnitionOn(void)
{
    return S_FUEL_IgnitionOn;
}

unsigned short int Get_FUEL_ref_Torque(void)
{
    return 0;
}

ECU_State_t Get_FUEL_ECU_State(void)
{
    return FUEL_ECUStateMachine;
}

bool Get_FUEL_Select_TINJ_CALC(void)
{
    return B_FUEL_Select_TINJ_CALC;
}

unsigned short Get_FUEL_KPa_MAP(void)
{
    return S_FUEL_KPa_MAP;
}

/*----------------------------------------------------*/
/*--------------------- OUTPUTS ---------------------*/

volatile float FUEL_Corr = 1.0;

void Set_FUEL_us_InjectionTime(unsigned short int value)
{
    //S_FUEL_us_InjectionTime = value;
    S_FUEL_us_InjectionTime = (float)value*FUEL_Corr;
}

unsigned short int Get_FUEL_us_InjectionTime(void)
{
    return S_FUEL_us_InjectionTime;
}

void Set_FUEL_deg_InjAdvance(short int value)
{
    S_FUEL_deg_InjAdvance = value;
}

short int Get_FUEL_deg_InjAdvance(void)
{
    return S_FUEL_deg_InjAdvance;
}
/*-----------------------------------------------------*/
