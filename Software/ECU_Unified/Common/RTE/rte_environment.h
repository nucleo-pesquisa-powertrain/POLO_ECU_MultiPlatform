/**
 * \file rte_environment.h
 * \brief RTE - Sinais de ambiente do motor (temperaturas, pressao, rpm, bateria)
 *
 * Interface publica para leitura dos sinais de ambiente adquiridos pelo modulo
 * RTE. Todos os valores sao atualizados via Update_RTE_EnvironmentSignals().
 *
 * Dependencias:
 *   - Platform_Types.h  : tipos AUTOSAR portaveis
 *   - ECU_State_interface.h : tipo ECU_State_t
 *
 * Historico:
 *   2026-03-28  Refatorado para usar MCAL API (Adc.h / Dio.h) em lugar de
 *               chamadas iLLD/HAL diretas (hal_adc_inputs.h, hal_discrete_inputs.h).
 */

#ifndef _RTE_ENVIRONMENT_H
#define _RTE_ENVIRONMENT_H

#include "Platform_Types.h"
#include "ECU_State_interface.h"

/* ------------------------------------------------------------------ */
/* Atualizacao dos sinais (chamar periodicamente na task de ambiente)  */
/* ------------------------------------------------------------------ */

/**
 * \brief Atualiza todos os sinais de ambiente do motor num unico ciclo.
 *
 * Deve ser chamada periodicamente pela task de controle (ex.: 10 ms).
 * Internamente invoca cada funcao Update_RTE_* individual.
 */
void Update_RTE_EnvironmentSignals(void);

/* ------------------------------------------------------------------ */
/* Leituras dos sinais de ambiente                                     */
/* ------------------------------------------------------------------ */

/** Temperatura do ar de admissao em Kelvin (K) */
short int Get16s_RTE_K_AirTemperature(void);

/** Temperatura do ar de admissao em graus Celsius (degC) */
short int Get16s_RTE_T_AirTemperature(void);

/** Temperatura do liquido de arrefecimento em graus Celsius (degC) */
short int Get16s_RTE_T_CoolantTemperature(void);

/** Pressao do coletor de admissao em kPa */
unsigned short int Get16u_RTE_P_ManAirPress(void);

/** Percentual de etanol no combustivel (escala 0-1000, onde 1000 = 100%) */
unsigned short int Get16u_RTE_p_EthanolPercent(void);

/** Rotacao do motor em RPM (nota: typo 'Speeed' preservado para compatibilidade de interface) */
unsigned short int Get16u_RTE_rpm_EngineSpeeed(void);

/** Tensao da bateria em mV */
unsigned short int Get16u_RTE_V_BatteryCharge(void);

/** Estado da chave de ignicao: 1 = ligada, 0 = desligada */
unsigned char Get8u_RTE_b_IgnitionOn(void);

/** Estado do sensor de fase (arvore de cames): 1 = ativo, 0 = inativo */
unsigned char Get8u_RTE_b_CamshaftState(void);

/** Estado geral da ECU */
ECU_State_t Get_RTE_ECU_State(void);

#endif /* _RTE_ENVIRONMENT_H */
