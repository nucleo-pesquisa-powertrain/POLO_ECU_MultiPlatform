/**
 * \file rte_components.h
 * \brief RTE - Sinais de componentes do motor (TPS, ignicao, injecao)
 *
 * Interface publica para leitura e escrita dos sinais de atuadores e
 * componentes eletronicos do motor: posicao da borboleta (TPS/TBI),
 * avanco de ignicao (SPARK) e parametros de injecao de combustivel.
 *
 * Todos os sinais sao atualizados via Update_RTE_ComponentsSignals().
 *
 * Dependencias:
 *   - Platform_Types.h : tipos AUTOSAR portaveis
 *
 * Historico:
 *   2026-03-28  Adicionado include de Platform_Types.h para padronizacao
 *               com a nova arquitetura MCAL. Sem dependencias iLLD neste
 *               cabecalho; logica interna do .c nao possui chamadas HAL ativas.
 */

#ifndef _RTE_COMPONENTS_H
#define _RTE_COMPONENTS_H

#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Atualizacao dos sinais (chamar periodicamente na task de controle)  */
/* ------------------------------------------------------------------ */

/**
 * \brief Atualiza todos os sinais de componentes num unico ciclo.
 *
 * Internamente chama Update_RTE_deg_TPSAnglePosition(),
 * Update_RTE_deg_SPARKTiming(), Update_RTE_deg_InjectAdvance() e
 * Update_RTE_t_InjectionTime().
 */
void Update_RTE_ComponentsSignals(void);

/* ------------------------------------------------------------------ */
/* TPS / TBI                                                           */
/* ------------------------------------------------------------------ */

/** Posicao angular da borboleta em centesimos de grau (0 = fechada, 10000 = 100.00 deg) */
unsigned short int Get16u_RTE_deg_TPSAnglePosition(void);

/** Retorna o setpoint de posicao da TBI (enviado pelo controlador) */
unsigned short int Get16u_RTE_deg_TBIPositionSetPoint(void);

/** Define o setpoint de posicao da TBI */
void Set16u_RTE_deg_TBIPositionSetPoint(unsigned short int value);

/* ------------------------------------------------------------------ */
/* SPARK - Ignicao                                                     */
/* ------------------------------------------------------------------ */

/** Atualiza o avanco de ignicao a partir do modulo SPARK */
void Update_RTE_deg_SPARKTiming(void);

/** Retorna o avanco de ignicao atual em centesimos de grau */
short int Get16s_RTE_deg_SPARKTiming(void);

/* ------------------------------------------------------------------ */
/* FUEL - Injecao de combustivel                                       */
/* ------------------------------------------------------------------ */

/** Atualiza o avanco de injecao a partir do modulo FUEL */
void Update_RTE_deg_InjectAdvance(void);

/** Retorna o avanco de injecao atual em centesimos de grau */
short int Get16s_deg_InjectAdvance(void);

/** Atualiza o tempo de injecao a partir do modulo FUEL */
void Update_RTE_t_InjectionTime(void);

/** Retorna o tempo de injecao atual em microsegundos */
short int Get16u_RTE_t_InjectionTime(void);

#endif /* _RTE_COMPONENTS_H */
