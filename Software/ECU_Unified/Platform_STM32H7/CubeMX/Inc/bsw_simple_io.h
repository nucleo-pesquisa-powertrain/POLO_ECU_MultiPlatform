/*
 * bsw_simple_io.h
 *
 *  Created on: Jul 4, 2022
 *      Author: henri
 */

#ifndef INC_BSW_SIMPLE_IO_H_
#define INC_BSW_SIMPLE_IO_H_

#include "main.h"

#define ON  1
#define OFF 0


#define GetBit_L15()			HAL_GPIO_ReadPin(DI_LINHA15_GPIO_Port, DI_LINHA15_Pin)		// Função Get Flag de Linha 15

#define GetBit_Fase()			HAL_GPIO_ReadPin(DI_FASE_GPIO_Port, DI_FASE_Pin)			// Função Get Flag Sensor de Fase(Hall)

#define GetBit_H_SF()			HAL_GPIO_ReadPin(DI_H_SF_GPIO_Port, DI_H_SF_Pin)			// Função Get Status Flag Ponte H

#define GetBit_Brake1()			HAL_GPIO_ReadPin(DI_BRAKE1_GPIO_Port, DI_BRAKE1_Pin)		// Função Get Flag Freio1

#define GetBit_Break2()			HAL_GPIO_ReadPin(DI_BRAKE2_GPIO_Port, DI_BRAKE2_Pin)		// Função Get Flag Freio2

#define GetBit_Clutch()			HAL_GPIO_ReadPin(DI_CLUTCH_GPIO_Port, DI_CLUTCH_Pin)		// Função Get Flag Embreagem

#define GetBit_AC_Button()		HAL_GPIO_ReadPin(DI_AC_BUTTON_GPIO_Port, DI_AC_BUTTON_Pin)	// Função Get Botão do A/C

#define GetBit_DB_Button1()		HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)						// Botões de Debug e testes;

#define GetBit_DB_Button2()		HAL_GPIO_ReadPin(DI_BUTTON2_GPIO_Port, DI_BUTTON2_Pin)		// Botões de Debug e testes;

#define GetBit_DB_Button3()		HAL_GPIO_ReadPin(DI_BUTTON3_GPIO_Port, DI_BUTTON3_Pin)		// Botões de Debug e testes;


#define SetBit_H_DI1(state)			HAL_GPIO_WritePin(DO_H_DI1_GPIO_Port, DO_H_DI1_Pin, state)		    // Função Set DI1 da Ponte H

#define SetBit_H_DI2(state)			HAL_GPIO_WritePin(DO_H_DI2_GPIO_Port, DO_H_DI2_Pin, state)		    // Função Set DI2 da Ponte H

#define SetBit_H_COD(state)			HAL_GPIO_WritePin(DO_H_COD_GPIO_Port, DO_H_COD_Pin, state)		    // Função Set COD da Ponte H

#define SetBit_AC_Relay(state)		HAL_GPIO_WritePin(DO_AC_RELE_GPIO_Port, DO_AC_RELE_Pin, state)	    // Função Set Relé de A/C

#define SetBit_ColdRelay(state) 	HAL_GPIO_WritePin(DO_RELE_FRIO_GPIO_Port, DO_RELE_FRIO_Pin, state)	// Função Set Relé Principal de Partida a Frio

#define SetBit_ColdBomb(state)		HAL_GPIO_WritePin(DO_BOMB_FRIO_GPIO_Port, DO_BOMB_FRIO_Pin, state)	// Função Set Bomba de Partida a Frio

#define SetBit_InjBomb(state)		HAL_GPIO_WritePin(DO_BOMB_INJ_GPIO_Port, DO_BOMB_INJ_Pin, state)	// Função Set Bomba de Injeção

#define SetBit_BOB1(state)			HAL_GPIO_WritePin(DO_BOB_IGN1_GPIO_Port, DO_BOB_IGN1_Pin, state)	// Função Set Bobina1

#define SetBit_BOB2(state)			HAL_GPIO_WritePin(DO_BOB_IGN2_GPIO_Port, DO_BOB_IGN2_Pin, state)	// Função Set Bobina2

#define SetBit_LowFan(state)		HAL_GPIO_WritePin(DO_SPD1_VEL_GPIO_Port, DO_SPD1_VEL_Pin, state)	// Função Set Ventilador Baixa Potência de Resfriamento do Motor

#define SetBit_HighFan(state)		HAL_GPIO_WritePin(DO_SPD2_VEL_GPIO_Port, DO_SPD2_VEL_Pin, state)	// Função Set Ventilador Alta Potência de Resfriamento do Motor

#define SetBit_WarmLambda1(state)	HAL_GPIO_WritePin(DO_AQUEC_LAMBDA1_GPIO_Port, DO_AQUEC_LAMBDA1_Pin, state)	// Função Set Aquecimento Sensor de Oxigênio1

#define SetBit_WarmLambda2(state)	HAL_GPIO_WritePin(DO_AQUEC_LAMBDA2_GPIO_Port, DO_AQUEC_LAMBDA2_Pin, state)	// Função Set Aquecimento Sensor de Oxigênio2

#define SetBit_DB_Led1(state)		HAL_GPIO_WritePin(DO_LED1_GPIO_Port, DO_LED1_Pin, state)	// LED de Debug e testes;

#define SetBit_DB_Led2(state)		HAL_GPIO_WritePin(DO_LED2_GPIO_Port, DO_LED2_Pin, state)	// LED de Debug e testes;

#define SetBit_DB_Led3(state)		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, state)	        // LED de Debug e testes;

#define SetBit_DB_Led4(state)		HAL_GPIO_WritePin(DO_LED4_GPIO_Port, DO_LED4_Pin, state)	// LED de Debug e testes;


uint16_t Get16u_V_MAP(void);		// Função Get Valor de Tensão do Sensor MAP

uint16_t Get16u_V_Tar(void);    	// Função Get Valor de Tensão do Sensor Temperatura do Ar

uint16_t Get16u_V_Tagua(void);		// Função Get Valor de Tensão do Sensor Temperatura do Motor

uint16_t Get16u_V_Pedal1(void);		// Função Get Valor de Tensão do Sensor Pedal1

uint16_t Get16u_V_Pedal2(void);		// Função Get Valor de Tensão do Sensor Pedal2

uint16_t Get16u_V_TPS1(void);		// Função Get Valor de Tensão do Sensor TPS1(Posição Borboleta)

uint16_t Get16u_V_TPS2(void);		// Função Get Valor de Tensão do Sensor TPS2(Posição Borboleta)

uint16_t Get16u_V_Lambda1(void);	// Função Get Valor de Tensão do Sensor de Oxigênio1

uint16_t Get16u_V_Lambda2(void);	// Função Get Valor de Tensão do Sensor de Oxigênio2

uint16_t Get16u_V_Battery(void);	// Função Get Valor de Tensão da Carga da Bateria

uint16_t Get16u_V_Alternator(void);	// Função Get Valor de Tensão do Alternador

uint16_t Get16u_V_KnockSign(void);	// Função Get Valor de Tensão do Sensor de Detonação

uint16_t Get16u_V_AC_Press(void);	// Função Get Valor de Tensão do Sensor de Pressão do A/C


#endif /* INC_BSW_SIMPLE_IO_H_ */
