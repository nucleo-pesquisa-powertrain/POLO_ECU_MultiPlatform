/*
 * File: Management.c
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

#include "Management.h"
#include "Management_private.h"

/* Exported block parameters */
uint16_T C_MNGT_DisturbEngSpeedThreshold = 750U;/* Variable: C_MNGT_DisturbEngSpeedThreshold
                                                 * Referenced by: '<S1>/Constant5'
                                                 */

/* Block signals and states (auto storage) */
D_Work_Management rtDWork_Management;

/* Real-time model */
RT_MODEL_Management rtM_Management_;
RT_MODEL_Management *const rtM_Management = &rtM_Management_;

/* Model step function */
void Management_step(void)
{
  uint16_T rtb_Switch2;
  uint16_T rtb_Switch1;
  uint16_T rtb_Switch3;

  /* Switch: '<S3>/Switch2' incorporates:
   *  Constant: '<S3>/Constant2'
   *  Delay: '<S3>/Delay1'
   *  Delay: '<S3>/Delay2'
   *  Sum: '<S3>/Add'
   */
  if (rtDWork_Management.Delay2_DSTATE >= 1) {
    rtb_Switch2 = (uint16_T)((uint32_T)rtDWork_Management.Delay2_DSTATE +
      rtDWork_Management.Delay1_DSTATE);
  } else {
    rtb_Switch2 = 0U;
  }

  /* End of Switch: '<S3>/Switch2' */

  /* Switch: '<S3>/Switch1' incorporates:
   *  Constant: '<S1>/Constant4'
   *  Constant: '<S1>/Constant5'
   *  Constant: '<S1>/Constant7'
   *  DataStoreRead: '<S2>/Data Store Read3'
   *  Delay: '<S1>/Delay'
   *  Delay: '<S3>/Delay'
   *  Logic: '<S1>/Logical Operator'
   *  Logic: '<S3>/Logical Operator'
   *  Logic: '<S3>/Logical Operator1'
   *  RelationalOperator: '<S1>/Relational Operator'
   *  RelationalOperator: '<S1>/Relational Operator1'
   *  RelationalOperator: '<S3>/Relational Operator1'
   */
  rtb_Switch1 = (uint16_T)(((rtDWork_Management.Delay_DSTATE != 0) ||
    ((rtDWork_Management.Delay_DSTATE_m == 0) && (Get_MNGT_rpm_EngSpeed() <
    C_MNGT_DisturbEngSpeedThreshold))) && (1000 >= rtb_Switch2));

  /* Switch: '<S1>/Switch1' incorporates:
   *  Constant: '<S1>/Constant6'
   *  DataStoreRead: '<S2>/Data Store Read3'
   *  Switch: '<S1>/Switch3'
   */
  if (rtb_Switch1 >= 1) {
    rtb_Switch3 = 3U;
  } else if (Get_MNGT_rpm_EngSpeed() >= 500) {
    /* Switch: '<S1>/Switch3' incorporates:
     *  DataStoreRead: '<S2>/Data Store Read1'
     *  Switch: '<S1>/Switch2'
     */
    rtb_Switch3 = (uint16_T)(Get_MNGT_p_Pedal() > 50);
  } else {
    /* Switch: '<S1>/Switch3' incorporates:
     *  Constant: '<S1>/Constant1'
     */
    rtb_Switch3 = 2U;
  }

  /* End of Switch: '<S1>/Switch1' */

  /* DataStoreWrite: '<Root>/Data Store Write' */
  Set_MNGT_s_ManagementState(rtb_Switch3);

  /* Update for Delay: '<S3>/Delay' */
  rtDWork_Management.Delay_DSTATE = rtb_Switch1;

  /* Update for Delay: '<S1>/Delay' */
  rtDWork_Management.Delay_DSTATE_m = rtb_Switch3;

  /* Update for Delay: '<S3>/Delay2' */
  rtDWork_Management.Delay2_DSTATE = rtb_Switch1;

  /* Update for Delay: '<S3>/Delay1' */
  rtDWork_Management.Delay1_DSTATE = rtb_Switch2;
}

/* Model initialize function */
void Management_initialize(void)
{
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
