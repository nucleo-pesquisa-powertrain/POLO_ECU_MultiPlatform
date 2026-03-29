/**
 * \file cdd_injectors.h
 * \brief CDD - Controle de Injetores de Combustivel
 *
 * Modulo responsavel pelo acionamento dos injetores via saidas DIO
 * e pelo controle do driver MC33810 via SPI.
 *
 * Suporta dois modos de operacao:
 *   - Injecao sequencial: injetores acionados individualmente com
 *     fila de ate 4 eventos pendentes
 *   - Injecao em grupo: todos os 4 injetores acionados simultaneamente
 *
 * Utiliza a MCAL AUTOSAR (Gpt.h, Dio.h, Spi.h) - sem dependencias iLLD.
 *
 * \note Os tempos de injecao sao recebidos e armazenados em
 *       MICROSEGUNDOS. Nenhuma conversao de ticks e' necessaria
 *       no layer CDD; a MCAL faz isso internamente.
 */
#ifndef CDD_INJECTORS_H
#define CDD_INJECTORS_H

#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Prototipos publicos                                                 */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo de injecao
 *
 * Inicializa o SPI (MCAL), configura o MC33810 via SPI e garante
 * que todos os injetores estejam desligados.
 */
void CDD_INJ_Init(void);

/**
 * \brief Inicia injecao sequencial para um injetor especifico
 *
 * \param inj_num  Numero do injetor (1 a 4)
 * \param inj_time Tempo de injecao em MICROSEGUNDOS
 *
 * Aciona o injetor indicado e agenda o encerramento via timer
 * GPT_CH_INJ_DURATION. Se o timer ja estiver em uso (injetor anterior
 * ainda aberto), o novo evento e' inserido na fila S_InjTimeQueue_us[].
 * Fila com capacidade de ate 4 entradas simultaneas.
 */
void CDD_INJ_PerformSeqFuelInj(uint8 inj_num, uint16 inj_time);

/**
 * \brief Inicia injecao em grupo (todos os 4 injetores)
 *
 * \param inj_time Tempo de injecao em MICROSEGUNDOS
 *
 * Aciona todos os injetores ao mesmo tempo e inicia o timer para
 * encerrar a injecao apos inj_time microsegundos.
 */
void CDD_INJ_PerformFullGroupInjection(uint16 inj_time);

/**
 * \brief Callback: encerra a injecao do injetor no topo da fila
 *
 * Chamado pela ISR do timer GPT_CH_INJ_DURATION. Desliga o injetor
 * do slot [0] da fila e avanca a fila. Se houver proxima entrada
 * pendente, reinicia o timer com o tempo residual correspondente.
 */
void CDD_INJ_StopFuelInjEvent(void);

#endif /* CDD_INJECTORS_H */
