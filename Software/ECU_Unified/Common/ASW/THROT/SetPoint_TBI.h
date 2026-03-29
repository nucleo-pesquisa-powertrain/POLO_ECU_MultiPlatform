/*
 * File: SetPoint_TBI.h
 *
 * Code generated for Simulink model 'SetPoint_TBI'.
 *
 * Model version                  : 1.39
 * Simulink Coder version         : 8.10 (R2016a) 10-Feb-2016
 * C/C++ source code generated on : Thu Feb 22 18:59:38 2024
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_SetPoint_TBI_h_
#define RTW_HEADER_SetPoint_TBI_h_
#ifndef SetPoint_TBI_COMMON_INCLUDES_
# define SetPoint_TBI_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* SetPoint_TBI_COMMON_INCLUDES_ */

/* Includes for objects with custom storage classes. */
#include "tbi_setpoint_interface.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((rtm)->errorStatus = (val))
#endif

#define SetPoint_TBI_M                 (SPTBIrtM)

/* Forward declaration for rtModel */
typedef struct SPTBItag_RTM SPTBIRT_MODEL;

/* Block signals and states (auto storage) for system '<Root>' */
typedef struct {
  real_T SPTBIref_vb;                  /* '<S6>/Idle_Speed' */
  real_T SPTBIcont_partida;            /* '<S6>/Idle_Speed' */
  uint8_T SPTBIis_active_c1_SetPoint_TBI;/* '<S6>/Idle_Speed' */
  uint8_T SPTBIis_c1_SetPoint_TBI;     /* '<S6>/Idle_Speed' */
} SPTBIDW;

/* Real-time Model Data Structure */
struct SPTBItag_RTM {
  const char_T *errorStatus;
};

/* Block signals and states (auto storage) */
extern SPTBIDW SPTBIrtDW;

/* Model entry point functions */
extern void SetPoint_TBI_initialize(void);
extern void SetPoint_TBI_step(void);

/* Real-time Model object */
extern SPTBIRT_MODEL *const SPTBIrtM;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S10>/Data Type Duplicate' : Unused code path elimination
 * Block '<S10>/Data Type Propagation' : Unused code path elimination
 * Block '<S1>/Switch1' : Eliminated due to constant selection input
 * Block '<S5>/Data Type Conversion1' : Eliminate redundant data type conversion
 * Block '<S1>/Pedal' : Unused code path elimination
 */

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'SetPoint_TBI'
 * '<S1>'   : 'SetPoint_TBI/Inputs_Control'
 * '<S2>'   : 'SetPoint_TBI/SetPoint_TBI'
 * '<S3>'   : 'SetPoint_TBI/Test&Simulation'
 * '<S4>'   : 'SetPoint_TBI/SetPoint_TBI/Calc_deg_SetPoint'
 * '<S5>'   : 'SetPoint_TBI/SetPoint_TBI/DoubleConverter '
 * '<S6>'   : 'SetPoint_TBI/SetPoint_TBI/IdleSpeed'
 * '<S7>'   : 'SetPoint_TBI/SetPoint_TBI/Load'
 * '<S8>'   : 'SetPoint_TBI/SetPoint_TBI/uint16Converter'
 * '<S9>'   : 'SetPoint_TBI/SetPoint_TBI/IdleSpeed/Idle_Speed'
 * '<S10>'  : 'SetPoint_TBI/SetPoint_TBI/Load/Saturation Dynamic2'
 * '<S11>'  : 'SetPoint_TBI/Test&Simulation/Saturation Dynamic2'
 */
#endif                                 /* RTW_HEADER_SetPoint_TBI_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
