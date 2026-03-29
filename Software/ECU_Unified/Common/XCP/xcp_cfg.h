/**
 * \file xcp_cfg.h
 * \brief Configuracao XCP portavel - MCAL (sem dependencia de iLLD ou HAL)
 *
 * Adaptado de: Vector XCP Basic Driver v1.30.05 TriCore sample.
 * Versao original: configurada para TC297B com IfxStm e intrinsics TASKING.
 * Esta versao: portavel via MCAL Icu.h e Mcal_Compiler.h.
 *
 * Plataformas suportadas (selecionar via define de compilacao):
 *   -DPLATFORM_TC297B   (Infineon AURIX TC297B, TASKING compiler)
 *   -DPLATFORM_STM32H7  (STM32H7xx, ARM GCC / IAR)
 *
 * Mudancas em relacao ao original:
 *   - REMOVIDO: #include "IfxStm.h" / "IfxStm_reg.h"
 *   - REMOVIDO: IfxStm_getLower(&MODULE_STM0) para timestamp
 *   - REMOVIDO: intrinsics TASKING __disable()/__enable() diretos
 *   - ADICIONADO: #include "Icu.h" -> Icu_GetTimestamp_us()
 *   - ADICIONADO: #include "Mcal_Compiler.h" -> Mcal_DisableAllInterrupts/Enable
 *   - Timestamp: 1 tick = 1 us (MCAL normaliza a frequencia internamente)
 *     Antes: 100 ticks/us (STM @ 100 MHz em escala bruta)
 */

#if defined ( __XCP_CFG_H__ )
#else
#define __XCP_CFG_H__


/*----------------------------------------------------------------------------*/
/* Includes MCAL portaveis                                                    */

#include <string.h>
#include "Icu.h"           /* Icu_GetTimestamp_us() - substitui IfxStm        */
#include "Mcal_Compiler.h" /* Mcal_DisableAllInterrupts/Enable - substitui
                            * intrinsics TASKING __disable()/__enable()        */


/*----------------------------------------------------------------------------*/
/* Test / Assert                                                               */

/* Descomentar para habilitar mensagens de diagnostico XCP via XCP_PRINT      */
/* #define XCP_ENABLE_TESTMODE */
#ifdef XCP_ENABLE_TESTMODE
  #define XCP_ASSERT(x) /* vazio */
#else
  #define XCP_ASSERT(x)
#endif


/*----------------------------------------------------------------------------*/
/* Tipos de dados XCP (vuint8/16/32)                                          */
/* Padrao Vector - mantidos identicos ao original                             */

typedef unsigned char  vuint8;
typedef signed char    vsint8;

typedef unsigned short vuint16;
typedef signed short   vsint16;

typedef unsigned long  vuint32;
typedef signed long    vsint32;


/*----------------------------------------------------------------------------*/
/* Identificacao do slave XCP                                                 */

#define kXcpStationIdLength 10       /* strlen("ECU_TC297B") */
#define kXcpStationIdString "ECU_TC297B"

#if defined ( kXcpStationIdLength )
  extern const vuint8 kXcpStationId[];
#endif


/*----------------------------------------------------------------------------*/
/* Parametros de protocolo XCP                                                */

/* Rotina de copia */
#define xcpMemCpy memcpy

/* Ordem de bytes - ambas as plataformas alvo sao little-endian               */
#define C_CPUTYPE_LITTLEENDIAN

/* Tamanho maximo das mensagens CTO e DTO (8 bytes para CAN classico)         */
#define kXcpMaxCTO     8
#define kXcpMaxDTO     8

#define XCP_DISABLE_PARAMETER_CHECK

/* COMM_MODE_INFO habilitado (informacao de modo de comunicacao)              */
#define XCP_ENABLE_COMM_MODE_INFO

/* Seed & Key desabilitado (sem protecao)                                     */
/* #define XCP_ENABLE_SEED_KEY */

/* Comandos de usuario desabilitados                                          */
/* #define XCP_ENABLE_USER_COMMAND */

/* Transmissao de mensagens de evento habilitada                              */
#define XCP_ENABLE_SEND_EVENT

/* Programacao FLASH desabilitada                                             */
#define XCP_DISABLE_PROGRAM
/* #define XCP_ENABLE_BOOTLOADER_DOWNLOAD */

/* Service Request Text desabilitado                                          */
#define XCP_DISABLE_SERV_TEXT
#define XCP_DISABLE_SERV_TEXT_PUTCHAR
#define XCP_DISABLE_SERV_TEXT_PRINT


