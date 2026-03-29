/*
 * File: TBI_PositionControl.h
 *
 * Code generated for Simulink model 'TBI_PositionControl'.
 *
 * Model version                  : 1.85
 * Simulink Coder version         : 8.10 (R2016a) 10-Feb-2016
 * C/C++ source code generated on : Wed Apr 09 00:19:13 2025
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Custom Processor->Custom
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_TBI_PositionControl_h_
#define RTW_HEADER_TBI_PositionControl_h_
#include <math.h>
#ifndef TBI_PositionControl_COMMON_INCLUDES_
# define TBI_PositionControl_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* TBI_PositionControl_COMMON_INCLUDES_ */

/* Includes for objects with custom storage classes. */
#include "tbi_interface.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((rtm)->errorStatus = (val))
#endif

#define TBI_PositionControl_M          (rtM)

/* Forward declaration for rtModel */
typedef struct tag_RTM RT_MODEL;

/* Block signals and states (auto storage) for system '<Root>' */
typedef struct {
  real_T Switch2;                      /* '<S26>/Switch2' */
  real_T Delay_DSTATE;                 /* '<S19>/Delay' */
  real_T Delay1_DSTATE;                /* '<S18>/Delay1' */
  real_T Delay_DSTATE_c;               /* '<S18>/Delay' */
  struct {
    void *AQHandles;
  } HiddenToAsyncQueue_InsertedFor_;   /* synthesized block */

  uint16_T Sensor;                     /* '<Root>/Data Store Read1' */
  boolean_T Delay2_DSTATE;             /* '<S19>/Delay2' */
  boolean_T Delay1_DSTATE_k;           /* '<S19>/Delay1' */
} DW;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T *errorStatus;
};

/* Block signals and states (auto storage) */
extern DW rtDW;

/*
 * Exported Global Parameters
 *
 * Note: Exported global parameters are tunable parameters with an exported
 * global storage class designation.  Code generation will declare the memory for
 * these parameters and exports their symbols.
 *
 */
extern real_T C_TBI_ControlDown_Ki;    /* Variable: C_TBI_ControlDown_Ki
                                        * Referenced by: '<S17>/Constant3'
                                        */
extern real_T C_TBI_ControlDown_Kp;    /* Variable: C_TBI_ControlDown_Kp
                                        * Referenced by: '<S17>/Constant2'
                                        */
extern real_T C_TBI_ControlUp_Ki;      /* Variable: C_TBI_ControlUp_Ki
                                        * Referenced by: '<S18>/Constant3'
                                        */
extern real_T C_TBI_ControlUp_Kp;      /* Variable: C_TBI_ControlUp_Kp
                                        * Referenced by: '<S18>/Kp'
                                        */
extern real_T C_TBI_DeadBandError;     /* Variable: C_TBI_DeadBandError
                                        * Referenced by: '<S18>/Constant6'
                                        */
extern real_T C_TBI_DitherAmplitude[10];/* Variable: C_TBI_DitherAmplitude
                                         * Referenced by: '<S19>/1-D Lookup Table'
                                         */
extern real_T C_TBI_DitherEnable;      /* Variable: C_TBI_DitherEnable
                                        * Referenced by: '<S2>/Constant11'
                                        */
extern real_T C_TBI_DitherFreq;        /* Variable: C_TBI_DitherFreq
                                        * Referenced by: '<S19>/Constant3'
                                        */
extern real_T C_TBI_DitherPositionBrkpts[10];/* Variable: C_TBI_DitherPositionBrkpts
                                              * Referenced by: '<S19>/1-D Lookup Table'
                                              */
extern real_T C_TBI_FFDownKs;          /* Variable: C_TBI_FFDownKs
                                        * Referenced by: '<S20>/Constant4'
                                        */
extern real_T C_TBI_FFDownTPC;         /* Variable: C_TBI_FFDownTPC
                                        * Referenced by: '<S20>/Constant2'
                                        */
