#ifndef PLATFORM_DEFS_H
#define PLATFORM_DEFS_H

/**
 * @file    platform_defs.h
 *
 * @author  ETAS GmbH
 *
 * @date    2017.11.10 10:36:34
 *
 * @brief   Header file containing header platform specific definitions
 *
 * @version 2.0
 *
 **/
#include "esdl_types.h"
#include "esdl_deltaTimeDefs.h"

/**
 * There are two example implementations available for delta time calculation between task activations.
 */
#define ASCET_DELTA_T_STATIC  1
#define ASCET_DELTA_T_DYNAMIC 2

#if (ASCET_DELTA_T_SUPPORT == ASCET_DELTA_T_STATIC)
    #if !defined(__TASKING__)
    #warning THESE CODE IS FOR EXAMPLE ONLY! DO NOT USE THIS CODE IN ECUS RUNNING IN ANY VEHICLE WITHOUT REVISING IT AGAINST YOUR REQUIREMENTS!
    #endif

    #define DEF_GLB_DT_MEASURE                  extern ASCET_DELTA_T_SCALED_TYPE ASD_DT_SCALED

    #define DEF_TASK_DT_MEASURE                 ASCET_DELTA_T_SCALED_TYPE ASD_dTSaved = ASD_DT_SCALED

    /* Note: do {...}while(0) idiom is used here for MISRA compliance */
    #define PRE_TASK_DT_MEASURE(TASK_PERIOD)    do { \
                                                    DisableAllInterrupts(); \
                                                    ASCET_SET_MODEL_DT(TASK_PERIOD); \
                                                    EnableAllInterrupts(); \
                                                } while(0)
    /* Note: do {...}while(0) idiom is used here for MISRA compliance */
    #define POST_TASK_DT_MEASURE                 do { \
                                                    DisableAllInterrupts(); \
                                                    ASD_DT_SCALED = ASD_dTSaved; \
                                                    EnableAllInterrupts(); \
                                                } while (0)
#endif /* ASCET_DELTA_T_SUPPORT == ASCET_DELTA_T_STATIC */

#if (ASCET_DELTA_T_SUPPORT == ASCET_DELTA_T_DYNAMIC)
    #if !defined(__TASKING__)
    #warning THESE CODE IS FOR EXAMPLE ONLY! DO NOT USE THIS CODE IN ECUS RUNNING IN ANY VEHICLE WITHOUT REVISING IT AGAINST YOUR REQUIREMENTS!
    #endif

    #define DEF_GLB_DT_MEASURE                  extern ASCET_DELTA_T_SCALED_TYPE ASD_DT_SCALED

    #define DEF_TASK_DT_MEASURE                 static TickType ASD_startTime = 0; \
                                                TickType ASD_curTime; \
                                                TickType ASD_deltaTicks; \
                                                ASCET_DELTA_T_SCALED_TYPE ASD_dTSaved

    /* Note: do {...}while(0) idiom is used here for MISRA compliance */
    #define PRE_TASK_DT_MEASURE(TASK_PERIOD)    do { \
                                                    ASD_dTSaved = ASD_DT_SCALED; \
                                                    ASD_curTime = GetStopwatch(); \
                                                    ASD_deltaTicks = (TickType)((ASD_curTime > ASD_startTime) ? \
                                                         (ASD_curTime - ASD_startTime) \
                                                        :(OSMAXALLOWEDVALUE-ASD_startTime+ASD_curTime)); \
                                                    DisableAllInterrupts(); \
                                                    ASCET_SET_MODEL_DT(OSTICKDURATION * ASD_deltaTicks/1.0e9); \
                                                    EnableAllInterrupts(); \
                                                    ASD_startTime = ASD_curTime; \
                                                } while(0)

    /* Note: do {...}while(0) idiom is used here for MISRA compliance */
    #define POST_TASK_DT_MEASURE                do { \
                                                    DisableAllInterrupts(); \
                                                    ASD_DT_SCALED = ASD_dTSaved; \
                                                    EnableAllInterrupts(); \
                                                } while(0)
#endif /* ASCET_DELTA_T_SUPPORT == ASCET_DELTA_T_DYNAMIC */

#endif /* PLATFORM_DEFS_H */
