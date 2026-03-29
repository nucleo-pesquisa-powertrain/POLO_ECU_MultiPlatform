/**
 * \file Adc_Cfg.h
 * \brief Mapeamento de canais logicos de ADC para canais fisicos do TC297B
 *
 * O TC297B possui o modulo VADC (Versatile ADC) com multiplos grupos (0..11).
 * Este projeto utiliza os grupos 0 a 5 em modo Background Scan, onde todos os
 * canais sao convertidos continuamente em background. A leitura e' feita
 * atraves da funcao readADCValue() do modulo ADC_Background_Scan.
 *
 * Mapeamento fisico:
 *   ADC_CH_TBI_POS       -> Grupo 0, AN0  (borboleta TPS1)
 *   ADC_CH_TBI_POS_RED   -> Grupo 0, AN2  (borboleta TPS2 redundante)
 *   ADC_CH_MAP           -> Grupo 0, AN3  (pressao do coletor MAP)
 *   ADC_CH_COOLANT_TEMP  -> Grupo 1, AN8  (temperatura da agua)
 *   ADC_CH_AIR_TEMP      -> Grupo 2, AN21 (temperatura do ar)
 *   ADC_CH_PEDAL         -> Grupo 3, AN24 (pedal do acelerador)
 *   ADC_CH_PEDAL_RED     -> Grupo 2, AN16 (pedal redundante)
 *   ADC_CH_VBATT         -> Grupo 2, AN17 (tensao da bateria)
 *   ADC_CH_LAMBDA1       -> Grupo 4, AN32 (sonda lambda 1)
 *   ADC_CH_LAMBDA2       -> Grupo 2, AN20 (sonda lambda 2)
 *   ADC_CH_KNOCK         -> Grupo 4, AN33 (sensor de detonacao)
 *   ADC_CH_AC_PRESS      -> Grupo 3, AN25 (pressao do A/C)
 *   ADC_CH_GENERATOR     -> Grupo 5, AN44 (tensao do alternador)
 *
 * Plataforma: Infineon AURIX TC297B
 * Driver iLLD: VADC Background Scan (readADCValue)
 */

#ifndef ADC_CFG_H
#define ADC_CFG_H

/* ------------------------------------------------------------------ */
/* Includes                                                            */
/* ------------------------------------------------------------------ */

#include "Platform_Types.h"

/* iLLD - tipos do VADC */
#include "IfxVadc.h"
#include "IfxVadc_Adc.h"

/* ------------------------------------------------------------------ */
/* Tipo: entrada da tabela de mapeamento de canal ADC                  */
/* ------------------------------------------------------------------ */

/**
 * \brief Associacao entre canal logico e canal fisico do VADC
 *
 * groupChannel e' o ponteiro para a estrutura de canal do iLLD,
 * correspondente ao g_vadcChannelX declarado no ADC_Background_Scan.
 *
 * channelId e' o identificador do canal dentro do grupo (IfxVadc_ChannelId),
 * usado para indexar o resultado no background scan.
 */
typedef struct
{
    IfxVadc_Adc_Channel* groupChannel;  /**< Ponteiro para canal iLLD (ex: &g_vadcChannel0) */
    IfxVadc_ChannelId    channelId;     /**< ID do canal no grupo (ex: IfxVadc_ChannelId_0)  */
} Adc_ChannelMapEntry;

/* ------------------------------------------------------------------ */
/* Declaracao do array de mapeamento (definido em Adc.c)               */
/* ------------------------------------------------------------------ */

/**
 * \brief Tabela de mapeamento: indice = canal logico ADC_CH_*
 *
 * Tamanho: ADC_NUM_CHANNELS (13 entradas)
 * Os ponteiros groupChannel apontam para as variaveis globais do modulo
 * ADC_Background_Scan (g_vadcChannel0 .. g_vadcChannel5).
 */
extern const Adc_ChannelMapEntry Adc_ChannelMap[];

/* ------------------------------------------------------------------ */
/* Identificadores de canal por grupo (alias para legibilidade)        */
/* ------------------------------------------------------------------ */

/* Grupo 0 - AN0..AN7 */
#define ADC_AN0_CHID    IfxVadc_ChannelId_0    /* TBI_POS      */
#define ADC_AN2_CHID    IfxVadc_ChannelId_2    /* TBI_POS_RED  */
#define ADC_AN3_CHID    IfxVadc_ChannelId_3    /* MAP          */

/* Grupo 1 - AN8..AN15 */
#define ADC_AN8_CHID    IfxVadc_ChannelId_0    /* COOLANT_TEMP (ch 0 do grupo 1) */

/* Grupo 2 - AN16..AN23 */
#define ADC_AN21_CHID   IfxVadc_ChannelId_5    /* AIR_TEMP     */
#define ADC_AN16_CHID   IfxVadc_ChannelId_0    /* PEDAL_RED    */
#define ADC_AN17_CHID   IfxVadc_ChannelId_1    /* VBATT        */
#define ADC_AN20_CHID   IfxVadc_ChannelId_4    /* LAMBDA2      */

/* Grupo 3 - AN24..AN31 */
#define ADC_AN24_CHID   IfxVadc_ChannelId_0    /* PEDAL        */
#define ADC_AN25_CHID   IfxVadc_ChannelId_1    /* AC_PRESS     */

/* Grupo 4 - AN32..AN39 */
#define ADC_AN32_CHID   IfxVadc_ChannelId_0    /* LAMBDA1      */
#define ADC_AN33_CHID   IfxVadc_ChannelId_1    /* KNOCK        */

/* Grupo 5 - AN40..AN47 */
#define ADC_AN44_CHID   IfxVadc_ChannelId_4    /* GENERATOR    */

#endif /* ADC_CFG_H */