extern real_T C_TBI_FFEnable;          /* Variable: C_TBI_FFEnable
                                        * Referenced by: '<S2>/Constant10'
                                        */
extern real_T C_TBI_FFUpKs;            /* Variable: C_TBI_FFUpKs
                                        * Referenced by: '<S20>/Constant3'
                                        */
extern real_T C_TBI_FFUpTPC;           /* Variable: C_TBI_FFUpTPC
                                        * Referenced by: '<S20>/Constant1'
                                        */

/* Model entry point functions */
extern void TBI_PositionControl_initialize(void);
extern void TBI_PositionControl_step(void);

/* Real-time Model object */
extern RT_MODEL *const rtM;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S25>/Data Type Duplicate' : Unused code path elimination
 * Block '<S25>/Data Type Propagation' : Unused code path elimination
 * Block '<S26>/Data Type Duplicate' : Unused code path elimination
 * Block '<S26>/Data Type Propagation' : Unused code path elimination
 * Block '<S19>/Constant1' : Unused code path elimination
 * Block '<S19>/Scope' : Unused code path elimination
 * Block '<S24>/Data Type Duplicate' : Unused code path elimination
 * Block '<S24>/Data Type Propagation' : Unused code path elimination
 * Block '<Root>/Scope' : Unused code path elimination
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
 * '<Root>' : 'TBI_PositionControl'
 * '<S1>'   : 'TBI_PositionControl/MainFunction'
 * '<S2>'   : 'TBI_PositionControl/MainFunction1'
 * '<S3>'   : 'TBI_PositionControl/Scheduler'
 * '<S4>'   : 'TBI_PositionControl/Simulation Out'
 * '<S5>'   : 'TBI_PositionControl/MainFunction/Control_Down'
 * '<S6>'   : 'TBI_PositionControl/MainFunction/Control_Select'
 * '<S7>'   : 'TBI_PositionControl/MainFunction/Control_Up'
 * '<S8>'   : 'TBI_PositionControl/MainFunction/Filter_Setpoint'
 * '<S9>'   : 'TBI_PositionControl/MainFunction/Saturation Dynamic'
 * '<S10>'  : 'TBI_PositionControl/MainFunction/Control_Down/Saturation Dynamic'
 * '<S11>'  : 'TBI_PositionControl/MainFunction/Control_Select/If Action Subsystem'
 * '<S12>'  : 'TBI_PositionControl/MainFunction/Control_Select/If Action Subsystem1'
 * '<S13>'  : 'TBI_PositionControl/MainFunction/Control_Up/Saturation Dynamic'
 * '<S14>'  : 'TBI_PositionControl/MainFunction/Filter_Setpoint/If Action Subsystem'
 * '<S15>'  : 'TBI_PositionControl/MainFunction/Filter_Setpoint/If Action Subsystem1'
 * '<S16>'  : 'TBI_PositionControl/MainFunction/Filter_Setpoint/If Action Subsystem2'
 * '<S17>'  : 'TBI_PositionControl/MainFunction1/Control_Down'
 * '<S18>'  : 'TBI_PositionControl/MainFunction1/Control_Up'
 * '<S19>'  : 'TBI_PositionControl/MainFunction1/Dither'
 * '<S20>'  : 'TBI_PositionControl/MainFunction1/FF'
 * '<S21>'  : 'TBI_PositionControl/MainFunction1/If Action Subsystem'
 * '<S22>'  : 'TBI_PositionControl/MainFunction1/If Action Subsystem1'
 * '<S23>'  : 'TBI_PositionControl/MainFunction1/If Action Subsystem2'
 * '<S24>'  : 'TBI_PositionControl/MainFunction1/Saturation Dynamic'
 * '<S25>'  : 'TBI_PositionControl/MainFunction1/Control_Down/Saturation Dynamic'
 * '<S26>'  : 'TBI_PositionControl/MainFunction1/Control_Up/Saturation Dynamic'
 * '<S27>'  : 'TBI_PositionControl/Simulation Out/Model'
 */
#endif                                 /* RTW_HEADER_TBI_PositionControl_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
