#ifndef ESDL_MUTEX_H
#define ESDL_MUTEX_H

/**
 * @file    esdl_mutex.h
 *
 * @author  ETAS GmbH
 *
 * @date    2014.09.25 10:36:34
 *
 * @brief   Header file containing header access to the underlying OS
 *          for ensuring mutual exclusion when caching messages
 *
 * @version 2.0
 *
 * @copyright ETAS GmbH, Stuttgart, Germany. All rights reserved
 **/

/* Provide all generated code global access to user provided header file(s) */

#include "esdl_usercfg.h"

#ifdef ASCET_PLATFORM_BUILD
#include "platform_defs.h"
#endif

/* Provide access to OS API for concurrency Control Macros */

#if defined(OSENV_RTAOS40)||defined(OSENV_RTAOS30)
    /* Use AUTOSAR OS */
    #include "Os.h"
#elif defined(OSENV_RTAOSEK)
    /* Use the RTA-OSEK version of OSEK */
    #include "osek.h"
#elif defined(OSENV_UNSUPPORTED)
    /* Use an OS not known to ESDL */
#elif defined(SINGLE_THREADED)
    #define SuspendAllInterrupts() do {} while (0)
    #define ResumeAllInterrupts() do {} while (0)
    #define EnableAllInterrupts() do {} while (0)
    #define DisableAllInterrupts() do {} while (0)
#else
    /* Use Define API */
    #if !defined(__TASKING__)
    #warning Potentially unsafe message caching. Race conditions may occur in multi-threaded environments. See esdl_mutex.h for options
    #endif
    #define SuspendAllInterrupts() do {} while (0)
    #define ResumeAllInterrupts() do {} while (0)
    #define EnableAllInterrupts() do {} while (0)
    #define DisableAllInterrupts() do {} while (0)
#endif

/* Message copy caching needs a memcpy */
#if !defined(ESDL_USER_MEMCPY)||!defined(RTE_LIBC_MEMCPY)
    #include "string.h"
#endif

#endif /* ESDL_H */
