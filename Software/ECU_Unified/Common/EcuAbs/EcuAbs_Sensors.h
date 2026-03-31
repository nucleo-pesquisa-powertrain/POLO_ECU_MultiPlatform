/**
 * \file EcuAbs_Sensors.h
 * \brief Camada de abstracao de sensores da ECU
 *
 * Fornece uma API unica e independente de plataforma para leitura dos
 * sensores do motor. Os valores sao atualizados ciclicamente por
 * EcuAbs_Update() e expostos via funcoes getter sem chamada MCAL.
 *
 * Dependencias:
 *   - Platform_Types.h (tipos AUTOSAR: uint8, uint16, sint16, etc.)
 *
 * Uso tipico:
 *   EcuAbs_Init();           // na inicializacao
 *   EcuAbs_Update();         // a cada 10 ms, antes do RTE
 *   temp = EcuAbs_GetAirTemp_degC();
 */
#ifndef ECUABS_SENSORS_H
#define ECUABS_SENSORS_H

#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Temperaturas                                                        */
/* ------------------------------------------------------------------ */

/** Temperatura do ar de admissao [degC] (curva quadratica, sat. -10..129) */
sint16  EcuAbs_GetAirTemp_degC(void);

/** Temperatura do liquido de arrefecimento [degC] (curva linear, sat. -5..143) */
sint16  EcuAbs_GetCoolantTemp_degC(void);

/* ------------------------------------------------------------------ */
/* Pressao                                                             */
/* ------------------------------------------------------------------ */

/** Pressao do coletor de admissao - MAP [kPa] (curva linear, sat. 8..121) */
uint16  EcuAbs_GetMAP_kPa(void);

/* ------------------------------------------------------------------ */
/* Tensao                                                              */
/* ------------------------------------------------------------------ */

/** Tensao da bateria [mV] (com fator do divisor resistivo 14/5) */
uint16  EcuAbs_GetVbatt_mV(void);

/* ------------------------------------------------------------------ */
/* Pedal e borboleta                                                   */
/* ------------------------------------------------------------------ */

/** Posicao do pedal do acelerador [%] (0..100, linearizado) */
uint16  EcuAbs_GetPedalPos_pct(void);

/** Posicao da borboleta - TPS [raw mV filtrado] */
uint16  EcuAbs_GetTPS_raw(void);

/* ------------------------------------------------------------------ */
/* Motor                                                               */
/* ------------------------------------------------------------------ */

/** Rotacao do motor [RPM] */
uint16  EcuAbs_GetEngineSpeed_rpm(void);

/** Percentual de etanol no combustivel (escala 0..1000) */
uint16  EcuAbs_GetEthanolPercent(void);

/* ------------------------------------------------------------------ */
/* Entradas discretas                                                  */
/* ------------------------------------------------------------------ */

/** Estado do sensor de fase (arvore de cames): 1 = ativo, 0 = inativo */
uint8   EcuAbs_GetPhaseState(void);

/** Estado da chave de ignicao: 1 = ligada, 0 = desligada */
uint8   EcuAbs_GetIgnitionOn(void);

/* ------------------------------------------------------------------ */
/* Inicializacao e atualizacao ciclica                                 */
/* ------------------------------------------------------------------ */

/** Inicializa o modulo EcuAbs (sensores ja inicializados pelo MCAL) */
void    EcuAbs_Init(void);

/** Atualiza todos os sensores - chamar a cada 10 ms antes do RTE */
void    EcuAbs_Update(void);

#endif /* ECUABS_SENSORS_H */
