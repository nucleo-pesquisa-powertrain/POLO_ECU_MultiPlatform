/**
 * \file Can.c
 * \brief Implementacao MCAL CAN para STM32H745 usando FDCAN
 *
 * Implementa a API Can.h usando o periferico FDCAN do STM32H7,
 * configurado em modo CAN classico (ISO 11898-1) a 500 kbps.
 *
 * Arquitetura:
 *   - FDCAN1 em modo Classic CAN (sem FD, DLC max 8 bytes)
 *   - TX via FIFO de transmissao (HAL_FDCAN_AddMessageToTxFifoQ)
 *   - RX via FIFO0 com filtro configurado para rxId especificado
 *   - Can_MainFunction() realiza polling do TX/RX para uso pelo XCP
 *   - Filtros RX: ID exato no FIFO0; frame invalidos rejeitados
 *
 * Configuracao de timing para 500 kbps com clock FDCAN = 80 MHz:
 *   Prescaler = 10 => tq = 10/80MHz = 125 ns
 *   Total TQ  = 16 => bit time = 16 * 125 ns = 2 us => 500 kbps
 *   Segmento 1 (Prop + Phase1) = 13 TQ
 *   Segmento 2 (Phase2)        =  2 TQ
 *   SJW                        =  1 TQ
 *   Amostragem: 87.5%
 *
 * TODO: Ajustar prescaler se o clock do FDCAN for diferente de 80 MHz.
 *       Usar calculadora de CAN bit timing (ex: Kvaser Calculator).
 *       O CubeMX mostra o clock do FDCAN na aba Clock Configuration.
 *
 * Plataforma : STM32H745
 * HAL        : stm32h7xx_hal_fdcan.h
 *
 * \note TODO: No CubeMX, configurar FDCAN1 com:
 *             - Frame Format: Classic mode
 *             - Mode: Normal
 *             - Auto Retransmission: Enable
 *             - TX FIFO Size: 3 (suficiente para XCP)
 *             - RX FIFO0 Size: 8
 *             - Habilitar interrupcoes RX FIFO0 e TX FIFO Empty
 */

#include "Can.h"
#include "stm32h7xx_hal.h"

/* ------------------------------------------------------------------ */
/* Handle do FDCAN (gerado pelo CubeMX)                              */
/* ------------------------------------------------------------------ */

/**
 * Handle do FDCAN1. Gerado pelo CubeMX em fdcan.c ou main.c.
 * TODO: Verificar nome gerado (hfdcan1 e' o padrao do CubeMX).
 */
extern FDCAN_HandleTypeDef hfdcan1;

/* ------------------------------------------------------------------ */
/* Constantes de timing CAN 500 kbps                                 */
/* ------------------------------------------------------------------ */

/**
 * Parametros de bit timing para 500 kbps com clock FDCAN = 80 MHz.
 * Calculado para amostragem a 87.5% do bit time.
 * TODO: Recalcular se o clock do FDCAN for diferente de 80 MHz.
 *       Formula: baudrate = ClockFreq / (Prescaler * (1 + Seg1 + Seg2))
 *       500000 = 80000000 / (10 * (1 + 13 + 2)) = 80000000 / 160
 */
#define CAN_BIT_PRESCALER           10U  /* Divisor do clock FDCAN              */
#define CAN_BIT_SEG1                13U  /* Propagation + Phase1 em TQ          */
#define CAN_BIT_SEG2                2U   /* Phase2 em TQ                        */
#define CAN_BIT_SJW                 1U   /* Sync Jump Width (tolerancia de erro) */

/* ------------------------------------------------------------------ */
/* Estado do modulo                                                   */
/* ------------------------------------------------------------------ */

/**
 * Armazena os IDs de TX e RX configurados durante Can_Init().
 * Usados para configurar o filtro RX e o header de TX.
 */
static uint32_t Can_TxId;
static uint32_t Can_RxId;

/**
 * Header de transmissao pre-configurado para reuso em Can_Write().
 * O DataLength e' atualizado a cada chamada de Can_Write().
 */
static FDCAN_TxHeaderTypeDef Can_TxHeader;

