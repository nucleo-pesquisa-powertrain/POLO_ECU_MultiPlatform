/*
 * ECU_StateMachine_interface.h
 *
 *  Created on: 1 de mai de 2025
 *      Author: u28m43
 */

#ifndef _ECU_STATE_INTERFACE_H
#define _ECU_STATE_INTERFACE_H

#include "ECU_State.h"

ECU_State_t Get_ECU_State(void);
void Set_ECU_State(ECU_State_t state);
void ECU_State_Task_10ms(void);

#endif /* _ECU_STATE_INTERFACE_H */
