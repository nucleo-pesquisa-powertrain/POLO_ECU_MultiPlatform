/*
 * File: SPK_SparkAdvance.h
 *
 * Code generated for Simulink model 'SPK_SparkAdvance'.
 *
 * Model version                  : 1.13
 * Simulink Coder version         : 8.10 (R2016a) 10-Feb-2016
 * C/C++ source code generated on : Sat Dec 09 15:02:24 2023
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Custom Processor->Custom
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_SPK_SparkAdvance_h_
#define RTW_HEADER_SPK_SparkAdvance_h_
#include <math.h>
#include <stddef.h>
#ifndef SPK_SparkAdvance_COMMON_INCLUDES_
# define SPK_SparkAdvance_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif                                 /* SPK_SparkAdvance_COMMON_INCLUDES_ */

#include "SPK_SparkAdvance_types.h"

/* Macros for accessing real-time model data structure */

/* Block signals and states (auto storage) for system '<Root>' */
typedef struct {
  real32_T Delay_DSTATE;               /* '<S1>/Delay' */
  real32_T Delay1_DSTATE;              /* '<S1>/Delay1' */
} D_Work;

/* Constant parameters (auto storage) */
typedef struct {
  /* Pooled Parameter (Expression: )
   * Referenced by:
   *   '<Root>/2-D Lookup E0'
   *   '<Root>/2-D Lookup E100'
   */
  uint32_T pooled2[2];

  /* Pooled Parameter (Expression: C_Brkpt_EngSpeed_SparkAdv)
   * Referenced by:
   *   '<Root>/2-D Lookup E0'
   *   '<Root>/2-D Lookup E100'
   */
  int16_T pooled4[66];

  /* Pooled Parameter (Expression: C_Brkpt_MAP_SparkAdv)
   * Referenced by:
   *   '<Root>/2-D Lookup E0'
   *   '<Root>/2-D Lookup E100'
   */
  int16_T pooled5[10];
} ConstParam;

/* Block signals and states (auto storage) */
extern D_Work rtDWork;

/* Constant parameters (auto storage) */
extern const ConstParam rtConstP;

/*
 * Exported Global Parameters
 *
 * Note: Exported global parameters are tunable parameters with an exported
 * global storage class designation.  Code generation will declare the memory for
 * these parameters and exports their symbols.
 *
 */
extern int32_T C_Tbl_deg_SparkAdvance[660];/* Variable: C_Tbl_deg_SparkAdvance
                                            * Referenced by: '<Root>/2-D Lookup E0'
                                            */
extern int32_T C_Tbl_deg_SparkAdvanceE100[660];/* Variable: C_Tbl_deg_SparkAdvanceE100
                                                * Referenced by: '<Root>/2-D Lookup E100'
                                                */

/* Model entry point functions */
extern void SPK_SparkAdvance_initialize(void);
extern void SPK_SparkAdvance_step(void);

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S2>/Data Type Duplicate' : Unused code path elimination
 * Block '<S2>/Data Type Propagation' : Unused code path elimination
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
 * '<Root>' : 'SPK_SparkAdvance'
 * '<S1>'   : 'SPK_SparkAdvance/IDLE Control'
 * '<S2>'   : 'SPK_SparkAdvance/IDLE Control/Saturation Dynamic'
 */
#endif                                 /* RTW_HEADER_SPK_SparkAdvance_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