/** Flag de inicializacao */
static uint8_t Can_Initialized = 0U;

/* ------------------------------------------------------------------ */
/* Funcoes privadas                                                   */
/* ------------------------------------------------------------------ */

/**
 * Configura o filtro de recepcao do FDCAN para aceitar apenas rxId.
 * Usa filtro de ID exato (mascara = 0x7FF = match exato de 11 bits).
 * Mensagens com ID diferente sao rejeitadas pelo hardware.
 *
 * \param rxId CAN ID de recepcao (11-bit standard)
 * \return E_OK se filtro configurado, E_NOT_OK se falha HAL
 */
static Std_ReturnType Can_ConfigureRxFilter(uint32_t rxId)
{
    FDCAN_FilterTypeDef  filterCfg = {0};
    HAL_StatusTypeDef    status;

    filterCfg.IdType       = FDCAN_STANDARD_ID;
    filterCfg.FilterIndex  = 0U;              /* Filtro 0 */
    filterCfg.FilterType   = FDCAN_FILTER_MASK; /* ID + Mascara */
    filterCfg.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filterCfg.FilterID1    = rxId;            /* ID alvo */
    filterCfg.FilterID2    = 0x7FFU;          /* Mascara: todos os 11 bits devem coincidir */

    status = HAL_FDCAN_ConfigFilter(&hfdcan1, &filterCfg);
    return (status == HAL_OK) ? E_OK : E_NOT_OK;
}

/**
 * Converte numero de bytes DLC para constante HAL FDCAN_DLC_BYTES_x.
 *
 * \param len Numero de bytes (0..8)
 * \return Constante FDCAN_DLC_* correspondente
 */
static uint32_t Can_LenToDLC(uint8_t len)
{
    static const uint32_t dlcTable[9] =
    {
        FDCAN_DLC_BYTES_0,
        FDCAN_DLC_BYTES_1,
        FDCAN_DLC_BYTES_2,
        FDCAN_DLC_BYTES_3,
        FDCAN_DLC_BYTES_4,
        FDCAN_DLC_BYTES_5,
        FDCAN_DLC_BYTES_6,
        FDCAN_DLC_BYTES_7,
        FDCAN_DLC_BYTES_8,
    };

    if (len > 8U)
    {
        len = 8U; /* Satura em 8 bytes (CAN classico) */
    }

    return dlcTable[len];
}

/* ------------------------------------------------------------------ */
/* Implementacao da API publica                                       */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo CAN (FDCAN1 em modo classico).
 *
 * Configura o FDCAN1 com os parametros de bit timing para o baudrate
 * especificado na configuracao, configura o filtro de RX e inicia
 * o periferico.
 *
 * Atualmente suporta apenas 500 kbps (parametros fixos).
 * TODO: Implementar calculo dinamico de bit timing para suportar
 *       outros baudrates (250 kbps, 1 Mbps).
 *
 * \param config Ponteiro para configuracao (baudrate, rxId, txId)
 */
