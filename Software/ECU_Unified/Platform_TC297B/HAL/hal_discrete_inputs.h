#ifndef _HAL_DISCRETE_INPUTS_H
#define _HAL_DISCRETE_INPUTS_H

#include "hal_discrete_common.h"

#define HAL_DISCRETE_Get_MC33186_SF()               IfxPort_getPinState(SF_MC33186_PIN)

#define HAL_DISCRETE_Get_IgnitionOn()               IfxPort_getPinState(IGNITION_ON_PIN)
#define HAL_DISCRETE_Get_PhaseState()               IfxPort_getPinState(PHASE_STATE_PIN)
#define HAL_DISCRETE_Get_BreakSwitch1()             IfxPort_getPinState(BRAKE1_SWITCH_PIN)
#define HAL_DISCRETE_Get_BreakSwitch2()             IfxPort_getPinState(BRAKE2_SWITCH_PIN)
#define HAL_DISCRETE_Get_ClutchSwitch()             IfxPort_getPinState(CLUTCH_SWITCH_PIN)
#define HAL_DISCRETE_Get_AC_Switch()                IfxPort_getPinState(AC_SWITCH_PIN)

void HAL_DISCRETE_Inputs_Init(void);

#endif /* _HAL_DISCRETE_INPUTS_H */