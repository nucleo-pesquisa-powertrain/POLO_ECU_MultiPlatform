/*
 * ECU_StateMachine_interface.c
 *
 *  Created on: 1 de mai de 2025
 *      Author: u28m43
 */

#include "ECU_State_interface.h"

static ECU_State_t ecu_current_state = ECU_STATE_OFF;

ECU_State_t Get_ECU_State(void)
{
	return ecu_current_state;
}

void Set_ECU_State(ECU_State_t state)
{
	ecu_current_state = state;
}
