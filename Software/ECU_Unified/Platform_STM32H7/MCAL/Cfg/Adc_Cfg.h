/**
 * \file Adc_Cfg.h
 * \brief Configuracao dos canais ADC para STM32H745
 *
 * Mapeia os canais logicos ADC (definidos em Adc.h) para as instancias
 * ADC fisicas do STM32H745 e seus respectivos numeros de canal.
 *
 * O STM32H745 possui tres instancias ADC:
 *   ADC1 e ADC2: conectados a GPIO dos bancos 1 e 2 (GPIOA, GPIOB, GPIOC)
 *   ADC3       : acesso independente, usado para sensores criticos
 *
 * Modo de operacao configurado no CubeMX:
 *   - Conversao continua com DMA circular (double buffer)
 *   - Resolucao 12-bit (0..4095)
 *   - Referencia: VREF+ = 3.3 V (hardware) ou 5 V via divisor resistivo
 *
 * A camada de aplicacao usa ADC_SUPPLY_VOLTAGE_MV (definido em Adc.h)
 * para converter raw -> mV. Ajustar se VREF do ADC for diferente de 5V.
 *
 * Plataforma : STM32H745
 * HAL        : stm32h7xx_hal_adc.h
 *
 * \note TODO: Todos os canais abaixo sao PLACEHOLDER.
 *             Ajustar conforme esquematico e configuracao CubeMX.
 *             Verificar sequencia de conversao no ADC e posicao no buffer DMA.
 */
#ifndef ADC_CFG_H
#define ADC_CFG_H

#include "stm32h7xx_hal.h"
#include "Adc.h"

/* ------------------------------------------------------------------ */
/* Handles ADC (gerados pelo CubeMX em adc.c / main.c)              */
/* ------------------------------------------------------------------ */

/*
 * Declaracao extern dos handles HAL gerados pelo CubeMX.
 * TODO: Verificar se o projeto CubeMX gerou hadc1, hadc2, hadc3.
 *       Alguns projetos usam apenas ADC1 com multiplexacao por DMA.
 */
extern ADC_HandleTypeDef hadc1;  /* ADC1: sensores principais          */
extern ADC_HandleTypeDef hadc2;  /* ADC2: sensores redundantes/extras  */
extern ADC_HandleTypeDef hadc3;  /* ADC3: knock + bateria (isolados)   */

/* ------------------------------------------------------------------ */
/* Tipo de configuracao de canal ADC                                  */
/* ------------------------------------------------------------------ */

/**
 * Estrutura que descreve um canal logico ADC.
 * O campo dmaBufferIndex indica a posicao no buffer circular DMA
 * preenchido automaticamente pelo HAL em modo DMA continuo.
 */
typedef struct
{
    ADC_HandleTypeDef* handle;          /**< Instancia ADC HAL                */
    uint32_t           adcChannel;      /**< Canal HAL (ADC_CHANNEL_x)        */
    uint8_t            dmaBufferIndex;  /**< Posicao no buffer DMA desta ADC  */
} Adc_ChannelConfigType;

/* ------------------------------------------------------------------ */
/* Buffer DMA global (preenchido automaticamente pelo HAL)            */
/* ------------------------------------------------------------------ */

/**
 * Numero de canais convertidos por cada instancia ADC.
 * Ajustar se canais forem adicionados ou removidos.
 *
 * TODO: Deve coincidir com o numero de canais na sequencia configurada
 *       no CubeMX (aba ADC -> Rank de cada canal).
 */
#define ADC1_DMA_BUFFER_SIZE    8U   /* TPS1, TPS2, MAP, ECT, IAT, PPS, PPS2, VBAT */
#define ADC2_DMA_BUFFER_SIZE    2U   /* LAMBDA1, LAMBDA2                             */
#define ADC3_DMA_BUFFER_SIZE    3U   /* KNOCK, AC_PRESS, GENERATOR                   */

/**
 * Declaracao extern dos buffers DMA.
 * Definidos em Adc.c e preenchidos pela interrupcao DMA do HAL.
 * Volatile porque sao escritos fora do fluxo normal de execucao.
 *
 * TODO: Se o CubeMX gerar os buffers automaticamente, referenciar os
 *       nomes gerados aqui (ex: adc1ConvBuf gerado em adc.c).
 */
extern volatile uint16_t Adc_DmaBuffer_ADC1[ADC1_DMA_BUFFER_SIZE];
extern volatile uint16_t Adc_DmaBuffer_ADC2[ADC2_DMA_BUFFER_SIZE];
extern volatile uint16_t Adc_DmaBuffer_ADC3[ADC3_DMA_BUFFER_SIZE];

/* ------------------------------------------------------------------ */
/* Tabela de mapeamento canal logico -> fisico                        */
/* Indexada por ADC_CH_* definidos em Adc.h                          */
/* ------------------------------------------------------------------ */

