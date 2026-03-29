/*
 * File: TBI_PositionControl.c
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

#include "TBI_PositionControl.h"

/* Exported block parameters */
real_T C_TBI_ControlDown_Ki = 0.1;    /* Variable: C_TBI_ControlDown_Ki
                                        * Referenced by: '<S17>/Constant3'
                                        */
real_T C_TBI_ControlDown_Kp = 10;     /* Variable: C_TBI_ControlDown_Kp
                                        * Referenced by: '<S17>/Constant2'
                                        */
real_T C_TBI_ControlUp_Ki = 0.1;       /* Variable: C_TBI_ControlUp_Ki
                                        * Referenced by: '<S18>/Constant3'
                                        */
real_T C_TBI_ControlUp_Kp = 10.0;      /* Variable: C_TBI_ControlUp_Kp
                                        * Referenced by: '<S18>/Kp'
                                        */
real_T C_TBI_DeadBandError = 0.1;      /* Variable: C_TBI_DeadBandError
                                        * Referenced by: '<S18>/Constant6'
                                        */
real_T C_TBI_DitherAmplitude[10] = { 1.0, 1.0, 0.25, 2.0, 2.5, 2.5, 1.0,
  2.0, 1.0, 1.0 } ;                 /* Variable: C_TBI_DitherAmplitude
                                        * Referenced by: '<S19>/1-D Lookup Table'
                                        */

real_T C_TBI_DitherEnable = 1.0;       /* Variable: C_TBI_DitherEnable
                                        * Referenced by: '<S2>/Constant11'
                                        */
real_T C_TBI_DitherFreq = 50.0;        /* Variable: C_TBI_DitherFreq
                                        * Referenced by: '<S19>/Constant3'
                                        */
real_T C_TBI_DitherPositionBrkpts[10] = { 0.0, 1.5, 3.0, 4.5, 6.0, 7.5, 8.0, 9.0,
  20.0, 30.0 } ;                       /* Variable: C_TBI_DitherPositionBrkpts
                                        * Referenced by: '<S19>/1-D Lookup Table'
                                        */

real_T C_TBI_FFDownKs = 0;         /* Variable: C_TBI_FFDownKs
                                        * Referenced by: '<S20>/Constant4'
                                        */
real_T C_TBI_FFDownTPC = -27;        /* Variable: C_TBI_FFDownTPC
                                        * Referenced by: '<S20>/Constant2'
                                        */
real_T C_TBI_FFEnable = 1.0;           /* Variable: C_TBI_FFEnable
                                        * Referenced by: '<S2>/Constant10'
                                        */
real_T C_TBI_FFUpKs = 0;          /* Variable: C_TBI_FFUpKs
                                        * Referenced by: '<S20>/Constant3'
                                        */
real_T C_TBI_FFUpTPC = 28;        /* Variable: C_TBI_FFUpTPC
                                        * Referenced by: '<S20>/Constant1'
                                        */

real_T C_TBI_LHMin = 8.0;

real_T C_TBI_LHMax = 9.0;

/* Block signals and states (auto storage) */
DW rtDW;

/* Real-time model */
RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;
real_T look1_binlx(real_T u0, const real_T bp0[], const real_T table[], uint32_T
                   maxIndex);
real_T look1_binlx(real_T u0, const real_T bp0[], const real_T table[], uint32_T
                   maxIndex)
{
  real_T frac;
  uint32_T iRght;
  uint32_T iLeft;
  uint32_T bpIdx;

  /* Lookup 1-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear'
     Extrapolation method: 'Linear'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Linear'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u0 <= bp0[0UL]) {
    iLeft = 0UL;
    frac = (u0 - bp0[0UL]) / (bp0[1UL] - bp0[0UL]);
  } else if (u0 < bp0[maxIndex]) {
    /* Binary Search */
    bpIdx = maxIndex >> 1UL;
    iLeft = 0UL;
    iRght = maxIndex;
    while (iRght - iLeft > 1UL) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1UL;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1UL] - bp0[iLeft]);
  } else {
    iLeft = maxIndex - 1UL;
    frac = (u0 - bp0[maxIndex - 1UL]) / (bp0[maxIndex] - bp0[maxIndex - 1UL]);
  }

  /* Interpolation 1-D
     Interpolation method: 'Linear'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'wrapping'
   */
  return (table[iLeft + 1UL] - table[iLeft]) * frac + table[iLeft];
}

