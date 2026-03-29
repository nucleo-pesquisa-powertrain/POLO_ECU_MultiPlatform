/**
 * \file cdd_fuelpump.h
 * \brief CDD - Controle da Bomba de Combustivel
 *
 * Interface publica do modulo de controle da bomba de combustivel.
 * A bomba e' ativada nos estados de operacao do motor e durante a
 * fase inicial de pre-pressurizacao do sistema (2 segundos).
 *
 * Utiliza a MCAL AUTOSAR (Dio.h) - sem dependencias iLLD.
 *
 * Historico:
 *   2026-03-28  Copiado de CDD/ para ECU_Unified/Common/CDD/.
 *               Adicionado cabecalho de documentacao. Interface sem alteracoes.
 */

#ifndef _CDD_FUELPUMP_H
#define _CDD_FUELPUMP_H

/**
 * \brief Tarefa periodica de controle da bomba de combustivel.
 *
 * Deve ser chamada a cada 10 ms pelo escalonador de tarefas. Controla
 * a saida DIO_CH_FUEL_PUMP de acordo com o estado atual da ECU:
 *   - Ativa nos estados CRANKING, IDLE, ACCELERATION e OVERRUN
 *   - Ativa por 2 segundos nos estados PRE_START e ENGINE_STOPPED
 *     (fase de pre-pressurizacao inicial)
 *   - Inativa em todos os outros estados
 */
void CDD_FuelPump_Event(void);

#endif /* _CDD_FUELPUMP_H */
