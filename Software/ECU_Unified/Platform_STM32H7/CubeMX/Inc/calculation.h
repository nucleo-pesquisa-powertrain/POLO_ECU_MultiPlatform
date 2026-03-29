/*
 * calculation.h
 *
 *  Created on: 28 de jul de 2022
 *      Author: henri
 */

#ifndef INC_CALCULATION_H_
#define INC_CALCULATION_H_


#define BOB_Timing      4000    // Tempo de Carregar Bobina de Ignição; Código PIC a do GOL que é utilizada
#define MIN_Time_INJ         // Tempo Mínimo para Injeção; Código PIC a do GOL que é utilizada
#define MAX_Time_INJ         // Tempo Máximo para Injeção; Código PIC a do GOL que é utilizada
#define Const_Gases     1397    // Valor calculado para simplificar a conta. Ler documento para compreensão.
#define Vazao_Inj       300.3   // Vazão de combustível dos bicos injetores; Código PIC a do GOL que é utilizada
#define MAX_Pedal       191     // Valor em bits; Código PIC a do GOL que é utilizada
#define MIN_Pedal       37      // Valor em Bits; Código PIC a do GOL que é utilizada
#define 


uint16_t Calc16u_Rot (uint16_t dent_time);

uint16_t Calc_Pressure (uint16_t V_MAP);

uint16_t Calc_AirMass (uint16_t pressure, uint8_t T_Air);

uint16_t Calc_FuelMass (uint16_t air_mass);

uint8_t Calc8u_P_Pedal (uint16_t V_Pedal);

uint8_t Calc8u_G_TPS (uint16_t V_TPS);

uint16_t Calc_T_Ign (uint16_t );

uint16_t Calc_T_Inj (uint16_t );


#endif /* INC_CALCULATION_H_ */
