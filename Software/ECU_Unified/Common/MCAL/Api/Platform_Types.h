/**
 * \file Platform_Types.h
 * \brief AUTOSAR Platform Types - Portable across TriCore and ARM Cortex-M
 *
 * Provides standardized type definitions for both Infineon AURIX TC297B
 * and STM32H7 platforms.
 */
#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#include <stdint.h>

/* CPU Type */
#define CPU_TYPE_8      8u
#define CPU_TYPE_16     16u
#define CPU_TYPE_32     32u
#define CPU_TYPE        CPU_TYPE_32

/* Byte Order */
#define HIGH_BYTE_FIRST 0u
#define LOW_BYTE_FIRST  1u
#define CPU_BYTE_ORDER  LOW_BYTE_FIRST  /* Both TriCore and Cortex-M are little-endian */

/* Bit Order */
#define MSB_FIRST       0u
#define LSB_FIRST       1u
#define CPU_BIT_ORDER   LSB_FIRST

/* Boolean */
#ifndef TRUE
#define TRUE            1u
#endif
#ifndef FALSE
#define FALSE           0u
#endif

/* AUTOSAR Integer Types (using stdint for portability) */
typedef uint8_t         uint8;
typedef uint16_t        uint16;
typedef uint32_t        uint32;
typedef uint64_t        uint64;

typedef int8_t          sint8;
typedef int16_t         sint16;
typedef int32_t         sint32;
typedef int64_t         sint64;

typedef uint8_t         boolean;

typedef float           float32;
typedef double          float64;

/* Standard Return Type */
typedef uint8           Std_ReturnType;
#define E_OK            0u
#define E_NOT_OK        1u

#endif /* PLATFORM_TYPES_H */
