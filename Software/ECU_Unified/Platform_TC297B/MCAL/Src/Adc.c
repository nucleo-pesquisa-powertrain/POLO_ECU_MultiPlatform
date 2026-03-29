/**
 * \file Adc.c
 * \brief Implementacao MCAL ADC para Infineon AURIX TC297B
 *
 * Encapsula a funcao readADCValue() do modulo ADC_Background_Scan do iLLD.
 * O VADC opera em modo Background Scan continuo: todos os canais configurados
 * sao convertidos ciclicamente pelo hardware. A leitura via readADCValue()
 * retorna sempre o resultado mais recente sem bloquear.
 *
 * Conversao de raw para milivolts:
 *   mV = (raw * ADC_SUPPLY_VOLTAGE_MV) / ADC_MAX_RAW_VALUE
 *      = (raw * 5000) / 4095
 *
 * Plataforma: Infineon AURIX TC297B
 * Driver iLLD: VADC Background Scan (IfxVadc_Adc, readADCValue)
 */

/* ------------------------------------------------------------------ */
/* Includes                                                            */
/* ------------------------------------------------------------------ */

#include "Adc.h"
#include "Adc_Cfg.h"

/* iLLD - VADC */
#include "IfxVadc.h"
#include "IfxVadc_Adc.h"

/*
 * Declaracao externa das variaveis de canal do modulo ADC_Background_Scan.
 * Estas variaveis sao definidas e inicializadas no BSP do projeto
 * (tipicamente em ADC_Background_Scan.c / Cpu0_Main.c).
 */
extern IfxVadc_Adc_Channel g_vadcChannel0;  /* Grupo 0: AN0..AN7  */
extern IfxVadc_Adc_Channel g_vadcChannel1;  /* Grupo 1: AN8..AN15 */
extern IfxVadc_Adc_Channel g_vadcChannel2;  /* Grupo 2: AN16..AN23 */
extern IfxVadc_Adc_Channel g_vadcChannel3;  /* Grupo 3: AN24..AN31 */
extern IfxVadc_Adc_Channel g_vadcChannel4;  /* Grupo 4: AN32..AN39 */
extern IfxVadc_Adc_Channel g_vadcChannel5;  /* Grupo 5: AN40..AN47 */

/* ------------------------------------------------------------------ */
/* Tabela de mapeamento: canal logico -> canal fisico VADC             */
/* ------------------------------------------------------------------ */
/*
 * A ordem das entradas DEVE seguir a numeracao dos canais ADC_CH_*
 * definidos em Adc.h. O canal logico e' usado diretamente como indice.
 *
 * groupChannel: ponteiro para a estrutura iLLD do canal, que ja contem
 *               o grupo e o handle necessarios para a leitura.
 * channelId:    indice do canal dentro do grupo para acesso ao resultado
 *               no registrador de resultado do VADC.
 */
const Adc_ChannelMapEntry Adc_ChannelMap[ADC_NUM_CHANNELS] =
{
    /* [ 0] ADC_CH_TBI_POS      */ { &g_vadcChannel0, ADC_AN0_CHID  },
    /* [ 1] ADC_CH_TBI_POS_RED  */ { &g_vadcChannel0, ADC_AN2_CHID  },
    /* [ 2] ADC_CH_MAP          */ { &g_vadcChannel0, ADC_AN3_CHID  },
    /* [ 3] ADC_CH_COOLANT_TEMP */ { &g_vadcChannel1, ADC_AN8_CHID  },
    /* [ 4] ADC_CH_AIR_TEMP     */ { &g_vadcChannel2, ADC_AN21_CHID },
    /* [ 5] ADC_CH_PEDAL        */ { &g_vadcChannel3, ADC_AN24_CHID },
    /* [ 6] ADC_CH_PEDAL_RED    */ { &g_vadcChannel2, ADC_AN16_CHID },
    /* [ 7] ADC_CH_VBATT        */ { &g_vadcChannel2, ADC_AN17_CHID },
    /* [ 8] ADC_CH_LAMBDA1      */ { &g_vadcChannel4, ADC_AN32_CHID },
    /* [ 9] ADC_CH_LAMBDA2      */ { &g_vadcChannel2, ADC_AN20_CHID },
    /* [10] ADC_CH_KNOCK        */ { &g_vadcChannel4, ADC_AN33_CHID },
    /* [11] ADC_CH_AC_PRESS     */ { &g_vadcChannel3, ADC_AN25_CHID },
    /* [12] ADC_CH_GENERATOR    */ { &g_vadcChannel5, ADC_AN44_CHID }
};

/* ------------------------------------------------------------------ */
/* Implementacao da API                                                */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo ADC
 *
 * O VADC Background Scan e' inicializado pelo BSP antes de qualquer
 * tarefa da aplicacao. Esta funcao valida que o background scan esta
 * ativo e registra os canais que esta camada MCAL utiliza.
 *
 * \note A configuracao do hardware VADC (prescaler, tempo de amostragem,
 *       resolucao) deve ser feita no modulo ADC_Background_Scan do BSP.
 *       Aqui apenas verificamos que o modulo esta pronto.
 */
void Adc_Init(void)
{
    /*
     * O Background Scan do VADC e' inicializado em ADC_Background_Scan_Init()
     * chamada no Cpu0_Main.c antes desta funcao.
     * Nao ha parametros adicionais a configurar nesta camada.
     * Esta funcao existe para satisfazer o contrato da API MCAL e
     * permitir eventuais verificacoes de sanidade no futuro.
     */
}

/**
 * \brief Le o valor bruto (raw) de um canal ADC
 *
 * Retorna o resultado mais recente do Background Scan para o canal
 * solicitado. A leitura e' nao-bloqueante (retorna o ultimo valor
 * convertido pelo hardware).
 *
 * \param ch Canal logico (ADC_CH_*)
 * \return Valor ADC 12-bit (0..4095), ou 0 se canal invalido
 */
uint16 Adc_ReadChannel_Raw(Adc_ChannelType ch)
{
    Ifx_VADC_RES result;

    if (ch >= ADC_NUM_CHANNELS)
    {
        return 0u;
    }

    /*
     * readADCValue() e' a funcao do modulo ADC_Background_Scan que le
     * o registrador de resultado (RESREG) do canal especificado.
     * Ela retorna Ifx_VADC_RES com o campo .B.RESULT contendo o valor
     * de 12 bits (bits 11:0) quando a conversao estiver valida (VF=1).
     */
    result = IfxVadc_Adc_getResult(Adc_ChannelMap[ch].groupChannel);

    /* Verificar bit de validade (Valid Flag) antes de retornar */
    if (result.B.VF == 0u)
    {
        /* Conversao ainda nao disponivel - retorna ultimo valor ou zero */
        return 0u;
    }

    return (uint16)(result.B.RESULT & ADC_MAX_RAW_VALUE);
}

/**
 * \brief Le o valor de um canal ADC convertido para milivolts
 *
 * Aplica a formula: mV = (raw * 5000) / 4095
 * A divisao e' feita com aritmetica de 32 bits para evitar overflow.
 *
 * \param ch Canal logico (ADC_CH_*)
 * \return Tensao em mV (0..5000), ou 0 se canal invalido
 */
uint32 Adc_ReadChannel_mV(Adc_ChannelType ch)
{
    uint32 raw;

    raw = (uint32)Adc_ReadChannel_Raw(ch);

    /* mV = raw * 5000 / 4095 */
    return (raw * (uint32)ADC_SUPPLY_VOLTAGE_MV) / (uint32)ADC_MAX_RAW_VALUE;
}
