/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: FUEL_FuelControl.h
 *
 * Code generated for Simulink model 'FUEL_FuelControl'.
 *
 * Model version                  : 18.6
 * Simulink Coder version         : 24.2 (R2024b) 21-Jun-2024
 * C/C++ source code generated on : Mon May  5 23:39:53 2025
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef FUEL_FuelControl_h_
#define FUEL_FuelControl_h_
#ifndef FUEL_FuelControl_COMMON_INCLUDES_
#define FUEL_FuelControl_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* FUEL_FuelControl_COMMON_INCLUDES_ */

#include "FUEL_FuelControl_types.h"
#include "rt_nonfinite.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Block states (default storage) for system '<Root>' */
typedef struct {
  uint16_T Delay_DSTATE;               /* '<S11>/Delay' */
} DW_FUEL_FuelControl_T;

/* Invariant block signals (default storage) */
typedef struct {
  const real_T Add1;                   /* '<S7>/Add1' */
} ConstB_FUEL_FuelControl_T;

/* Constant parameters (default storage) */
typedef struct {
  /* Computed Parameter: uDLookupTable_maxIndex
   * Referenced by: '<S5>/2-D Lookup Table'
   */
  uint32_T uDLookupTable_maxIndex[2];
} ConstP_FUEL_FuelControl_T;

/* Real-time Model Data Structure */
struct tag_RTM_FUEL_FuelControl_T {
  const char_T * volatile errorStatus;
};

/* Block states (default storage) */
extern DW_FUEL_FuelControl_T FUEL_FuelControl_DW;
extern const ConstB_FUEL_FuelControl_T FUEL_FuelControl_ConstB;/* constant block i/o */

/* Constant parameters (default storage) */
extern const ConstP_FUEL_FuelControl_T FUEL_FuelControl_ConstP;

/*
 * Exported Global Parameters
 *
 * Note: Exported global parameters are tunable parameters with an exported
 * global storage class designation.  Code generation will declare the memory for
 * these parameters and exports their symbols.
 *
 */
extern real_T C_INJMAPA_PRESSAO_Y_AXIS[9];/* Variable: C_INJMAPA_PRESSAO_Y_AXIS
                                           * Referenced by: '<S5>/2-D Lookup Table'
                                           */
extern real_T C_INJMAPA_RPM_X_AXIS[9]; /* Variable: C_INJMAPA_RPM_X_AXIS
                                        * Referenced by: '<S5>/2-D Lookup Table'
                                        */
extern real_T C_INJ_MAPA_DATA[81];     /* Variable: C_INJ_MAPA_DATA
                                        * Referenced by: '<S5>/2-D Lookup Table'
                                        */
extern int16_T C_Brkpt_EngSpeed_InjAdv[66];/* Variable: C_Brkpt_EngSpeed_InjAdv
                                            * Referenced by: '<S1>/1-D Lookup Table'
                                            */
extern int16_T C_Vet_deg_InjectionAdvance[66];/* Variable: C_Vet_deg_InjectionAdvance
                                               * Referenced by: '<S1>/1-D Lookup Table'
                                               */

/* Model entry point functions */
extern void FUEL_FuelControl_initialize(void);
extern void FUEL_FuelControl_step(void);
extern void FUEL_FuelControl_terminate(void);

/* Real-time Model object */
extern RT_MODEL_FUEL_FuelControl_T *const FUEL_FuelControl_M;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S11>/Display1' : Unused code path elimination
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
 * '<Root>' : 'FUEL_FuelControl'
 * '<S1>'   : 'FUEL_FuelControl/Injection_Advance'
 * '<S2>'   : 'FUEL_FuelControl/Injection_Time'
 * '<S3>'   : 'FUEL_FuelControl/Input_Control'
 * '<S4>'   : 'FUEL_FuelControl/Injection_Time/Compare To Constant'
 * '<S5>'   : 'FUEL_FuelControl/Injection_Time/If Action Running'
 * '<S6>'   : 'FUEL_FuelControl/Injection_Time/If Action Subsystem1'
 * '<S7>'   : 'FUEL_FuelControl/Injection_Time/If Action Running/CoolTemp_Inj_Factor'
 * '<S8>'   : 'FUEL_FuelControl/Injection_Time/If Action Running/FuelMass_Calculation'
 * '<S9>'   : 'FUEL_FuelControl/Injection_Time/If Action Running/Inj_Factor_Calculation'
 * '<S10>'  : 'FUEL_FuelControl/Injection_Time/If Action Running/Inj_Time_Calculation'
 * '<S11>'  : 'FUEL_FuelControl/Injection_Time/If Action Running/Inj_Factor_Calculation/D_Factor_Inj'
 * '<S12>'  : 'FUEL_FuelControl/Injection_Time/If Action Running/Inj_Factor_Calculation/Inj_Factor'
 * '<S13>'  : 'FUEL_FuelControl/Injection_Time/If Action Running/Inj_Factor_Calculation/Inj_Factor/If Action Subsystem'
 * '<S14>'  : 'FUEL_FuelControl/Injection_Time/If Action Running/Inj_Factor_Calculation/Inj_Factor/If Action Subsystem1'
 */
#endif                                 /* FUEL_FuelControl_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
