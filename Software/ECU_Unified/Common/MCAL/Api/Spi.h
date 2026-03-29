/**
 * \file Spi.h
 * \brief MCAL SPI Communication Abstraction
 *
 * API para comunicacao SPI, usada para controle do MC33810.
 * TC297B: QSPI (iLLD)
 * STM32H7: SPI (HAL)
 */
#ifndef SPI_H
#define SPI_H

#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* API                                                                 */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo SPI (master mode)
 */
void Spi_Init(void);

/**
 * \brief Transmite dados via SPI
 * \param data Ponteiro para buffer de dados
 * \param len  Numero de bytes a transmitir
 * \return E_OK se sucesso, E_NOT_OK se erro
 */
Std_ReturnType Spi_Transmit(const uint8* data, uint16 len);

/**
 * \brief Transmite e recebe dados via SPI (full-duplex)
 * \param txData Ponteiro para buffer TX
 * \param rxData Ponteiro para buffer RX
 * \param len    Numero de bytes
 * \return E_OK se sucesso, E_NOT_OK se erro
 */
Std_ReturnType Spi_TransmitReceive(const uint8* txData, uint8* rxData, uint16 len);

/**
 * \brief Verifica se o SPI esta ocupado
 * \return TRUE se ocupado, FALSE se livre
 */
boolean Spi_IsBusy(void);

#endif /* SPI_H */
