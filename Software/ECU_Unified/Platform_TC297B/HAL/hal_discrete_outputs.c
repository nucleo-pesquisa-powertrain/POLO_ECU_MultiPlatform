#include "hal_discrete_outputs.h"

void HAL_DISCRETE_Outputs_Init(void)
{
    IfxPort_setPinModeOutput(DI1_MC33186_PIN   , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(DI2_MC33186_PIN   , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(COD_MC33186_PIN   , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    //IfxPort_setPinModeOutput(COLDSTART_PUMP_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(COLDSTART_RELAY_PIN,IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(FUEL_PUMP_PIN     , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(COIL1_PIN         , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(COIL2_PIN         , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(FAN_LOW_PIN       , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(FAN_HIGH_PIN      , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(LAMBDA_HEATER1_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(LAMBDA_HEATER2_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(IN1_MC33186_PIN   , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    IfxPort_setPinModeOutput(INJECTOR1_PIN   , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(INJECTOR2_PIN   , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(INJECTOR3_PIN   , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(INJECTOR4_PIN   , IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(CS_SPI_MC33810_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(ENOUT_MC33810_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    
    IfxPort_setPinModeOutput(GIN0_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(GIN1_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(GIN2_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinModeOutput(GIN3_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    HAL_DISCRETE_Set_MC33810_EnableOut(IfxPort_State_high);
    HAL_DISCRETE_Set_GIN0(IfxPort_State_low);
    HAL_DISCRETE_Set_GIN1(IfxPort_State_low);
    HAL_DISCRETE_Set_GIN2(IfxPort_State_low);
    HAL_DISCRETE_Set_GIN3(IfxPort_State_low);

    HAL_DISCRETE_Set_MC33186_DI1(IfxPort_State_low);
    HAL_DISCRETE_Set_MC33186_DI2(IfxPort_State_low);  
    HAL_DISCRETE_Set_MC33186_COD(IfxPort_State_low);  
    HAL_DISCRETE_Set_MC33186_IN1(IfxPort_State_low);
    //HAL_DISCRETE_Set_ColdStartPump(IfxPort_State_low);
    HAL_DISCRETE_Set_ColdStartRelay(IfxPort_State_low);
    HAL_DISCRETE_Set_FuelPump(IfxPort_State_low);     
    HAL_DISCRETE_Set_Coil1(IfxPort_State_low);        
    HAL_DISCRETE_Set_Coil2(IfxPort_State_low);        
    HAL_DISCRETE_Set_FanLow(IfxPort_State_low);      
    HAL_DISCRETE_Set_FanHigh(IfxPort_State_low);      
    HAL_DISCRETE_Set_LambdaHeater1(IfxPort_State_low);
    HAL_DISCRETE_Set_LambdaHeater2(IfxPort_State_low);

    HAL_DISCRETE_Set_Injector1(IfxPort_State_low);
    HAL_DISCRETE_Set_Injector2(IfxPort_State_low);
    HAL_DISCRETE_Set_Injector3(IfxPort_State_low);
    HAL_DISCRETE_Set_Injector4(IfxPort_State_low);
    HAL_DISCRETE_Set_MC33810_SPI_CS(IfxPort_State_high);

}