/*
 * TODO: Todos os canais ADC abaixo sao PLACEHOLDER.
 *
 * Mapeamento de pinos ADC do STM32H745 (banco 1/2):
 *   ADC1_IN0  = PA0    ADC1_IN1  = PA1    ADC1_IN2  = PA2
 *   ADC1_IN3  = PA3    ADC1_IN4  = PA4    ADC1_IN5  = PA5
 *   ADC1_IN6  = PA6    ADC1_IN7  = PA7    ADC1_IN8  = PB0
 *   ADC1_IN9  = PB1    ADC1_IN10 = PC0    ADC1_IN11 = PC1
 *   ADC2_IN*  : mesmo mapeamento de pino que ADC1 (compartilhados)
 *   ADC3_IN0  = PA0_C  ADC3_IN1  = PA1_C  (pinos exclusivos ADC3)
 *   ADC3_IN4  = PC4    ADC3_IN5  = PB1    ADC3_IN6  = PB0
 *
 * Atribuicao proposta (a confirmar com esquematico):
 *   ADC_CH_TBI_POS      -> ADC1 IN3 (PA3)   dmaIdx 0
 *   ADC_CH_TBI_POS_RED  -> ADC1 IN4 (PA4)   dmaIdx 1
 *   ADC_CH_MAP          -> ADC1 IN5 (PA5)   dmaIdx 2
 *   ADC_CH_COOLANT_TEMP -> ADC1 IN6 (PA6)   dmaIdx 3
 *   ADC_CH_AIR_TEMP     -> ADC1 IN7 (PA7)   dmaIdx 4
 *   ADC_CH_PEDAL        -> ADC1 IN8 (PB0)   dmaIdx 5
 *   ADC_CH_PEDAL_RED    -> ADC1 IN9 (PB1)   dmaIdx 6
 *   ADC_CH_VBATT        -> ADC1 IN10(PC0)   dmaIdx 7
 *   ADC_CH_LAMBDA1      -> ADC2 IN3 (PA3)   dmaIdx 0  (ADC2)
 *   ADC_CH_LAMBDA2      -> ADC2 IN4 (PA4)   dmaIdx 1  (ADC2)
 *   ADC_CH_KNOCK        -> ADC3 IN4 (PC4)   dmaIdx 0  (ADC3)
 *   ADC_CH_AC_PRESS     -> ADC3 IN5 (PB1_C) dmaIdx 1  (ADC3)
 *   ADC_CH_GENERATOR    -> ADC3 IN6 (PB0_C) dmaIdx 2  (ADC3)
 */
static const Adc_ChannelConfigType Adc_ChannelCfg[ADC_NUM_CHANNELS] =
{
    /* [ADC_CH_TBI_POS = 0]     */ { &hadc1, ADC_CHANNEL_3,  0U },
    /* [ADC_CH_TBI_POS_RED = 1] */ { &hadc1, ADC_CHANNEL_4,  1U },
    /* [ADC_CH_MAP = 2]         */ { &hadc1, ADC_CHANNEL_5,  2U },
    /* [ADC_CH_COOLANT_TEMP = 3]*/ { &hadc1, ADC_CHANNEL_6,  3U },
    /* [ADC_CH_AIR_TEMP = 4]    */ { &hadc1, ADC_CHANNEL_7,  4U },
    /* [ADC_CH_PEDAL = 5]       */ { &hadc1, ADC_CHANNEL_8,  5U },
    /* [ADC_CH_PEDAL_RED = 6]   */ { &hadc1, ADC_CHANNEL_9,  6U },
    /* [ADC_CH_VBATT = 7]       */ { &hadc1, ADC_CHANNEL_10, 7U },
    /* [ADC_CH_LAMBDA1 = 8]     */ { &hadc2, ADC_CHANNEL_3,  0U },
    /* [ADC_CH_LAMBDA2 = 9]     */ { &hadc2, ADC_CHANNEL_4,  1U },
    /* [ADC_CH_KNOCK = 10]      */ { &hadc3, ADC_CHANNEL_4,  0U },
    /* [ADC_CH_AC_PRESS = 11]   */ { &hadc3, ADC_CHANNEL_5,  1U },
    /* [ADC_CH_GENERATOR = 12]  */ { &hadc3, ADC_CHANNEL_6,  2U },
};

/* ------------------------------------------------------------------ */
/* Referencia de tensao do ADC                                        */
/* ------------------------------------------------------------------ */

/**
 * Tensao de referencia do ADC em milivolts (VREF+).
 * O STM32H745 opera com VREF+ = 3.3 V internamente, mas os sensores
 * podem ser condicionados para 0..5 V via divisor resistivo externo.
 *
 * Se o circuito de condicionamento mapear 0..5V -> 0..3.3V, usar
 * ADC_VREF_MV = 3300 e escalar na aplicacao com fator de 5000/3300.
 *
 * TODO: Definir circuito de condicionamento e ajustar este valor.
 *       ADC_SUPPLY_VOLTAGE_MV (Adc.h) define a escala da aplicacao.
 */
#define ADC_VREF_MV                 3300U   /* TODO: Confirmar VREF+ do hardware */

/* ------------------------------------------------------------------ */
/* Configuracao de over-sampling (opcional)                           */
/* ------------------------------------------------------------------ */

/**
 * O STM32H745 suporta over-sampling por hardware (ate' 1024x).
 * Habilitar no CubeMX pode melhorar SNR do sinal de knock.
 * TODO: Configurar over-sampling do ADC3 (knock) no CubeMX se necessario.
 */

/* ------------------------------------------------------------------ */
/* Configuracao DMA                                                   */
/* ------------------------------------------------------------------ */

/**
 * O DMA deve ser configurado no CubeMX com as seguintes opcoes:
 *   - Mode: Circular
 *   - Data Width: Word (32-bit) para alinhamento, ou Half-Word (16-bit)
 *   - Incremento de endereco: habilitado no lado memoria
 *   - Interrupcao Half-Transfer e Transfer-Complete: habilitadas
 *
 * TODO: Mapear corretamente os streams DMA para cada ADC no CubeMX:
 *   ADC1 -> DMA1 Stream 1 (ou conforme disponibilidade)
 *   ADC2 -> DMA1 Stream 2
 *   ADC3 -> DMA2 Stream 0
 */

#endif /* ADC_CFG_H */