/* Model step function */
void TBI_PositionControl_step(void)
{
  real_T rtb_Divide1;
  real_T rtb_Divide2;
  boolean_T rtb_RelationalOperator_l;
  real_T rtb_Subtract;
  real_T rtb_Add1_hj;
  boolean_T rtb_Switch3_k;
  uint8_T Merge;

  /* DataStoreRead: '<Root>/Data Store Read1' */
  rtDW.Sensor = Get_TBI_deg_Feedback();

  /* S-Function (fcncallgen): '<S3>/Function-Call Generator' incorporates:
   *  SubSystem: '<Root>/MainFunction1'
   */
  /* Product: '<S2>/Divide1' incorporates:
   *  Constant: '<S2>/Constant4'
   *  DataStoreRead: '<Root>/Data Store Read'
   *  DataTypeConversion: '<S2>/Data Type Conversion2'
   */
  rtb_Divide1 = (real_T)Get_TBI_deg_SetPoint() / 10.0;

  /* Product: '<S2>/Divide2' incorporates:
   *  Constant: '<S2>/Constant5'
   *  DataTypeConversion: '<S2>/Data Type Conversion1'
   */
  rtb_Divide2 = (real_T)rtDW.Sensor / 10.0;

  /* If: '<S2>/If' incorporates:
   *  Constant: '<S2>/Constant'
   *  Constant: '<S2>/Constant1'
   *  Constant: '<S2>/Constant14'
   *  Inport: '<S21>/In1'
   *  Inport: '<S22>/In1'
   *  Inport: '<S23>/In1'
   */
  if (((rtb_Divide1 > C_TBI_LHMax) && (rtb_Divide2 > C_TBI_LHMin)) || ((rtb_Divide1 <= C_TBI_LHMax) &&
       (rtb_Divide2 > C_TBI_LHMax))) {
    /* Outputs for IfAction SubSystem: '<S2>/If Action Subsystem' incorporates:
     *  ActionPort: '<S21>/Action Port'
     */
    Merge = 1U;

    /* End of Outputs for SubSystem: '<S2>/If Action Subsystem' */
  } else if (((rtb_Divide1 < C_TBI_LHMin) && (rtb_Divide2 <= C_TBI_LHMax)) || ((rtb_Divide1 >=
               C_TBI_LHMin) && (rtb_Divide2 < C_TBI_LHMin))) {
    /* Outputs for IfAction SubSystem: '<S2>/If Action Subsystem1' incorporates:
     *  ActionPort: '<S22>/Action Port'
     */
    Merge = 2U;

    /* End of Outputs for SubSystem: '<S2>/If Action Subsystem1' */
  } else {
    /* Outputs for IfAction SubSystem: '<S2>/If Action Subsystem2' incorporates:
     *  ActionPort: '<S23>/Action Port'
     */
    Merge = 3U;

    /* End of Outputs for SubSystem: '<S2>/If Action Subsystem2' */
  }

  /* End of If: '<S2>/If' */

  /* Outputs for Enabled SubSystem: '<S2>/Control_Up' incorporates:
   *  EnablePort: '<S18>/Enable'
   */
  /* Sum: '<S18>/Subtract' */
  rtb_Divide1 -= rtb_Divide2;

  /* Switch: '<S18>/Switch3' incorporates:
   *  Abs: '<S18>/Abs'
   *  Constant: '<S18>/Constant5'
   *  Constant: '<S18>/Constant6'
   *  RelationalOperator: '<S18>/Relational Operator'
   */
  if (fabs(rtb_Divide1) <= C_TBI_DeadBandError) {
    rtb_Divide1 = 0.0;
  }

  /* End of Switch: '<S18>/Switch3' */

  /* Sum: '<S18>/Add1' incorporates:
   *  Constant: '<S18>/Constant3'
   *  Delay: '<S18>/Delay'
   *  Delay: '<S18>/Delay1'
   *  Product: '<S18>/Product1'
   *  Sum: '<S18>/Add2'
   */
  rtb_Subtract = (rtDW.Delay_DSTATE_c + rtb_Divide1) * C_TBI_ControlUp_Ki +
    rtDW.Delay1_DSTATE;

  /* Sum: '<S18>/Add' incorporates:
   *  Constant: '<S18>/Kp'
   *  Product: '<S18>/Product'
   */
  rtb_Add1_hj = rtb_Divide1 * C_TBI_ControlUp_Kp + rtb_Subtract;

  /* Switch: '<S26>/Switch2' incorporates:
   *  Constant: '<S18>/Constant'
   *  Constant: '<S18>/Constant1'
   *  RelationalOperator: '<S26>/LowerRelop1'
   *  RelationalOperator: '<S26>/UpperRelop'
   *  Switch: '<S26>/Switch'
   */
  if (rtb_Add1_hj > 100.0) {
    rtDW.Switch2 = 100.0;
  } else if (rtb_Add1_hj < -100.0) {
    /* Switch: '<S26>/Switch' incorporates:
     *  Constant: '<S18>/Constant1'
     */
    rtDW.Switch2 = -100.0;
  } else {
    rtDW.Switch2 = rtb_Add1_hj;
  }

  /* End of Switch: '<S26>/Switch2' */

  /* Update for Delay: '<S18>/Delay1' */
  rtDW.Delay1_DSTATE = rtb_Subtract;

  /* Update for Delay: '<S18>/Delay' */
  rtDW.Delay_DSTATE_c = rtb_Divide1;

  /* End of Outputs for SubSystem: '<S2>/Control_Up' */

  /* Lookup_n-D: '<S19>/1-D Lookup Table' */
  rtb_Subtract = look1_binlx(rtb_Divide2, C_TBI_DitherPositionBrkpts,
    C_TBI_DitherAmplitude, 9UL);

  /* Switch: '<S19>/Switch2' incorporates:
   *  Constant: '<S19>/Constant2'
   *  Constant: '<S19>/Constant4'
   *  Delay: '<S19>/Delay'
   *  Delay: '<S19>/Delay1'
   *  Sum: '<S19>/Add2'
   */
  if (rtDW.Delay1_DSTATE_k) {
    rtb_Divide1 = 0.0;
  } else {
    rtb_Divide1 = rtDW.Delay_DSTATE + 1.0;
  }

  /* End of Switch: '<S19>/Switch2' */

  /* RelationalOperator: '<S19>/Relational Operator' incorporates:
   *  Constant: '<S19>/Constant3'
   *  Constant: '<S19>/Constant5'
   *  Constant: '<S19>/Cycle'
   *  Constant: '<S19>/FreqToTime'
   *  Constant: '<S19>/Ts'
   *  Product: '<S19>/Divide'
   *  Product: '<S19>/Divide1'
   *  Product: '<S19>/Divide2'
   *  Sum: '<S19>/Add1'
   */
  rtb_RelationalOperator_l = (rtb_Divide1 >= 1.0 / C_TBI_DitherFreq / 2.0 / 0.01
    - 1.0);

  /* Switch: '<S19>/Switch3' incorporates:
   *  Delay: '<S19>/Delay2'
   *  Logic: '<S19>/Logical Operator1'
   */
  if (rtb_RelationalOperator_l) {
    rtb_Switch3_k = !rtDW.Delay2_DSTATE;
  } else {
    rtb_Switch3_k = rtDW.Delay2_DSTATE;
  }

  /* End of Switch: '<S19>/Switch3' */

  /* Switch: '<S2>/Switch1' incorporates:
   *  Constant: '<S2>/Constant2'
   *  DataStoreRead: '<Root>/Data Store Read2'
   *  Sum: '<S2>/Add1'
   *  Switch: '<S2>/Switch2'
   */
  if (Get_IgnitionOn() >= 1) {
    /* Switch: '<S2>/Switch4' incorporates:
     *  Constant: '<S2>/Constant11'
     *  Constant: '<S2>/Constant13'
     */
    if (C_TBI_DitherEnable >= 0.5) {
      /* Switch: '<S19>/Switch1' incorporates:
       *  Product: '<S19>/Product1'
       */
      if (!rtb_Switch3_k) {
        rtb_Subtract = -rtb_Subtract;
      }

      /* End of Switch: '<S19>/Switch1' */
    } else {
      rtb_Subtract = 0.0;
    }

    /* End of Switch: '<S2>/Switch4' */

    /* Switch: '<S2>/Switch3' incorporates:
     *  Constant: '<S2>/Constant10'
     *  Constant: '<S2>/Constant12'
     */
    if (C_TBI_FFEnable >= 0.5) {
      /* MultiPortSwitch: '<S20>/Multiport Switch' incorporates:
       *  Constant: '<S20>/Constant1'
       *  Constant: '<S20>/Constant12'
       *  Constant: '<S20>/Constant2'
       *  Constant: '<S20>/Constant3'
       *  Constant: '<S20>/Constant4'
       *  Product: '<S20>/Product1'
       *  Product: '<S20>/Product2'
       *  Sum: '<S20>/Add1'
       *  Sum: '<S20>/Add2'
       */
      switch (Merge) {
       case 1:
        rtb_Divide2 = C_TBI_FFUpKs * rtb_Divide2 + C_TBI_FFUpTPC;
        break;

       case 2:
        rtb_Divide2 = C_TBI_FFDownKs * rtb_Divide2 + C_TBI_FFDownTPC;
        break;

       default:
        rtb_Divide2 = 0.0;
        break;
      }

      /* End of MultiPortSwitch: '<S20>/Multiport Switch' */
    } else {
      rtb_Divide2 = 0.0;
    }

    /* End of Switch: '<S2>/Switch3' */
    rtb_Divide2 = (rtDW.Switch2 + rtb_Divide2) + rtb_Subtract;
  } else {
    rtb_Divide2 = 0.0;
  }

  /* End of Switch: '<S2>/Switch1' */

  /* Switch: '<S24>/Switch2' incorporates:
   *  Constant: '<S2>/Constant8'
   *  Constant: '<S2>/Constant9'
   *  RelationalOperator: '<S24>/LowerRelop1'
   *  RelationalOperator: '<S24>/UpperRelop'
   *  Switch: '<S24>/Switch'
   */
  if (rtb_Divide2 > 100.0) {
    rtb_Divide2 = 100.0;
  } else {
    if (rtb_Divide2 < -100.0) {
      /* Switch: '<S24>/Switch' incorporates:
       *  Constant: '<S2>/Constant9'
       */
      rtb_Divide2 = -100.0;
    }
  }

  /* End of Switch: '<S24>/Switch2' */

  /* Product: '<S2>/Product' incorporates:
   *  Constant: '<S2>/Constant3'
   */
  rtb_Divide2 *= 10.0;

  /* Update for Delay: '<S19>/Delay2' */
  rtDW.Delay2_DSTATE = rtb_Switch3_k;

  /* Update for Delay: '<S19>/Delay1' */
  rtDW.Delay1_DSTATE_k = rtb_RelationalOperator_l;

  /* Update for Delay: '<S19>/Delay' */
  rtDW.Delay_DSTATE = rtb_Divide1;

  /* DataStoreWrite: '<Root>/Data Store Write' incorporates:
   *  DataTypeConversion: '<S2>/Data Type Conversion'
   */
  Set_TBI_PWM_Out((int16_T)floor(rtb_Divide2));

  /* End of Outputs for S-Function (fcncallgen): '<S3>/Function-Call Generator' */

  /* DataStoreWrite: '<Root>/Data Store Write1' */
  Set_TBI_type(Merge);
}

/* Model initialize function */
void TBI_PositionControl_initialize(void)
{
  /* Start for DataStoreMemory: '<Root>/Data Store Memory SetPoint' */
  Set_TBI_deg_SetPoint(60U);
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
