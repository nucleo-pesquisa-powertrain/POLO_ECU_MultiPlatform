/**
 * \file xcp_appl.c
 * \brief Callbacks de aplicacao XCP - implementacao minima
 *
 * Apenas ApplXcpGetPointer() e necessario para este projeto.
 * Ambas as plataformas alvo (TC297B e STM32H7) usam modelo de
 * memoria plano de 32 bits, portanto o endereco XCP e diretamente
 * um ponteiro C valido.
 *
 * Nenhuma troca de pagina de calibracao e necessaria (XCP_DISABLE_CALIBRATION_PAGE).
 */

#include "XcpBasic.h"


/**
 * \brief Converte o endereco XCP (ext:addr) para ponteiro C nativo.
 *
 * Com modelo de memoria plano (TC297B e STM32H7), o campo addr
 * ja e um endereco fisico valido; addr_ext nao e utilizado.
 *
 * \param addr_ext  Extensao de endereco XCP (ignorada - modelo plano)
 * \param addr      Endereco XCP de 32 bits
 * \return          Ponteiro de byte equivalente
 */
MTABYTEPTR ApplXcpGetPointer(vuint8 addr_ext, vuint32 addr)
{
    (void)addr_ext;  /* sem segmentacao de memoria nas plataformas alvo */
    return (MTABYTEPTR)addr;
}
