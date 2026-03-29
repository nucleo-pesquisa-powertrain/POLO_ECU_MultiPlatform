/**
 * \file Can.c
 * \brief Implementacao MCAL CAN para Infineon AURIX TC297B
 *
 * Encapsula o modulo MultiCAN do TC297B para fornecer comunicacao CAN
 * usada primariamente pelo protocolo XCP (calibracao/medicao via INCA).
 *
 * Configuracao de hardware:
 *   - Node: CAN Node 0 (MultiCAN)
 *   - Baudrate: configuravel via Can_ConfigType (tipicamente 500 kbps)
 *   - TX Message Object: MO1 (Message Object 1)
 *   - RX Message Object: MO0 (Message Object 0)
 *   - Frame: CAN 2.0A (11-bit standard ID)
 *
 * Arquitetura de message objects:
 *   MO0 (RX): aceita frames com ID = config->rxId, mascara = 0x7FF (exato)
 *   MO1 (TX): transmite frames com ID = config->txId
 *
 * Uso de polling vs interrupcao:
 *   Esta implementacao usa polling via Can_MainFunction() para verificar
 *   TX completion e receber frames, compativel com o modelo XCP transport.
 *   Nao utiliza interrupcoes CAN para manter simplicidade e previsibilidade.
 *
 * Plataforma: Infineon AURIX TC297B
 * Driver iLLD: IfxMultican (IfxMultican_Can, IfxMultican_Can_MsgObj)
 */

/* ------------------------------------------------------------------ */
/* Includes                                                            */
/* ------------------------------------------------------------------ */

#include "Can.h"
#include "Mcal_Compiler.h"

/* iLLD - MultiCAN */
#include "IfxMultican_Can.h"
#include "IfxMultican_reg.h"

/* ------------------------------------------------------------------ */
/* Indices dos message objects                                         */
/* ------------------------------------------------------------------ */

/** Message Object para recepcao (RX) */
#define CAN_MO_RX_INDEX     0

/** Message Object para transmissao (TX) */
#define CAN_MO_TX_INDEX     1

/** Numero total de message objects alocados */
#define CAN_MO_COUNT        2u

/* ------------------------------------------------------------------ */
/* Estado interno do modulo CAN                                        */
/* ------------------------------------------------------------------ */

/** Handle do modulo MultiCAN */
static IfxMultican_Can         Can_Module;

/** Handle do node CAN 0 */
static IfxMultican_Can_Node    Can_Node;

/** Handle dos message objects */
static IfxMultican_Can_MsgObj  Can_MsgObjRx;
static IfxMultican_Can_MsgObj  Can_MsgObjTx;

/** ID de transmissao configurado (salvo para uso em Can_Write) */
static uint32 Can_TxId;

/** Indica se o modulo foi inicializado */
static boolean Can_Initialized = FALSE;

/* ------------------------------------------------------------------ */
/* Implementacao da API                                                */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo CAN
 *
 * Configura o MultiCAN Node 0 com o baudrate especificado.
 * Aloca e configura os message objects de RX e TX.
 *
 * \param config Ponteiro para estrutura de configuracao com baudrate,
 *               rxId e txId.
 */
