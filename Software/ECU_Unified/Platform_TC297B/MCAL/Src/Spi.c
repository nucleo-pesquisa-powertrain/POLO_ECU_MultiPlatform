/**
 * \file Spi.c
 * \brief Implementacao MCAL SPI para TC297B - delega para SPI_CPU.c (HardwareAdp)
 */

#include "Spi.h"
#include "SPI_CPU.h"

void Spi_Init(void)
{
    initQSPI2Master();
    initQSPI2MasterChannel();
    initQSPI2MasterBuffers();
}

Std_ReturnType Spi_Transmit(const uint8* data, uint16 len)
{
    /* SPI_SendBuffer espera unsigned short int como dado e unsigned char como tamanho.
     * Para compatibilidade, empacota os 2 primeiros bytes como uint16. */
    uint16 word = 0u;
    if (len >= 2u)
    {
        word = ((uint16)data[0] << 8u) | (uint16)data[1];
    }
    else if (len == 1u)
    {
        word = ((uint16)data[0] << 8u);
    }

    SPI_SendBuffer(word, (unsigned char)len);
    return E_OK;
}

Std_ReturnType Spi_TransmitReceive(const uint8* txData, uint8* rxData, uint16 len)
{
    (void)rxData; /* RX nao implementado neste wrapper */
    return Spi_Transmit(txData, len);
}

boolean Spi_IsBusy(void)
{
    return (IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == IfxQspi_Status_busy) ? TRUE : FALSE;
}
