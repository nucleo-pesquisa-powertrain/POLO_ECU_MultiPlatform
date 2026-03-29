 /*
 * File: Management.h
 *
 * Code generated for Simulink model 'Management'.
 *
 * Model version                  : 1.45
 * Simulink Coder version         : 8.10 (R2016a) 10-Feb-2016
 * C/C++ source code generated on : Sat Dec 09 15:45:37 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_Management_h_
#define RTW_HEADER_Management_h_
#ifndef Management_COMMON_INCLUDES_
# define Management_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* Management_COMMON_INCLUDES_ */

#include "Management_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((rtm)->errorStatus = (val))
#endif

#define Management_M                   (rtM_Management)

/* Block signals and states (auto storage) for system '<Root>' */
typedef struct {
  uint16_T Delay_DSTATE;               /* '<S3>/Delay' */
  uint16_T Delay_DSTATE_m;             /* '<S1>/Delay' */
  uint16_T Delay2_DSTATE;              /* '<S3>/Delay2' */
  uint16_T Delay1_DSTATE;              /* '<S3>/Delay1' */
} D_Work_Management;

/* Real-time Model Data Structure */
struct tag_RTM_Management {
  const char_T *errorStatus;
};

/* Block signals and states (auto storage) */
extern D_Work_Management rtDWork_Management;

/*
 * Exported Global Parameters
 *
 * Note: Exported global parameters are tunable parameters with an exported
 * global storage class designation.  Code generation will declare the memory for
 * these parameters and exports their symbols.
 *
 */
extern uint16_T C_MNGT_DisturbEngSpeedThreshold;/* Variable: C_MNGT_DisturbEngSpeedThreshold
                                                 * Referenced by: '<S1>/Constant5'
                                                 */

/* Model entry point functions */
extern void Management_initialize(void);
extern void Management_step(void);

/* Real-time Model object */
extern RT_MODEL_Management *const rtM_Management;

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
 * '<Root>' : 'Management'
 * '<S1>'   : 'Management/Command Management'
 * '<S2>'   : 'Management/Inputs Control'
 * '<S3>'   : 'Management/Command Management/Latch'
 */
#endif                                 /* RTW_HEADER_Management_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
