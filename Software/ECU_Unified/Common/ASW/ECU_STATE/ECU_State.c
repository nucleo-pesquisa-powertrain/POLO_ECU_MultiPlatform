/*
 * mngmt_EcuStates.c
 *
 *  Created on: 1 de mai de 2025
 *      Author: u28m43
 */

#include "ECU_State.h"
#include "ECU_State_interface.h"
#include "rte_operator.h"
#include "rte_environment.h"

void ECU_State_Task_10ms(void)
{
	unsigned short int rpm = Get16u_RTE_rpm_EngineSpeeed();
	unsigned char pedal = 0; //Update_RTE_p_ThrottlePedal();
    unsigned char ignition = Get8u_RTE_b_IgnitionOn();

    ECU_State_t next_state = Get_ECU_State();

    switch (next_state)
    {
        case ECU_STATE_OFF:
            if(ignition == 1)
            {
            	next_state = ECU_STATE_PRE_START;
            }
        break;

        case ECU_STATE_PRE_START:
        	if(rpm >= 50)
        	{
        		next_state = ECU_STATE_CRANKING;
        	}
        break;

        case ECU_STATE_ENGINE_STOPPED:
            // Aguarda nova tentativa de partida
            if(rpm >= 50)
            {
                next_state = ECU_STATE_CRANKING;
            }
        break;

        case ECU_STATE_CRANKING:
            if(rpm >= 700)
            {
            	next_state = ECU_STATE_IDLE;
            }
            else if(rpm == 0)
            {
            	next_state = ECU_STATE_ENGINE_STOPPED;
            }
        break;

        case ECU_STATE_IDLE:
            if(pedal > 5)
            {
            	next_state = ECU_STATE_ACCELERATION;
            }
            else if(rpm == 0)
            {
            	next_state = ECU_STATE_ENGINE_STOPPED;
            }
        break;

        case ECU_STATE_ACCELERATION:
            if(pedal < 5)
            {
            	next_state = ECU_STATE_OVERRUN;
            }
            else if(rpm == 0)
            {
            	next_state = ECU_STATE_ENGINE_STOPPED;
            }
        break;

        case ECU_STATE_OVERRUN:
            if(pedal > 5)
            {
            	next_state = ECU_STATE_ACCELERATION;
            }
            else if(rpm <= 1000)
            {
            	next_state = ECU_STATE_IDLE;
            }
            else if(rpm == 0)
            {
            	next_state = ECU_STATE_ENGINE_STOPPED;
            }
        break;

        case ECU_STATE_FAULT:

        break;
    }

	if(ignition == 0)
	{
		next_state = ECU_STATE_OFF;
	}

	Set_ECU_State(next_state);


}