void Can_Init(const Can_ConfigType* config)
{
    HAL_StatusTypeDef status;

    if (config == NULL)
    {
        return;
    }

    /* Salva IDs para uso posterior */
    Can_TxId = config->txId;
    Can_RxId = config->rxId;

    /* Configura parametros de bit timing do FDCAN1.
     * O CubeMX pode ter pre-configurado, mas sobrescrevemos aqui
     * para garantir os valores corretos independente do projeto. */
    hfdcan1.Init.FrameFormat         = FDCAN_FRAME_CLASSIC;
    hfdcan1.Init.Mode                = FDCAN_MODE_NORMAL;
    hfdcan1.Init.AutoRetransmission  = ENABLE;
    hfdcan1.Init.TransmitPause       = DISABLE;
    hfdcan1.Init.ProtocolException   = ENABLE;

    /* Bit timing nominal (500 kbps com FDCAN clock = 80 MHz) */
    hfdcan1.Init.NominalPrescaler    = CAN_BIT_PRESCALER;
    hfdcan1.Init.NominalSyncJumpWidth = CAN_BIT_SJW;
    hfdcan1.Init.NominalTimeSeg1     = CAN_BIT_SEG1;
    hfdcan1.Init.NominalTimeSeg2     = CAN_BIT_SEG2;

    /* Em modo classico, os parametros de dados (FD) sao iguais ao nominal */
    hfdcan1.Init.DataPrescaler       = CAN_BIT_PRESCALER;
    hfdcan1.Init.DataSyncJumpWidth   = CAN_BIT_SJW;
    hfdcan1.Init.DataTimeSeg1        = CAN_BIT_SEG1;
    hfdcan1.Init.DataTimeSeg2        = CAN_BIT_SEG2;

    /* Tamanho dos FIFOs e buffers de mensagem */
    hfdcan1.Init.StdFiltersNbr       = 1U;   /* 1 filtro standard (para rxId) */
    hfdcan1.Init.ExtFiltersNbr       = 0U;   /* Sem filtros extended          */
    hfdcan1.Init.TxFifoQueueMode     = FDCAN_TX_FIFO_OPERATION;
    hfdcan1.Init.RxFifo0ElmtsNbr    = 8U;
    hfdcan1.Init.RxFifo0ElmtSize    = FDCAN_DATA_BYTES_8;
    hfdcan1.Init.TxFifoQueueElmtsNbr = 3U;
    hfdcan1.Init.TxElmtSize         = FDCAN_DATA_BYTES_8;

    /* Inicializa o periferico FDCAN via HAL */
    status = HAL_FDCAN_Init(&hfdcan1);
    if (status != HAL_OK)
    {
        /* TODO: Registrar erro de inicializacao do CAN */
        return;
    }

    /* Configura filtro de recepcao */
    if (Can_ConfigureRxFilter(config->rxId) != E_OK)
    {
        /* TODO: Registrar erro de configuracao do filtro */
        return;
    }

    /* Rejeita mensagens nao correspondentes ao filtro (va'o para FIFO1 ou descartadas).
     * FDCAN_REJECT: frames sem filtro correspondente sao descartados. */
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,
                                  FDCAN_REJECT,         /* Non-matching Standard */
                                  FDCAN_REJECT,         /* Non-matching Extended */
                                  FDCAN_FILTER_REMOTE,  /* Rejeita RTR standard  */
                                  FDCAN_FILTER_REMOTE); /* Rejeita RTR extended  */

    /* Pre-configura o header de TX (reaproveitado em cada Can_Write()) */
    Can_TxHeader.Identifier          = config->txId;
    Can_TxHeader.IdType              = FDCAN_STANDARD_ID;
    Can_TxHeader.TxFrameType         = FDCAN_DATA_FRAME;
    Can_TxHeader.DataLength          = FDCAN_DLC_BYTES_8; /* Sobrescrito em Can_Write */
    Can_TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    Can_TxHeader.BitRateSwitch       = FDCAN_BRS_OFF;     /* Sem FD bit rate switch */
    Can_TxHeader.FDFormat            = FDCAN_CLASSIC_CAN;
    Can_TxHeader.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
    Can_TxHeader.MessageMarker       = 0U;

    /* Inicia a comunicacao FDCAN */
    status = HAL_FDCAN_Start(&hfdcan1);
    if (status != HAL_OK)
    {
        /* TODO: Registrar erro de start do FDCAN */
        return;
    }

    Can_Initialized = 1U;
}

/**
 * \brief Transmite um frame CAN via FDCAN TX FIFO.
 *
 * Enfileira o frame no TX FIFO do FDCAN1. A transmissao ocorre
 * automaticamente pelo hardware quando o barramento CAN estiver livre.
 *
 * Retorna E_NOT_OK se o TX FIFO estiver cheio (sem espera).
 *
 * \param id   CAN ID (11-bit standard, <= 0x7FF)
 * \param data Ponteiro para dados (max 8 bytes)
 * \param len  Numero de bytes (0..8)
 * \return E_OK se enfileirado, E_NOT_OK se FIFO cheio ou nao inicializado
 */
