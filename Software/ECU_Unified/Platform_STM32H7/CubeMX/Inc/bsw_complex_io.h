/*
 * bsw_complex_io.h
 *
 *  Created on: 22 de jul de 2022
 *      Author: henri
 */

#include "main.h"

#ifndef INC_BSW_COMPLEX_IO_H_
#define INC_BSW_COMPLEX_IO_H_

#define ON 		 1
#define OFF		 0
#define AboveLH  1
#define UnderLH  2
#define LimpHome 0

uint16_t Get16u_T_DentTime();		// Função Get Tempo de 1 dente em microssengundos(us); Interrupção externa

void SetPWM_H_Throttle(uint8_t type, uint16_t value);	// Função Set PWM da Ponte H (Válvula Borboleta);

void SetPWM_Coal(uint8_t type, uint16_t value);	        // Função Set PWM do Carvão Ativo

#endif /* INC_BSW_COMPLEX_IO_H_ */