void Can_Init(const Can_ConfigType* config)
{
    IfxMultican_Can_Config      moduleConfig;
    IfxMultican_Can_NodeConfig  nodeConfig;
    IfxMultican_Can_MsgObjConfig msgObjConfig;

    if (config == (const Can_ConfigType*)0)
    {
        return;
    }

    /* --- Inicializacao do modulo MultiCAN --- */
    IfxMultican_Can_initModuleConfig(&moduleConfig, &MODULE_CAN);
    IfxMultican_Can_initModule(&Can_Module, &moduleConfig);

    /* --- Configuracao do Node 0 --- */
    IfxMultican_Can_Node_initConfig(&nodeConfig, &Can_Module);
    nodeConfig.nodeId               = IfxMultican_NodeId_0;
    nodeConfig.rxPin                = &IfxMultican_RXD0B_P20_7_IN;   /* CAN0 RX: P20.7 */
    nodeConfig.rxPinMode            = IfxPort_InputMode_pullUp;
    nodeConfig.txPin                = &IfxMultican_TXD0_P20_8_OUT;   /* CAN0 TX: P20.8 */
    nodeConfig.baudrate             = config->baudrate;               /* ex: 500000 bps */

    IfxMultican_Can_Node_init(&Can_Node, &nodeConfig);

    /* Salva o TX ID para uso posterior em Can_Write */
    Can_TxId = config->txId;

    /* --- Configuracao do Message Object de RX (MO0) --- */
    IfxMultican_Can_MsgObj_initConfig(&msgObjConfig, &Can_Node);
    msgObjConfig.msgObjId           = CAN_MO_RX_INDEX;
    msgObjConfig.messageId          = config->rxId;
    msgObjConfig.acceptanceMask     = 0x7FFu;                         /* Mascara exata (11-bit) */
    msgObjConfig.frame              = IfxMultican_Frame_receive;
    msgObjConfig.msgObjCount        = 1u;
    msgObjConfig.dataLengthCode     = IfxMultican_DataLengthCode_8;
    IfxMultican_Can_MsgObj_init(&Can_MsgObjRx, &msgObjConfig);

    /* --- Configuracao do Message Object de TX (MO1) --- */
    IfxMultican_Can_MsgObj_initConfig(&msgObjConfig, &Can_Node);
    msgObjConfig.msgObjId           = CAN_MO_TX_INDEX;
    msgObjConfig.messageId          = config->txId;
    msgObjConfig.acceptanceMask     = 0x7FFu;
    msgObjConfig.frame              = IfxMultican_Frame_transmit;
    msgObjConfig.msgObjCount        = 1u;
    msgObjConfig.dataLengthCode     = IfxMultican_DataLengthCode_8;
    IfxMultican_Can_MsgObj_init(&Can_MsgObjTx, &msgObjConfig);

    Can_Initialized = TRUE;
}

/**
 * \brief Transmite um frame CAN
 *
 * Empacota os bytes em dois uint32 (formato iLLD: data[0] = bytes 0..3,
 * data[1] = bytes 4..7) e envia via message object TX.
 *
 * \param id   CAN ID (11-bit standard)
 * \param data Ponteiro para os dados (max 8 bytes)
 * \param len  Numero de bytes (0..8)
 * \return E_OK se frame enfileirado com sucesso, E_NOT_OK se ocupado
 */
Std_ReturnType Can_Write(uint32 id, const uint8* data, uint8 len)
{
    IfxMultican_Message txMsg;
    uint8               paddedData[8];
    uint8               i;

    if ((Can_Initialized == FALSE) || (data == (const uint8*)0) || (len > 8u))
    {
        return E_NOT_OK;
    }

    /* Verifica se o MO de TX esta livre antes de tentar enviar */
    if (IfxMultican_Can_MsgObj_isTransmitRequested(&Can_MsgObjTx) == TRUE)
    {
        /* Transmissao anterior ainda em andamento */
        return E_NOT_OK;
    }

    /* Preenche buffer com zeros e copia os dados validos */
    for (i = 0u; i < 8u; i++)
    {
        paddedData[i] = (i < len) ? data[i] : 0u;
    }

    /*
     * Empacota 8 bytes em dois uint32 (little-endian):
     *   data[0] = byte0 | (byte1 << 8) | (byte2 << 16) | (byte3 << 24)
     *   data[1] = byte4 | (byte5 << 8) | (byte6 << 16) | (byte7 << 24)
     */
    IfxMultican_Message_init(&txMsg,
                             id,
                             (uint32)paddedData[0]
                             | ((uint32)paddedData[1] << 8u)
                             | ((uint32)paddedData[2] << 16u)
                             | ((uint32)paddedData[3] << 24u),
                             (uint32)paddedData[4]
                             | ((uint32)paddedData[5] << 8u)
                             | ((uint32)paddedData[6] << 16u)
                             | ((uint32)paddedData[7] << 24u),
                             (IfxMultican_DataLengthCode)len);

    if (IfxMultican_Can_MsgObj_sendMessage(&Can_MsgObjTx, &txMsg)
        != IfxMultican_Status_ok)
    {
        return E_NOT_OK;
    }

    return E_OK;
}

