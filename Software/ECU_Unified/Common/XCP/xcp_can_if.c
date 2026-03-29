/**
 * \file xcp_can_if.c
 * \brief Camada de transporte XCP sobre CAN - versao portavel usando MCAL Can API
 *
 * Substitui a implementacao original baseada em iLLD MultiCAN (IfxMultican_Can_*).
 * Toda interacao com hardware CAN e feita exclusivamente via Can.h (MCAL).
 *
 * Protocolo:
 *   RX CAN ID: 0x600  (Master -> Slave: comandos XCP)
 *   TX CAN ID: 0x601  (Slave -> Master: respostas e DAQ)
 *   Baudrate  : 500 kbps
 *
 * Compativel com:
 *   - Infineon AURIX TC297B  (MCAL Can.h -> IfxMultican iLLD interno)
 *   - STM32H7                (MCAL Can.h -> HAL FDCAN interno)
 *
 * Uso:
 *   1. Chamar XcpCanIf_Init() durante inicializacao do sistema, antes de XcpInit().
 *   2. Chamar XcpCanIf_Handler() periodicamente (ex: a cada 10 ms na task principal).
 *   3. XcpBasic.c chama ApplXcpSend() e ApplXcpSendStall() automaticamente.
 */

#include "xcp_can_if.h"
#include "XcpBasic.h"
#include "Can.h"

/*----------------------------------------------------------------------------*/
/* Defines de CAN ID e baudrate - identicos ao original iLLD                 */

#define XCP_CAN_RX_ID       0x600u   /* Master -> Slave */
#define XCP_CAN_TX_ID       0x601u   /* Slave -> Master */
#define XCP_CAN_BAUDRATE    500000u  /* 500 kbps        */

/*----------------------------------------------------------------------------*/
/* Configuracao estatica do modulo CAN para XCP                               */
/* Passada para Can_Init() - o MCAL trata os detalhes de hardware             */

static const Can_ConfigType xcpCanConfig = {
    XCP_CAN_BAUDRATE,   /* baudrate */
    XCP_CAN_RX_ID,      /* rxId     */
    XCP_CAN_TX_ID       /* txId     */
};

/*----------------------------------------------------------------------------*/
/* Flag de controle de TX                                                      */
/* 1 = transmissao em andamento, 0 = TX livre                                 */

static volatile vuint8 xcpCanTxBusy = 0;

/*----------------------------------------------------------------------------*/
/* Variaveis de diagnostico - acessiveis via XCP ou depurador                 */

volatile vuint32 dbg_CAN_TxFrameCnt  = 0u;  /* Total de frames TX enviados        */
volatile vuint32 dbg_CAN_RxFrameCnt  = 0u;  /* Total de frames RX recebidos       */
volatile vuint32 dbg_CAN_TxStalledCnt = 0u; /* Vezes que ApplXcpSendStall esperou */


/*============================================================================*/
/*  XcpCanIf_Init                                                              */
/*============================================================================*/
/**
 * \brief Inicializa o modulo CAN para uso pelo XCP.
 *
 * Deve ser chamada uma unica vez durante a inicializacao do sistema
 * (antes de XcpInit()).
 *
 * Internamente invoca Can_Init() do MCAL, que configura o hardware
 * especifico da plataforma (MultiCAN no TC297B, FDCAN no STM32H7).
 */
void XcpCanIf_Init(void)
{
    Can_Init(&xcpCanConfig);
    xcpCanTxBusy = 0u;
}


/*============================================================================*/
/*  XcpCanIf_Handler                                                           */
/*============================================================================*/
/**
 * \brief Polling de CAN para o XCP: verifica TX e processa RX.
 *
 * Deve ser chamada periodicamente (ex: a cada 10 ms na task XCP).
 *
 * Fluxo:
 *  1. Se TX estava ocupado, verifica conclusao via Can_IsTxComplete().
 *     - Quando concluido: libera flag e notifica XcpBasic via XcpSendCallBack().
 *  2. Tenta ler frame RX via Can_Read().
 *     - Se houver dado novo: passa buffer diretamente para XcpCommand().
 *       (Can.h ja entrega bytes na ordem correta - sem conversao uint32 necessaria)
 */
void XcpCanIf_Handler(void)
{
    uint32 rxId;
    uint8  rxData[8];

    /* --- Verificacao de conclusao de TX ---------------------------------- */
    if (xcpCanTxBusy != 0u)
    {
        if (Can_IsTxComplete() == TRUE)
        {
            xcpCanTxBusy = 0u;
            XcpSendCallBack();
        }
    }

    /* --- Recepcao de comando XCP ----------------------------------------- */
    /* Can_Read() retorna E_OK apenas quando ha novo frame disponivel no RX ID
     * configurado (0x600). O buffer rxData contem os bytes na ordem de recepcao,
     * pronto para XcpCommand() - nao e necessario desempacotar uint32 como no
     * codigo iLLD original. */
    if (Can_Read(&rxId, rxData) == E_OK)
    {
        dbg_CAN_RxFrameCnt++;
        /* rxId nao e verificado aqui: Can.h so entrega frames do rxId configurado */
        XcpCommand((const vuint32*)(void*)rxData);
    }
}


/*============================================================================*/
/*  ApplXcpSend                                                                */
/*============================================================================*/
/**
 * \brief Callback chamado pelo XcpBasic para transmitir uma resposta ou DAQ.
 *
 * \param len  Numero de bytes validos em msg (1..8)
 * \param msg  Ponteiro para os dados a transmitir
 *
 * A conversao de byte array para 2x uint32 (necessaria no codigo iLLD original)
 * foi eliminada: Can_Write() aceita diretamente o ponteiro uint8[].
 *
 * Nota: Can_Write() enfileira o frame; a conclusao e detectada em
 * XcpCanIf_Handler() via Can_IsTxComplete().
 */
void ApplXcpSend(vuint8 len, MEMORY_ROM BYTEPTR msg)
{
    xcpCanTxBusy = 1u;
    Can_Write(XCP_CAN_TX_ID, (const uint8*)msg, (uint8)len);
    dbg_CAN_TxFrameCnt++;
}


/*============================================================================*/
/*  ApplXcpSendStall                                                           */
/*============================================================================*/
/**
 * \brief Aguarda conclusao da transmissao TX atual (bloqueante).
 *
 * Chamado pelo XcpBasic quando precisa garantir que o buffer de TX foi
 * liberado antes de enfileirar o proximo pacote (ex: modo SEND_QUEUE).
 *
 * \return 1 sempre (indicando que TX foi liberado com sucesso)
 */
vuint8 ApplXcpSendStall(void)
{
    while (xcpCanTxBusy != 0u)
    {
        if (Can_IsTxComplete() == TRUE)
        {
            xcpCanTxBusy = 0u;
            XcpSendCallBack();
        }
        dbg_CAN_TxStalledCnt++;
    }
    return 1u;
}