/*----------------------------------------------------------------------------*/
/* Parametros de Calibracao XCP                                               */

#define XCP_ENABLE_CALIBRATION

#define XCP_ENABLE_SHORT_DOWNLOAD
#define XCP_ENABLE_SHORT_UPLOAD

/* Transferencia em bloco habilitada                                          */
#define XCP_ENABLE_BLOCK_UPLOAD
#define XCP_ENABLE_BLOCK_DOWNLOAD

/* Checksum de memoria habilitado (calculado em XcpBackground())              */
#define XCP_ENABLE_CHECKSUM
#define kXcpChecksumMethod XCP_CHECKSUM_TYPE_ADD14

/* Troca de paginas de calibracao desabilitada (modelo de memoria plano)      */
/* #define XCP_ENABLE_CALIBRATION_PAGE */
#define XCP_DISABLE_CALIBRATION_PAGE
#define XCP_DISABLE_SEGMENT_INFO
#define XCP_DISABLE_PAGE_INFO
#define XCP_DISABLE_PAGE_FREEZE


/*----------------------------------------------------------------------------*/
/* Parametros de Estimulacao de Dados XCP (STIM)                              */

/* #define XCP_ENABLE_STIM */


/*----------------------------------------------------------------------------*/
/* Parametros de Aquisicao de Dados XCP (DAQ)                                 */

#define XCP_ENABLE_DAQ

#define XCP_ENABLE_DAQ_PRESCALER
#define XCP_ENABLE_DAQ_OVERRUN_INDICATION
#define XCP_ENABLE_DAQ_PROCESSOR_INFO
#define XCP_ENABLE_DAQ_RESOLUTION_INFO

#define XCP_ENABLE_WRITE_DAQ_MULTIPLE

/* Memoria reservada para DAQ (2 KB)                                          */
#define kXcpDaqMemSize (1024*2)
#define XCP_ENABLE_SEND_QUEUE
#define XCP_DISABLE_SEND_BUFFER

/* Timestamp DAQ ------------------------------------------------------------ */
#define XCP_ENABLE_DAQ_TIMESTAMP
#ifdef XCP_ENABLE_DAQ_TIMESTAMP
  /* Tamanho: 4 bytes (32 bits) */
  #define kXcpDaqTimestampSize  4

  /* Unidade base: 1 us (DAQ_TIMESTAMP_UNIT_1US)                              */
  #define kXcpDaqTimestampUnit  DAQ_TIMESTAMP_UNIT_1US

  /* Icu_GetTimestamp_us() ja retorna microsegundos normalizados.
   * => 1 tick = 1 us => kXcpDaqTimestampTicksPerUnit = 1
   *
   * Comparacao com original (IfxStm @ 100 MHz):
   *   Original : 100 ticks/us (valor bruto do STM0)
   *   Este     : 1  tick/us   (MCAL Icu normaliza internamente)               */
  #define kXcpDaqTimestampTicksPerUnit  1

  /* Leitura do timestamp via MCAL ICU - portavel entre TC297B e STM32H7      */
  #define ApplXcpGetTimestamp()     (XcpDaqTimestampType)Icu_GetTimestamp_us()
  #define ApplXcpDaqGetTimestamp()  (XcpDaqTimestampType)Icu_GetTimestamp_us()
#endif

/* DAQ header ODT desabilitado (DAQ padrao)                                   */
#define XCP_DISABLE_DAQ_HDR_ODT_DAQ

/* Event info desabilitado (nao necessario para DAQ basico)                   */
/* #define XCP_ENABLE_DAQ_EVENT_INFO */


/*----------------------------------------------------------------------------*/
/* Protecao de interrupcoes                                                    */
/* XcpSendCallBack() pode ser chamado de XcpCanIf_Handler() em contexto de
 * task. Como tudo roda na mesma task, nao ha preempcao real. Os macros abaixo
 * usam o MCAL Mcal_Compiler.h para portabilidade entre plataformas:
 *   TC297B  : Mcal_DisableAllInterrupts() -> __disable() (TASKING intrinsic)
 *   STM32H7 : Mcal_DisableAllInterrupts() -> __disable_irq() (CMSIS)         */

#define XcpInterruptDisable()          Mcal_DisableAllInterrupts()
#define XcpInterruptEnable()           Mcal_EnableAllInterrupts()
#define ApplXcpInterruptDisable()      Mcal_DisableAllInterrupts()
#define ApplXcpInterruptEnable()       Mcal_EnableAllInterrupts()


#endif /* __XCP_CFG_H__ */
