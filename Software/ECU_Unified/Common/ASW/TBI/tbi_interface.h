#ifndef _TBI_INTERFACE_H
#define _TBI_INTERFACE_H

#include <stdint.h>

/* Inputs */
void Set_TBI_deg_SetPoint(uint16_t setpoint);
uint16_t Get_TBI_deg_SetPoint(void);
uint16_t Get_TBI_deg_Feedback(void);
uint8_t Get_IgnitionOn(void);


/* Outputs */
void Set_TBI_PWM_Out(int16_t pwmout);
int16_t Get_TBI_PWM_Out(void);

void Set_TBI_type(uint8_t type);
uint8_t Get_TBI_type(void);


void TBI_MainTask10ms(void);



extern uint16_t Debug_Setpoint;
extern short int Debug_PWMOut;
extern uint16_t Debug_Type;
extern uint16_t Debug_TBIPos;

#endif /* INC_TBI_INTERFACE_H_ */
