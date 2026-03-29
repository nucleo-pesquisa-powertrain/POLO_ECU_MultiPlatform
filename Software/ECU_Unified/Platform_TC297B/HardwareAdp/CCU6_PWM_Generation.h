/*
 * CCU6_PWM_Generation.h
 *
 *  Created on: 18 de nov de 2020
 *      Author: davic
 */

#ifndef CCU6_PWM_GENERATION_H_
#define CCU6_PWM_GENERATION_H_

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
void initCCU6(void);
void startPWMGeneration(void);
void UpdatePWM (int Set_DutyCyle);

#endif /* CCU6_PWM_GENERATION_H_ */
