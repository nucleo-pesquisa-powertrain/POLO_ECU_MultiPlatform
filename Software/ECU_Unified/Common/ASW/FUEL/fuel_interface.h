#ifndef _FUEL_INTERFACE_H
#define _FUEL_INTERFACE_H

#include "ECU_State_interface.h"
#include <stdbool.h>

void FUEL_MainTask10ms(void);

/*---------------------------------------------------*/
/*--------------------- INPUTS ---------------------*/

unsigned short int Get_FUEL_mg_AirMass(void);
unsigned short int Get_FUEL_p_EthanolPct(void);
unsigned short int Get_FUEL_LambdaControl(void);
unsigned short int Get_FUEL_p_Pedal(void);
unsigned short int Get_FUEL_T_CoolTemp(void);
unsigned short int Get_FUEL_RPM_EngSpeed(void);
unsigned short int Get_FUEL_IgnitionOn(void);
unsigned short int Get_FUEL_ref_Torque(void);
ECU_State_t Get_FUEL_ECU_State(void);
bool Get_FUEL_Select_TINJ_CALC(void);
unsigned short Get_FUEL_KPa_MAP(void);

/*----------------------------------------------------*/
/*--------------------- OUTPUTS ---------------------*/
void Set_FUEL_us_InjectionTime(unsigned short int value);
unsigned short int Get_FUEL_us_InjectionTime(void);
void Set_FUEL_deg_InjAdvance(short int value);
short int Get_FUEL_deg_InjAdvance(void);

/*-----------------------------------------------------*/


#endif /* _FUEL_INTERFACE_H */
