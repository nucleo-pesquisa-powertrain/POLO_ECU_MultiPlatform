/**
 * \file Spi.c
 * \brief Implementacao MCAL SPI para STM32H745
 *
 * Implementa a API Spi.h usando HAL_SPI do STM32H7.
 * Usado primariamente para comunicacao com o MC33810 (controlador
 * de saidas de ignicao/injecao da NXP).
 *
 * Configuracao SPI:
 *   - Modo Master, Full-duplex
 *   - Fase e polaridade: CPOL=0, CPHA=0 (Modo 0, SPI Mode 0)
 *     conforme datasheet MC33810 rev 4 secao 4.3.
 *     TODO: Verificar modo SPI no datasheet do MC33810 utilizado.
 *   - Tamanho do frame: 8-bit
 *   - MSB first (MC33810 exige MSB primeiro)
 *   - Frequencia: 5 MHz (limite do MC33810: 10 MHz max, usar margem)
 *     Com SPI clock = 100 MHz (APB2), Prescaler = 32: 100/32 = 3.125 MHz
 *     TODO: Ajustar prescaler conforme clock APB2 real no CubeMX.
 *   - NSS (Chip Select): gerenciado via GPIO (Software NSS)
 *     O CS do MC33810 e' controlado por Dio_WriteChannel(DIO_CH_MC33810_CS)
 *     na camada superior (Driver MC33810).
 *
 * Timeout de operacao: 10 ms (suficiente para frames de 1..8 bytes a 3 MHz).
 * Em producao, considerar DMA para liberar o CPU durante transferencias longas.
 *
 * Plataforma : STM32H745
 * HAL        : stm32h7xx_hal_spi.h
 *
 * \note TODO: No CubeMX, configurar SPI (ex: SPI1 ou SPI2) com:
 *             - Mode: Full-Duplex Master
 *             - Data Size: 8 Bits
 *             - First Bit: MSB First
 *             - Prescaler: ajustar para ~3..5 MHz
 *             - CPOL: Low (0), CPHA: 1 Edge (Modo 0)
 *             - NSS: Software (gerenciado por GPIO separado)
 *             Verificar se o SPI escolhido conflita com pinos usados pelo DIO.
 */

#include "Spi.h"
#include "stm32h7xx_hal.h"

/* ------------------------------------------------------------------ */
/* Handle do SPI (gerado pelo CubeMX)                                */
/* ------------------------------------------------------------------ */

/**
 * Handle do SPI. Gerado pelo CubeMX em spi.c ou main.c.
 * TODO: Ajustar para o numero de instancia SPI alocado no CubeMX.
 *       Opcoes tipicas: hspi1 (SPI1/APB2) ou hspi2 (SPI2/APB1).
 */
extern SPI_HandleTypeDef hspi1;

/* ------------------------------------------------------------------ */
/* Constantes de operacao                                             */
/* ------------------------------------------------------------------ */

/**
 * Timeout maximo para operacoes SPI bloqueantes em milissegundos.
 * 10 ms e' conservador para frames de ate' 64 bytes a 3 MHz.
 */
#define SPI_TIMEOUT_MS      10U

/* ------------------------------------------------------------------ */
/* Estado do modulo                                                   */
/* ------------------------------------------------------------------ */

/** Flag de inicializacao */
static uint8_t Spi_Initialized = 0U;

/* ------------------------------------------------------------------ */
/* Implementacao da API publica                                       */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo SPI em modo Master.
 *
 * Configura o SPI1 com os parametros necessarios para o MC33810
 * e inicia o periferico via HAL_SPI_Init().
 *
 * Se o CubeMX ja gerou MX_SPI1_Init(), esta funcao pode ser
 * simplificada para apenas verificar o estado do handle.
 *
 * TODO: Se o CubeMX gerar a init completa, remover a configuracao
 *       manual abaixo e manter apenas HAL_SPI_Init(&hspi1) ou nada.
 */
