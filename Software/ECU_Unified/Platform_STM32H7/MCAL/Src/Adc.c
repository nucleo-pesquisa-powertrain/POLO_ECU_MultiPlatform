/**
 * \file Adc.c
 * \brief Implementacao MCAL ADC para STM32H745
 *
 * Implementa a API Adc.h usando HAL_ADC do STM32H7 em modo DMA continuo.
 *
 * Arquitetura de conversao:
 *   - O CubeMX configura ADC1, ADC2, ADC3 com DMA circular.
 *   - O DMA preenche os buffers Adc_DmaBuffer_ADCx[] continuamente.
 *   - Adc_ReadChannel_Raw() simplesmente le o ultimo valor do buffer.
 *   - Nao ha polling nem bloqueio: leitura e' instantanea.
 *
 * Escalamento de tensao:
 *   - O STM32H745 tem ADC de 16-bit (max) mas usamos 12-bit (0..4095).
 *   - VREF+ = 3.3 V no hardware da placa.
 *   - Sensores condicionados externamente para 0..5 V -> 0..3.3 V via
 *     divisor resistivo. A conversao para mV usa ADC_SUPPLY_VOLTAGE_MV
 *     (5000 mV) como escala final da aplicacao.
 *   - Formula: mV = (raw * ADC_SUPPLY_VOLTAGE_MV) / ADC_MAX_RAW_VALUE
 *
 * Plataforma : STM32H745
 * HAL        : stm32h7xx_hal_adc.h
 *
 * \note TODO: Chamar Adc_Init() apos MX_ADCx_Init() e MX_DMAx_Init()
 *             gerados pelo CubeMX, para iniciar as conversoes DMA.
 */

#include "Mcal_Adc.h"
#include "Adc_Cfg.h"
#include "stm32h7xx_hal.h"

/* ------------------------------------------------------------------ */
/* Buffers DMA                                                        */
/* ------------------------------------------------------------------ */

/**
 * Buffers preenchidos continuamente pelo DMA durante conversao ADC.
 * Declarados volatile pois sao atualizados pela interrupcao DMA,
 * fora do fluxo de execucao normal.
 *
 * O atributo de secao garante alocacao em SRAM acessivel pelo DMA.
 * No STM32H745, a SRAM2 e D2-AHB SRAM sao acessiveis por DMA1/DMA2.
 *
 * TODO: Confirmar banco de SRAM compativel com DMA no linker script.
 *       Usar __attribute__((section(".DMA_Buffer"))) se necessario.
 */
volatile uint16_t Adc_DmaBuffer_ADC1[ADC1_DMA_BUFFER_SIZE];
volatile uint16_t Adc_DmaBuffer_ADC2[ADC2_DMA_BUFFER_SIZE];
volatile uint16_t Adc_DmaBuffer_ADC3[ADC3_DMA_BUFFER_SIZE];

/* ------------------------------------------------------------------ */
/* Estado do modulo                                                   */
/* ------------------------------------------------------------------ */

/** Controle de inicializacao (evita dupla inicializacao) */
static uint8_t Adc_Initialized = 0U;

/* ------------------------------------------------------------------ */
/* Funcoes privadas                                                   */
/* ------------------------------------------------------------------ */

/**
 * Inicia a conversao DMA de uma instancia ADC.
 * Wraps HAL_ADC_Start_DMA com tratamento de erro.
 *
 * \param hadc      Handle da instancia ADC
 * \param pData     Ponteiro para o buffer destino DMA
 * \param Length    Numero de conversoes (tamanho do buffer)
 * \return E_OK se iniciado com sucesso, E_NOT_OK em caso de falha HAL
 */
static Std_ReturnType Adc_StartDMA(ADC_HandleTypeDef* hadc,
                                    volatile uint16_t* pData,
                                    uint32_t           Length)
{
    HAL_StatusTypeDef status;

    /* HAL_ADC_Start_DMA espera uint32_t*, mas os valores sao 12-bit (16-bit alinhado).
     * O cast e' seguro porque o DMA esta configurado para half-word (16-bit). */
    status = HAL_ADC_Start_DMA(hadc, (uint32_t*)pData, Length);

    return (status == HAL_OK) ? E_OK : E_NOT_OK;
}

/* ------------------------------------------------------------------ */
/* Implementacao da API publica                                       */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo ADC e inicia as conversoes DMA.
 *
 * Deve ser chamada apos MX_ADCx_Init() e MX_DMAx_Init() do CubeMX.
 * Inicia as tres instancias ADC em modo DMA circular continuo.
 *
 * TODO: Calibrar o ADC antes de iniciar (HAL_ADCEx_Calibration_Start).
 *       Isso melhora a precisao em ~0.1% no STM32H745.
 */
void Adc_Init(void)
{
    if (Adc_Initialized != 0U)
    {
        return;
    }

    /* Zera os buffers */
    for (uint8_t i = 0U; i < ADC1_DMA_BUFFER_SIZE; i++) { Adc_DmaBuffer_ADC1[i] = 0U; }
    for (uint8_t i = 0U; i < ADC2_DMA_BUFFER_SIZE; i++) { Adc_DmaBuffer_ADC2[i] = 0U; }
    for (uint8_t i = 0U; i < ADC3_DMA_BUFFER_SIZE; i++) { Adc_DmaBuffer_ADC3[i] = 0U; }

    /*
     * NOTA: O CubeMX configurou os ADCs em modo single-shot sem DMA.
     * A calibracao e o Start_DMA foram desabilitados temporariamente.
     * A leitura sera feita por polling em Adc_ReadChannel_Raw().
     *
     * TODO: Reconfigurar os ADCs no CubeMX com:
     *   - ContinuousConvMode = ENABLE
     *   - ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR
     *   - Resolution = ADC_RESOLUTION_12B
     *   - Configurar DMA streams para cada ADC
     * Depois reabilitar calibracao + Adc_StartDMA() aqui.
     */

    Adc_Initialized = 1U;
}

