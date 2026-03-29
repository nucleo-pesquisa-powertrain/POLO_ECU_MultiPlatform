/**
 * \file xcp_par.c
 * \brief XCP Parameters for ECU_TC297B
 *
 * Defines the XCP station ID string.
 */

#include "XcpBasic.h"

/* XCP Station ID */
#if defined ( kXcpStationIdLength )
V_MEMROM0 MEMORY_ROM vuint8 kXcpStationId[kXcpStationIdLength] = kXcpStationIdString;
#endif
