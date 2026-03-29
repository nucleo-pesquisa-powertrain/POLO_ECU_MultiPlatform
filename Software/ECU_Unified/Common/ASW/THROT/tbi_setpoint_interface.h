#ifndef _THROTTLE_INTERFACE_H
#define _THROTTLE_INTERFACE_H


void THROTTLE_MainTask10ms(void);
unsigned short int Get_SPTBI_p_Pedal(void);
unsigned short int Get_SPTBI_T_Water(void);
unsigned short int Get_SPTBI_RPM(void);
unsigned short int Get_SPTBI_IgnitionOn(void);
void Set_SPTBI_deg_Setpoint(unsigned short int value);
unsigned short int Get_SPTBI_deg_Setpoint(void);

#endif //_THROTTLE_INTERFACE_H
