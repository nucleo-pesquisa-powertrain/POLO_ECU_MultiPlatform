/**
 * \file rte_operator.c
 * \brief RTE - Leitura e processamento do sinal do pedal do acelerador
 *
 * Responsavel por:
 *   - Ler o canal ADC_CH_PEDAL via MCAL Adc_ReadChannel_mV()
 *   - Converter a tensao em mV para um valor normalizado ADC (0..255)
 *   - Linearizar o sinal entre os pontos de calibracao do pedal
 *   - Expor o resultado em percentual (0..100%) via Get16u_RTE_p_ThrottlePedal()
 *
 * Dependencias:
 *   - EcuAbs_Sensors.h : camada de abstracao de sensores (EcuAbs_GetPedalPos_pct())
 *
 * Historico:
 *   2026-03-28  Refatorado: substituida chamada iLLD/HAL direta
 *               HAL_ADC_Get_V_ThrottlePedal1() por
 *               Adc_ReadChannel_mV(ADC_CH_PEDAL). O retorno e' em mV
 *               em ambas as APIs; a formula de conversao foi preservada
 *               identica.
 *   2026-03-30  Refatorado: Update_* delega para EcuAbs_Sensors.
 *               Algoritmo de conversao movido para EcuAbs.
 */

#include "rte_operator.h"
#include "EcuAbs_Sensors.h"
#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Sinal do RTE (variavel de estado interna)                           */
/* ------------------------------------------------------------------ */

/** Posicao do pedal do acelerador em percentual (0 = solto, 100 = fundo) */
unsigned short int S_RTE_p_ThrottlePedal;

/* ------------------------------------------------------------------ */
/* Atualizacao do sinal                                                 */
/* ------------------------------------------------------------------ */

/**
 * \brief Obtem a posicao do pedal do acelerador via EcuAbs [%].
 *
 * O algoritmo de normalizacao e linearizacao e aplicado internamente
 * pelo EcuAbs.
 */
void Update_RTE_p_ThrottlePedal(void)
{
    S_RTE_p_ThrottlePedal = EcuAbs_GetPedalPos_pct();
}

/* ------------------------------------------------------------------ */
/* Leitura do sinal                                                     */
/* ------------------------------------------------------------------ */

/**
 * \brief Retorna a posicao do pedal do acelerador em percentual (0 .. 100).
 */
unsigned short int Get16u_RTE_p_ThrottlePedal(void)
{
    return S_RTE_p_ThrottlePedal;
}
