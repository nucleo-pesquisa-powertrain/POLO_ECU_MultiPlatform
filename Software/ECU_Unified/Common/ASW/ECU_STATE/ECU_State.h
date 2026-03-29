/*
 * mngmt_EcuStates.h
 *
 *  Created on: 1 de mai de 2025
 *      Author: u28m43
 */

#ifndef _ECU_STATE_H
#define _ECU_STATE_H

typedef enum {
    ECU_STATE_OFF = 0,
    ECU_STATE_PRE_START = 1,
    ECU_STATE_CRANKING = 2,
    ECU_STATE_IDLE = 3,
    ECU_STATE_ACCELERATION = 4,
    ECU_STATE_OVERRUN = 5,
    ECU_STATE_ENGINE_STOPPED = 6,
    ECU_STATE_FAULT = 7
} ECU_State_t;

#endif /* _ECU_STATE_H */
