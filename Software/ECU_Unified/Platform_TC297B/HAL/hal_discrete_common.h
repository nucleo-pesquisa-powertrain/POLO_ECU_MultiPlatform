#ifndef _HAL_DISCRETE_COMMON_H
#define _HAL_DISCRETE_COMMON_H

#include "IfxPort.h"

#define LED_HIGH                &MODULE_P13, 0          /* LED D107                     */
#define LED_MID                 &MODULE_P13, 1          /* LED D108                     */
#define LED_LOW                 &MODULE_P13, 2          /* LED D109                     */
#define LED_GND                 &MODULE_P13, 3          /* LED D109                     */

#define LED_1                   &MODULE_P33, 4
#define LED_2                   &MODULE_P33, 2
#define LED_3                   &MODULE_P33, 12 
#define LED_4                   &MODULE_P33, 6

#define BUTTON_1                &MODULE_P23, 3 
#define BUTTON_2                &MODULE_P33, 11

/* INPUTS */
#define IGNITION_ON_PIN         &MODULE_P23,4
#define PHASE_STATE_PIN         &MODULE_P14,8
#define SF_MC33186_PIN          &MODULE_P00,7
#define BRAKE1_SWITCH_PIN       &MODULE_P14,4
#define BRAKE2_SWITCH_PIN       &MODULE_P15,4
#define CLUTCH_SWITCH_PIN       &MODULE_P15,6
#define AC_SWITCH_PIN           &MODULE_P14,7

/* OUTPUTS */
#define DI1_MC33186_PIN         &MODULE_P02,6
#define DI2_MC33186_PIN         &MODULE_P00,7
#define COD_MC33186_PIN         &MODULE_P02,4
#define IN1_MC33186_PIN         &MODULE_P00,11

#define CS_SPI_MC33810_PIN      &MODULE_P22,3

#define INJECTOR1_PIN           &MODULE_P02,1
#define INJECTOR2_PIN           &MODULE_P02,3      
#define INJECTOR3_PIN           &MODULE_P10,6      
#define INJECTOR4_PIN           &MODULE_P10,4 

#define GIN3_PIN                &MODULE_P00,6
#define GIN2_PIN                &MODULE_P00,4
#define GIN1_PIN                &MODULE_P00,2
#define GIN0_PIN                &MODULE_P00,0

#define ENOUT_MC33810_PIN       &MODULE_P10,7

//#define COLDSTART_PUMP_PIN      &MODULE_P02,0 //Pino P02,0 Agora é o EXT de Rotaçao
#define COLDSTART_RELAY_PIN     &MODULE_P00,12
#define FUEL_PUMP_PIN           &MODULE_P22,2
#define COIL2_PIN               &MODULE_P00,4 // Alterado de &MODULE_P33,9 para &MODULE_P02,2 porque o antigo estava enviando 3V
#define COIL1_PIN               &MODULE_P33,10
#define FAN_LOW_PIN             &MODULE_P00,8
#define FAN_HIGH_PIN            &MODULE_P00,10
#define LAMBDA_HEATER1_PIN      &MODULE_P00,1
#define LAMBDA_HEATER2_PIN      &MODULE_P02,8



#endif /* _HAL_DISCRETE_COMMON_H */
