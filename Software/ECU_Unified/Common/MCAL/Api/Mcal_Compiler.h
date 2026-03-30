/**
 * \file Mcal_Compiler.h
 * \brief Compiler abstraction for critical sections and intrinsics
 *
 * Provides platform-independent macros for interrupt control.
 * Each platform defines the actual implementation.
 */
#ifndef MCAL_COMPILER_H
#define MCAL_COMPILER_H

#if defined(PLATFORM_TC297B)
    /* TriCore TASKING compiler intrinsics */
    #define Mcal_DisableAllInterrupts()   __disable()
    #define Mcal_EnableAllInterrupts()    __enable()

#elif defined(PLATFORM_STM32H7)
    /* ARM Cortex-M CMSIS intrinsics */
    #include "cmsis_gcc.h"
    #define Mcal_DisableAllInterrupts()   __disable_irq()
    #define Mcal_EnableAllInterrupts()    __enable_irq()

#else
    #error "Platform not defined! Define PLATFORM_TC297B or PLATFORM_STM32H7"
#endif

#endif /* MCAL_COMPILER_H */