Std_ReturnType Can_Write(uint32 id, const uint8* data, uint8 len)
{
    HAL_StatusTypeDef status;

    if ((Can_Initialized == 0U) || (data == NULL))
    {
        return E_NOT_OK;
    }

    /* Verifica espaco no FIFO antes de tentar enfileirar */
    if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) == 0U)
    {
        return E_NOT_OK; /* FIFO cheio - tentar novamente depois */
    }

    /* Atualiza o DLC com o tamanho atual */
    Can_TxHeader.Identifier = id;
    Can_TxHeader.DataLength = Can_LenToDLC(len);

    /* Enfileira o frame no TX FIFO.
     * HAL_FDCAN_AddMessageToTxFifoQ e' thread-safe em relacao ao hardware
     * mas nao em relacao a chamadas concorrentes de Can_Write().
     * TODO: Adicionar mutex se Can_Write() for chamada de multiplas tasks. */
    status = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &Can_TxHeader, (uint8_t*)data);

    return (status == HAL_OK) ? E_OK : E_NOT_OK;
}

/**
 * \brief Verifica e le um frame CAN do RX FIFO0.
 *
 * Verifica se ha mensagem disponivel no RX FIFO0 e copia para os
 * buffers fornecidos pelo chamador.
 *
 * \param id   [out] CAN ID do frame recebido
 * \param data [out] Buffer para dados (deve ter pelo menos 8 bytes)
 * \return E_OK se frame lido, E_NOT_OK se FIFO vazio
 */
Std_ReturnType Can_Read(uint32* id, uint8* data)
{
    FDCAN_RxHeaderTypeDef rxHeader = {0};
    HAL_StatusTypeDef     status;

    if ((Can_Initialized == 0U) || (id == NULL) || (data == NULL))
    {
        return E_NOT_OK;
    }

    /* Verifica se ha mensagem no FIFO0 */
    if (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) == 0U)
    {
        return E_NOT_OK; /* Nenhuma mensagem disponivel */
    }

    /* Le a mensagem do FIFO0 */
    status = HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &rxHeader, data);

    if (status != HAL_OK)
    {
        return E_NOT_OK;
    }

    /* Extrai o ID do header */
    *id = rxHeader.Identifier;

    return E_OK;
}

/**
 * \brief Verifica se a transmissao anterior foi concluida.
 *
 * Retorna TRUE se o TX FIFO tem pelo menos um slot livre,
 * indicando que o periferico esta' pronto para nova transmissao.
 *
 * \return TRUE se TX disponivel, FALSE se FIFO cheio
 */
boolean Can_IsTxComplete(void)
{
    if (Can_Initialized == 0U)
    {
        return FALSE;
    }

    return (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) > 0U) ? TRUE : FALSE;
}

/**
 * \brief Handler de polling - chamado periodicamente pelo XCP.
 *
 * Verifica o estado do FDCAN e processa erros de bus.
 * Para uso com XCP, verificar se ha mensagem RX disponivel e
 * notificar a camada de transporte XCP.
 *
 * TODO: Integrar com XCP transport layer:
 *       if (Can_Read(&id, data) == E_OK && id == Can_RxId) {
 *           Xcp_CanRxIndication(id, data, len);
 *       }
 */
void Can_MainFunction(void)
{
    FDCAN_ErrorCountersTypeDef errorCounters = {0};
    uint32_t                   protocolStatus;

    if (Can_Initialized == 0U)
    {
        return;
    }

    /* Verifica erros do protocolo CAN */
    protocolStatus = HAL_FDCAN_GetProtocolStatus(&hfdcan1, NULL);
    (void)protocolStatus; /* TODO: Processar erros de bus-off, error warning, etc. */

    /* Le contadores de erro TX e RX */
    HAL_FDCAN_GetErrorCounters(&hfdcan1, &errorCounters);
    /* TODO: Se TxErrorCnt > 96 (error passive threshold), registrar
     *       evento de diagnostico. Se = 255, e' bus-off: tentar recuperacao. */
    (void)errorCounters;

    /* TODO: Recuperacao de bus-off:
     * if (errorCounters.TxErrorCnt >= 255U) {
     *     HAL_FDCAN_Stop(&hfdcan1);
     *     HAL_FDCAN_Start(&hfdcan1);  // Tentativa de volta ao bus
     * }
     */
}
