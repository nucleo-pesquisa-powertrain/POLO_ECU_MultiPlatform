#ifndef _MNGMT_INTERFACE_H
#define _MNGMT_INTERFACE_H

void MNGT_MainTask20ms(void);

/*---------------------------------------------------*/
/*--------------------- INPUTS ---------------------*/

unsigned short int Get_MNGT_rpm_EngSpeed(void);
unsigned short int Get_MNGT_p_Pedal(void);

/*----------------------------------------------------*/
/*--------------------- OUTPUTS ---------------------*/

void Set_MNGT_s_ManagementState(unsigned short int value);
unsigned short int Get_MNGT_s_ManagementState(void);
/*-----------------------------------------------------*/


#endif /* _MNGMT_INTERFACE_H */