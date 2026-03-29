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
 * Algoritmo de conversao (preservado identico ao original):
 *   1. Normaliza mV -> ADC 8-bit: adc8 = mV * 255 / 5000
 *   2. Lineariza entre calibracao minima (39) e maxima (191):
 *      percent = (adc8 - 39) * 100 / (191 - 39)
 *
 * Dependencias MCAL (plataforma-independentes):
 *   - Adc.h : Adc_ReadChannel_mV(), canal ADC_CH_PEDAL
 *
 * Historico:
 *   2026-03-28  Refatorado: substituida chamada iLLD/HAL direta
 *               HAL_ADC_Get_V_ThrottlePedal1() por
 *               Adc_ReadChannel_mV(ADC_CH_PEDAL). O retorno e' em mV
 *               em ambas as APIs; a formula de conversao foi preservada
 *               identica.
 */

#include "rte_operator.h"
#include "Adc.h"
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
 * \brief Le o canal ADC_CH_PEDAL e converte para percentual de posicao.
 *
 * Etapa 1 - Normalizacao para 8-bit ADC:
 *   adc8 = Adc_ReadChannel_mV(ADC_CH_PEDAL) * 255 / 5000
 *   (equivalente ao range 0-255 que o algoritmo original esperava)
 *
 * Etapa 2 - Linearizacao com pontos de calibracao do pedal:
 *   percent = (adc8 - 39) * 100 / (191 - 39)
 *   onde 39 = ADC_8BIT na posicao de repouso, 191 = ADC_8BIT na posicao maxima
 *
 * Adc_ReadChannel_mV() retorna milivolts (uint32), identico ao
 * comportamento anterior de HAL_ADC_Get_V_ThrottlePedal1().
 */
void Update_RTE_p_ThrottlePedal(void)
{
    unsigned short int L_RTE_p_ThrottlePedal;

    /* Etapa 1: converte tensao em mV para ADC normalizado 0..255 */
    L_RTE_p_ThrottlePedal = (unsigned short int)
        ((unsigned long) Adc_ReadChannel_mV(ADC_CH_PEDAL) * 255UL / 5000UL);

    /* Etapa 2: linearizacao entre os pontos de calibracao do pedal */
    L_RTE_p_ThrottlePedal = (unsigned short int)
        ((((unsigned long) L_RTE_p_ThrottlePedal - 39UL) * 100UL) / (191UL - 39UL));

    S_RTE_p_ThrottlePedal = L_RTE_p_ThrottlePedal;
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
