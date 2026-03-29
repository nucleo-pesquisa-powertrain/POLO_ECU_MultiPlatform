/**
 * \file rte_operator.h
 * \brief RTE - Sinal do pedal do acelerador (entrada do operador)
 *
 * Interface publica para leitura da posicao do pedal do acelerador,
 * convertida para percentual (0 .. 100%).
 *
 * Dependencias:
 *   - Platform_Types.h : tipos AUTOSAR portaveis
 *
 * Historico:
 *   2026-03-28  Adicionado include de Platform_Types.h para padronizacao
 *               com a nova arquitetura MCAL.
 */

#ifndef _RTE_OPERATOR_H
#define _RTE_OPERATOR_H

#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Atualizacao e leitura do pedal do acelerador                        */
/* ------------------------------------------------------------------ */

/**
 * \brief Atualiza o percentual de posicao do pedal do acelerador.
 *
 * Le o canal ADC_CH_PEDAL via MCAL, converte para ADC normalizado
 * (0..255) e aplica a linearizacao entre os pontos de calibracao.
 */
void Update_RTE_p_ThrottlePedal(void);

/**
 * \brief Retorna a posicao do pedal do acelerador em percentual (0 .. 100).
 */
unsigned short int Get16u_RTE_p_ThrottlePedal(void);

#endif /* _RTE_OPERATOR_H */
