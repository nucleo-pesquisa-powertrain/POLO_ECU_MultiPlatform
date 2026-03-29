/**
 * \file cdd_tbi.h
 * \brief CDD - Controle da Borboleta Eletronica (TBI - Throttle Body Injector)
 *
 * Interface publica do modulo de controle da borboleta eletronica.
 * O driver MC33186 e' controlado via saidas DIO para selecao de direcao
 * e via canal PWM para controle de posicao.
 *
 * Utiliza a MCAL AUTOSAR (Dio.h, Pwm.h) - sem dependencias iLLD/CCU6/GTM.
 *
 * Historico:
 *   2026-03-28  Copiado de CDD/ para ECU_Unified/Common/CDD/.
 *               Adicionado cabecalho de documentacao. Interface sem alteracoes.
 */

#ifndef _CDD_TBI_H
#define _CDD_TBI_H

/**
 * \brief Inicializa o modulo de controle da borboleta.
 *
 * Configura o driver MC33186: habilita o chip (COD=LOW), define direcao
 * inicial (DI2=HIGH, DI1=LOW) e inicia o PWM com duty cycle de 50%.
 */
void CDD_TBI_Init(void);

/**
 * \brief Define a posicao da borboleta via controle PWM.
 *
 * \param pwmvalue Valor de controle na faixa [-999 .. +999].
 *   - Positivo: abre a borboleta (DI1=HIGH, duty = (1000-pwmvalue)/10 %)
 *   - Negativo: fecha a borboleta (DI1=LOW,  duty = |pwmvalue|/10 %)
 *   - Zero:     sem movimento     (DI1=LOW,  duty = 1 %)
 *
 * Valores fora da faixa sao saturados em +/-999 internamente.
 */
void CDD_TBI_SetPWM(int pwmvalue);

#endif /* _CDD_TBI_H */
