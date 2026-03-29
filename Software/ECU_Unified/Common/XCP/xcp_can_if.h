/**
 * \file xcp_can_if.h
 * \brief Camada de transporte XCP sobre CAN - interface publica
 *
 * Versao portavel: sem dependencia de iLLD (IfxMultican) ou HAL especifico.
 * Toda interacao com hardware e realizada via MCAL Can.h.
 *
 * CAN Node 0 (ou equivalente na plataforma alvo):
 *   Baudrate: 500 kbps
 *   RX ID   : 0x600  (Master -> Slave: comandos XCP)
 *   TX ID   : 0x601  (Slave  -> Master: respostas e frames DAQ)
 *
 * Sequencia de inicializacao:
 *   1. XcpCanIf_Init()  -- inicializa CAN via MCAL
 *   2. XcpInit()        -- inicializa protocolo XCP
 *
 * Uso em tempo de execucao:
 *   - Chamar XcpCanIf_Handler() periodicamente (ex: 10 ms) na task XCP.
 *   - ApplXcpSend() e ApplXcpSendStall() sao chamados internamente pelo XcpBasic.
 */

#ifndef XCP_CAN_IF_H
#define XCP_CAN_IF_H

/**
 * \brief Inicializa o modulo CAN para uso exclusivo do XCP.
 *
 * Configura baudrate (500 kbps), RX ID (0x600) e TX ID (0x601)
 * via Can_Init() do MCAL. Deve ser chamada antes de XcpInit().
 */
void XcpCanIf_Init(void);

/**
 * \brief Handler de polling CAN para o XCP.
 *
 * Verificar periodicamente (recomendado: a cada 10 ms):
 *   - Conclusao de TX -> chama XcpSendCallBack()
 *   - Recepcao de frame RX -> chama XcpCommand()
 */
void XcpCanIf_Handler(void);

#endif /* XCP_CAN_IF_H */
