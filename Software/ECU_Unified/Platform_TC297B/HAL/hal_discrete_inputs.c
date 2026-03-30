#include "hal_discrete_inputs.h"

void HAL_DISCRETE_Inputs_Init(void)
{
    IfxPort_setPinModeInput(IGNITION_ON_PIN  , IfxPort_InputMode_noPullDevice);
    IfxPort_setPinModeInput(PHASE_STATE_PIN  , IfxPort_InputMode_noPullDevice);
    IfxPort_setPinModeInput(SF_MC33186_PIN   , IfxPort_InputMode_noPullDevice);
    IfxPort_setPinModeInput(BRAKE1_SWITCH_PIN, IfxPort_InputMode_noPullDevice);
    IfxPort_setPinModeInput(BRAKE2_SWITCH_PIN, IfxPort_InputMode_noPullDevice);
    IfxPort_setPinModeInput(CLUTCH_SWITCH_PIN, IfxPort_InputMode_noPullDevice);
    IfxPort_setPinModeInput(AC_SWITCH_PIN    , IfxPort_InputMode_noPullDevice);
}