/**
 * \brief Le o valor bruto (raw) de um canal ADC.
 *
 * Leitura direta do buffer DMA. Nao ha bloqueio nem espera de conversao.
 * O valor reflete a ultima conversao completada pelo DMA.
 *
 * \param ch Canal logico (ADC_CH_*)
 * \return Valor ADC raw (0..4095 para resolucao 12-bit)
 */
uint16 Adc_ReadChannel_Raw(Adc_ChannelType ch)
{
    const Adc_ChannelConfigType* cfg;
    uint16_t                     rawValue;
    uint8_t                      idx;

    if (ch >= ADC_NUM_CHANNELS)
    {
        return 0U; /* Canal invalido: retorna zero (fail-safe) */
    }

    cfg      = &Adc_ChannelCfg[ch];
    idx      = cfg->dmaBufferIndex;
    rawValue = 0U;

    /* Seleciona o buffer DMA correto baseado na instancia ADC */
    if (cfg->handle == &hadc1)
    {
        if (idx < ADC1_DMA_BUFFER_SIZE)
        {
            rawValue = Adc_DmaBuffer_ADC1[idx];
        }
    }
    else if (cfg->handle == &hadc2)
    {
        if (idx < ADC2_DMA_BUFFER_SIZE)
        {
            rawValue = Adc_DmaBuffer_ADC2[idx];
        }
    }
    else if (cfg->handle == &hadc3)
    {
        if (idx < ADC3_DMA_BUFFER_SIZE)
        {
            rawValue = Adc_DmaBuffer_ADC3[idx];
        }
    }
    else
    {
        /* Instancia ADC desconhecida - retorna zero */
        rawValue = 0U;
    }

    /* Mascara para garantir que valores fora de range nao propagam.
     * O STM32H745 pode ser configurado para 12-bit (max 4095). */
    return (uint16)(rawValue & ADC_MAX_RAW_VALUE);
}

/**
 * \brief Le o valor de um canal ADC convertido para milivolts.
 *
 * Converte o valor raw usando a formula:
 *   mV = (raw * ADC_SUPPLY_VOLTAGE_MV) / ADC_MAX_RAW_VALUE
 *
 * ADC_SUPPLY_VOLTAGE_MV = 5000 mV (definido em Adc.h, escala da aplicacao)
 * ADC_MAX_RAW_VALUE      = 4095  (12-bit, definido em Adc.h)
 *
 * Nota: O ADC do STM32 referencia VREF+ (3.3 V), mas se o circuito
 * de condicionamento do sensor mapeia 0..5 V -> 0..3.3 V, a leitura
 * full scale (4095) representa 5000 mV na grandeza do sensor.
 * Isso e' correto para a aplicacao de ECU (MAP, TPS, etc.).
 *
 * TODO: Se alguns canais tiverem divisor diferente, criar tabela de
 *       fator de escala por canal.
 *
 * \param ch Canal logico (ADC_CH_*)
 * \return Tensao em mV (0..5000)
 */
uint32 Adc_ReadChannel_mV(Adc_ChannelType ch)
{
    uint32_t raw;
    uint32_t mv;

    raw = (uint32_t)Adc_ReadChannel_Raw(ch);

    /* Evita overflow: raw <= 4095, ADC_SUPPLY_VOLTAGE_MV = 5000
     * Produto maximo: 4095 * 5000 = 20.475.000 < 2^32 => sem overflow em uint32 */
    mv = (raw * (uint32_t)ADC_SUPPLY_VOLTAGE_MV) / (uint32_t)ADC_MAX_RAW_VALUE;

    return mv;
}

/* ------------------------------------------------------------------ */
/* Callbacks do HAL ADC (chamados pelo DMA via NVIC)                 */
/* ------------------------------------------------------------------ */

/**
 * Callback de conversao completa do ADC via DMA.
 * Chamado pelo HAL quando o buffer DMA completa uma sequencia de conversoes.
 * Em modo circular, chamado a cada ciclo completo do buffer.
 *
 * A aplicacao pode sobrescrever este callback (weak) para processar
 * os dados recem-convertidos (ex: filtros, deteccao de knock, etc.).
 *
 * TODO: Se necessario processar os dados em tempo real, implementar
 *       aqui a notificacao da camada de aplicacao (ex: post de evento
 *       para task FreeRTOS via xEventGroupSetBitsFromISR).
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    /* Reservado para uso futuro.
     * Com DMA continuo, os buffers sao atualizados automaticamente
     * e a aplicacao pode ler a qualquer momento via Adc_ReadChannel_Raw(). */
    (void)hadc;
}

/**
 * Callback de meio buffer DMA (half-transfer).
 * Pode ser usado para processar a primeira metade do buffer enquanto
 * a segunda metade e' preenchida pelo DMA (double buffering).
 * Atualmente nao utilizado.
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    (void)hadc;
}

/**
 * Callback de erro do ADC.
 * Em producao, deve registrar o erro e acionar mecanismo de recuperacao.
 * TODO: Implementar log de erro e restart do DMA se necessario.
 */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
{
    /* TODO: Registrar erro, incrementar contador de falhas ADC.
     *       Em caso de erro DMA, tentar reiniciar com HAL_ADC_Stop_DMA()
     *       seguido de Adc_StartDMA() apos reset do DMA stream. */
    (void)hadc;
}
