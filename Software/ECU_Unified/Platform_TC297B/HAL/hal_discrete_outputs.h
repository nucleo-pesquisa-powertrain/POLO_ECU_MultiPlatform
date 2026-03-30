#ifndef _HAL_DISCRETE_OUTPUTS_H
#define _HAL_DISCRETE_OUTPUTS_H

#include "hal_discrete_common.h"

#define HAL_DISCRETE_Set_MC33186_DI1(state)                  IfxPort_setPinState(DI1_MC33186_PIN, state)
#define HAL_DISCRETE_Set_MC33186_DI2(state)                  IfxPort_setPinState(DI2_MC33186_PIN, state)
#define HAL_DISCRETE_Set_MC33186_COD(state)                  IfxPort_setPinState(COD_MC33186_PIN, state)
#define HAL_DISCRETE_Set_MC33186_IN1(state)                  IfxPort_setPinState(IN1_MC33186_PIN, state)

#define HAL_DISCRETE_Set_MC33810_SPI_CS(state)               IfxPort_setPinState(CS_SPI_MC33810_PIN, state)

#define HAL_DISCRETE_Set_MC33810_EnableOut(state)            IfxPort_setPinState(ENOUT_MC33810_PIN, state)

#define HAL_DISCRETE_Set_Injector1(state)                    IfxPort_setPinState(INJECTOR1_PIN, state)
#define HAL_DISCRETE_Set_Injector2(state)                    IfxPort_setPinState(INJECTOR2_PIN, state)
#define HAL_DISCRETE_Set_Injector3(state)                    IfxPort_setPinState(INJECTOR3_PIN, state)
#define HAL_DISCRETE_Set_Injector4(state)                    IfxPort_setPinState(INJECTOR4_PIN, state)

#define HAL_DISCRETE_Set_GIN3(state)                         IfxPort_setPinState(GIN3_PIN, state)
#define HAL_DISCRETE_Set_GIN2(state)                         IfxPort_setPinState(GIN2_PIN, state)
#define HAL_DISCRETE_Set_GIN1(state)                         IfxPort_setPinState(GIN1_PIN, state)
#define HAL_DISCRETE_Set_GIN0(state)                         IfxPort_setPinState(GIN0_PIN, state)


//#define HAL_DISCRETE_Set_ColdStartPump(state)                IfxPort_setPinState(COLDSTART_PUMP_PIN, state)
#define HAL_DISCRETE_Set_ColdStartRelay(state)               IfxPort_setPinState(COLDSTART_RELAY_PIN, state)
#define HAL_DISCRETE_Set_FuelPump(state)                     IfxPort_setPinState(FUEL_PUMP_PIN, state)
#define HAL_DISCRETE_Set_Coil1(state)                        IfxPort_setPinState(COIL1_PIN, state)
#define HAL_DISCRETE_Set_Coil2(state)                        IfxPort_setPinState(COIL2_PIN, state)
#define HAL_DISCRETE_Set_FanLow(state)                       IfxPort_setPinState(FAN_LOW_PIN, state)
#define HAL_DISCRETE_Set_FanHigh(state)                      IfxPort_setPinState(FAN_HIGH_PIN, state)
#define HAL_DISCRETE_Set_LambdaHeater1(state)                IfxPort_setPinState(LAMBDA_HEATER1_PIN, state)
#define HAL_DISCRETE_Set_LambdaHeater2(state)                IfxPort_setPinState(LAMBDA_HEATER2_PIN, state)


void HAL_DISCRETE_Outputs_Init(void);


#endif /* _HAL_DISCRETE_OUTPUTS_H */