void Spi_Init(void)
{
    if (Spi_Initialized != 0U)
    {
        return;
    }

    /*
     * Configuracao do SPI para o MC33810:
     *   - BaudRatePrescaler: SPI_BAUDRATEPRESCALER_32
     *     Com APB2 = 100 MHz: 100/32 = 3.125 MHz (dentro do limite de 10 MHz)
     *   - DataSize: SPI_DATASIZE_8BIT
     *   - CLKPolarity: SPI_POLARITY_LOW (CPOL=0)
     *   - CLKPhase: SPI_PHASE_1EDGE (CPHA=0) => Modo SPI 0
     *   - NSS: SPI_NSS_SOFT (CS controlado por GPIO no driver MC33810)
     *   - FirstBit: SPI_FIRSTBIT_MSB
     *
     * TODO: Confirmar modo SPI (CPOL/CPHA) pelo datasheet MC33810.
     * TODO: Ajustar prescaler se o clock APB2 for diferente de 100 MHz.
     */
    hspi1.Instance               = SPI1;
    hspi1.Init.Mode              = SPI_MODE_MASTER;
    hspi1.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity       = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase          = SPI_PHASE_1EDGE;
    hspi1.Init.NSS               = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode            = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial     = 7U;
    hspi1.Init.CRCLength         = SPI_CRC_LENGTH_DATASIZE;
    hspi1.Init.NSSPMode          = SPI_NSS_PULSE_ENABLE;

    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        /* TODO: Registrar erro de inicializacao do SPI */
        return;
    }

    Spi_Initialized = 1U;
}

/**
 * \brief Transmite dados via SPI (somente TX, descarta RX).
 *
 * Transferencia bloqueante com timeout de SPI_TIMEOUT_MS.
 * O Chip Select deve ser controlado externamente pelo driver do
 * dispositivo (ex: Drv_MC33810) via DIO_CH_MC33810_CS.
 *
 * Sequencia tipica de uso:
 *   Dio_WriteChannel(DIO_CH_MC33810_CS, DIO_LOW);  // CS ativo
 *   Spi_Transmit(data, len);
 *   Dio_WriteChannel(DIO_CH_MC33810_CS, DIO_HIGH); // CS inativo
 *
 * TODO: Para operacoes nao-bloqueantes (uso em RTOS com tarefas de
 *       alta prioridade), implementar modo DMA:
 *       HAL_SPI_Transmit_DMA(&hspi1, data, len).
 *
 * \param data Ponteiro para buffer de dados TX
 * \param len  Numero de bytes a transmitir
 * \return E_OK se sucesso, E_NOT_OK se timeout ou erro HAL
 */
Std_ReturnType Spi_Transmit(const uint8* data, uint16 len)
{
    HAL_StatusTypeDef status;

    if ((Spi_Initialized == 0U) || (data == NULL) || (len == 0U))
    {
        return E_NOT_OK;
    }

    /* HAL_SPI_Transmit e' bloqueante: aguarda conclusao ou timeout.
     * O cast remove o qualifier const; o HAL nao modifica o buffer TX.
     * TODO: Reportar bug ao ST se o HAL exigir uint8_t* nao-const. */
    status = HAL_SPI_Transmit(&hspi1,
                               (uint8_t*)data,
                               (uint16_t)len,
                               SPI_TIMEOUT_MS);

    return (status == HAL_OK) ? E_OK : E_NOT_OK;
}

/**
 * \brief Transmite e recebe dados via SPI em modo full-duplex.
 *
 * Operacao simultanea de TX e RX. Necessaria para leitura de registros
 * de status do MC33810 (o MC33810 envia status enquanto recebe comando).
 *
 * Ambos os buffers devem ter o mesmo tamanho (len bytes).
 * O buffer rxData sera' preenchido com os dados recebidos do slave.
 *
 * \param txData Ponteiro para buffer TX (dados a enviar)
 * \param rxData Ponteiro para buffer RX (dados recebidos)
 * \param len    Numero de bytes (TX e RX simetricos)
 * \return E_OK se sucesso, E_NOT_OK se timeout ou erro HAL
 */
Std_ReturnType Spi_TransmitReceive(const uint8* txData, uint8* rxData, uint16 len)
{
    HAL_StatusTypeDef status;

    if ((Spi_Initialized == 0U) || (txData == NULL) || (rxData == NULL) || (len == 0U))
    {
        return E_NOT_OK;
    }

    status = HAL_SPI_TransmitReceive(&hspi1,
                                      (uint8_t*)txData,
                                      rxData,
                                      (uint16_t)len,
                                      SPI_TIMEOUT_MS);

    return (status == HAL_OK) ? E_OK : E_NOT_OK;
}

/**
 * \brief Verifica se o SPI esta' ocupado.
 *
 * Retorna TRUE se o HAL reportar estado diferente de READY.
 * Util para verificar conclusao de operacoes DMA antes de iniciar nova.
 *
 * \return TRUE se ocupado, FALSE se livre para nova operacao
 */
boolean Spi_IsBusy(void)
{
    if (Spi_Initialized == 0U)
    {
        return FALSE;
    }

    return (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY) ? TRUE : FALSE;
}