/**
 * \brief Verifica se ha frame CAN recebido e le os dados
 *
 * Verifica o flag de novo dado do MO de RX. Se disponivel, desempacota
 * os dois uint32 para o array de bytes.
 *
 * \param id   [out] CAN ID do frame recebido
 * \param data [out] Buffer para os dados (deve ter ao menos 8 bytes)
 * \return E_OK se frame disponivel e lido, E_NOT_OK se buffer vazio
 */
Std_ReturnType Can_Read(uint32* id, uint8* data)
{
    IfxMultican_Message rxMsg;
    uint32              rawData0;
    uint32              rawData1;

    if ((Can_Initialized == FALSE)
        || (id == (uint32*)0)
        || (data == (uint8*)0))
    {
        return E_NOT_OK;
    }

    /* Verifica se ha novo dado no MO de RX */
    if (IfxMultican_Can_MsgObj_isNewDataAvailable(&Can_MsgObjRx) == FALSE)
    {
        return E_NOT_OK;
    }

    /* Le e limpa o flag de novo dado */
    if (IfxMultican_Can_MsgObj_readMessage(&Can_MsgObjRx, &rxMsg)
        != IfxMultican_Status_ok)
    {
        return E_NOT_OK;
    }

    *id = rxMsg.id;

    /*
     * Desempacota os dois uint32 para array de bytes (little-endian):
     *   data[0..3] <- rxMsg.data[0]
     *   data[4..7] <- rxMsg.data[1]
     */
    rawData0 = rxMsg.data[0];
    rawData1 = rxMsg.data[1];

    data[0] = (uint8)(rawData0        & 0xFFu);
    data[1] = (uint8)((rawData0 >> 8u)  & 0xFFu);
    data[2] = (uint8)((rawData0 >> 16u) & 0xFFu);
    data[3] = (uint8)((rawData0 >> 24u) & 0xFFu);
    data[4] = (uint8)(rawData1        & 0xFFu);
    data[5] = (uint8)((rawData1 >> 8u)  & 0xFFu);
    data[6] = (uint8)((rawData1 >> 16u) & 0xFFu);
    data[7] = (uint8)((rawData1 >> 24u) & 0xFFu);

    return E_OK;
}

/**
 * \brief Verifica se a transmissao anterior foi concluida
 *
 * \return TRUE se o MO de TX esta' livre para nova transmissao,
 *         FALSE se ainda ha transmissao pendente
 */
boolean Can_IsTxComplete(void)
{
    if (Can_Initialized == FALSE)
    {
        return TRUE;    /* Nao inicializado = sem transmissao pendente */
    }

    /*
     * TXRQ (Transmit Request) permanece em 1 enquanto o frame
     * aguarda ou esta sendo enviado. Quando o frame e' transmitido
     * com sucesso, TXRQ e' limpo pelo hardware.
     */
    return (IfxMultican_Can_MsgObj_isTransmitRequested(&Can_MsgObjTx) == FALSE)
           ? TRUE : FALSE;
}

/**
 * \brief Handler de polling periodico
 *
 * Deve ser chamado regularmente pela camada de transporte XCP.
 * Nesta implementacao de polling, a funcao e' minima pois TX e RX
 * sao verificados diretamente em Can_Write() e Can_Read().
 *
 * Pode ser expandido para tratamento de erros de bus (BusOff, ErrorPassive)
 * se necessario.
 */
void Can_MainFunction(void)
{
    /*
     * Verificacao de estado de erro do node CAN.
     * Em caso de BusOff, o node precisa ser reinicializado.
     * Por ora, apenas placeholder para expansao futura.
     */
    if (Can_Initialized == FALSE)
    {
        return;
    }

    /* TODO: monitorar IfxMultican_Can_Node_getErrorCounters() para
     *       deteccao de BusOff e recuperacao automatica */
}
