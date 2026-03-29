/**
 * \file Can.h
 * \brief MCAL CAN Communication Abstraction
 *
 * API para comunicacao CAN, usada primariamente pelo XCP.
 * TC297B: MultiCAN (iLLD)
 * STM32H7: FDCAN (HAL)
 */
#ifndef CAN_H
#define CAN_H

#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Tipos                                                               */
/* ------------------------------------------------------------------ */

/** Configuracao de inicializacao do CAN */
typedef struct {
    uint32 baudrate;    /**< Baudrate em bps (ex: 500000) */
    uint32 rxId;        /**< CAN ID para recepcao (ex: 0x600) */
    uint32 txId;        /**< CAN ID para transmissao (ex: 0x601) */
} Can_ConfigType;

/* ------------------------------------------------------------------ */
/* API                                                                 */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo CAN
 * \param config Ponteiro para configuracao
 */
void Can_Init(const Can_ConfigType* config);

/**
 * \brief Transmite um frame CAN
 * \param id   CAN ID (11-bit standard)
 * \param data Ponteiro para dados (max 8 bytes)
 * \param len  Numero de bytes (0..8)
 * \return E_OK se frame enfileirado, E_NOT_OK se busy
 */
Std_ReturnType Can_Write(uint32 id, const uint8* data, uint8 len);

/**
 * \brief Verifica se ha frame CAN recebido e le os dados
 * \param id   [out] CAN ID do frame recebido
 * \param data [out] Buffer para dados (min 8 bytes)
 * \return E_OK se frame disponivel, E_NOT_OK se vazio
 */
Std_ReturnType Can_Read(uint32* id, uint8* data);

/**
 * \brief Verifica se a transmissao anterior foi concluida
 * \return TRUE se TX livre, FALSE se ainda transmitindo
 */
boolean Can_IsTxComplete(void);

/**
 * \brief Handler de polling - deve ser chamado periodicamente
 *
 * Verifica TX completion e processa RX.
 * Usado pelo XCP transport layer.
 */
void Can_MainFunction(void);

#endif /* CAN_H */
