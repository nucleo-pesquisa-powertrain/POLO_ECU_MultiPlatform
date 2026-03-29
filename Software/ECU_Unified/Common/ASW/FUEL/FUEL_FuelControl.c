/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: FUEL_FuelControl.c
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

#include "FUEL_FuelControl.h"
#include "fuel_interface.h"
#include <math.h>
#include "rt_nonfinite.h"
#include "rtwtypes.h"
#include "FUEL_FuelControl_private.h"

/* Exported block parameters */
real_T C_INJMAPA_PRESSAO_Y_AXIS[9] = { 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0,
  80.0, 90.0 } ;                       /* Variable: C_INJMAPA_PRESSAO_Y_AXIS
                                        * Referenced by: '<S5>/2-D Lookup Table'
                                        */

real_T C_INJMAPA_RPM_X_AXIS[9] = { 700.0, 800.0, 900.0, 1000.0, 1100.0, 1200.0,
  1300.0, 1400.0, 1500.0 } ;           /* Variable: C_INJMAPA_RPM_X_AXIS
                                        * Referenced by: '<S5>/2-D Lookup Table'
                                        */

real_T C_INJ_MAPA_DATA[81] = { 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 2.9,
  2.9, 2.9, 2.9, 2.9, 2.9, 2.9, 2.9, 2.9, 2.7, 2.7, 2.7, 2.7, 2.7, 2.7, 2.7, 2.7,
  2.7, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.3, 2.3, 2.3, 2.3, 2.3, 2.3,
  2.3, 2.3, 2.3, 2.1, 2.1, 2.1, 2.1, 2.1, 2.1, 2.1, 2.1, 2.1, 2.0, 2.0, 2.0, 2.0,
  2.0, 2.0, 2.0, 2.0, 2.0, 1.9, 1.9, 1.9, 1.9, 1.9, 1.9, 1.9, 1.9, 1.9, 1.8, 1.8,
  1.8, 1.8, 1.8, 1.8, 1.8, 1.8, 1.8 } ;/* Variable: C_INJ_MAPA_DATA
                                        * Referenced by: '<S5>/2-D Lookup Table'
                                        */

int16_T C_Brkpt_EngSpeed_InjAdv[66] = { 0, 100, 200, 300, 400, 500, 600, 700,
  800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000,
  2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900, 3000, 3100, 3200, 3300,
  3400, 3500, 3600, 3700, 3800, 3900, 4000, 4100, 4200, 4300, 4400, 4500, 4600,
  4700, 4800, 4900, 5000, 5100, 5200, 5300, 5400, 5500, 5600, 5700, 5800, 5900,
  6000, 6100, 6200, 6300, 6400, 6500 } ;/* Variable: C_Brkpt_EngSpeed_InjAdv
                                         * Referenced by: '<S1>/1-D Lookup Table'
                                         */

int16_T C_Vet_deg_InjectionAdvance[66] = { -1116, -21, -21, -21, -21, -21, -21,
  -21, -21, -21, -21, -22, -23, -24, -25, -26, -27, -29, -30, -31, -32, -33, -35,
  -36, -37, -39, -40, -41, -43, -44, -45, -46, -48, -49, -50, -51, -53, -54, -55,
  -56, -57, -59, -60, -61, -62, -63, -64, -65, -66, -66, -67, -68, -69, -70, -70,
  -71, -71, -72, -72, -73, -73, -73, -74, -74, -74, -1116 } ;/* Variable: C_Vet_deg_InjectionAdvance
                                                              * Referenced by: '<S1>/1-D Lookup Table'
                                                              */

/* Block states (default storage) */
DW_FUEL_FuelControl_T FUEL_FuelControl_DW;

/* Real-time model */
static RT_MODEL_FUEL_FuelControl_T FUEL_FuelControl_M_;
RT_MODEL_FUEL_FuelControl_T *const FUEL_FuelControl_M = &FUEL_FuelControl_M_;
int16_T look1_is16lu16n16Ds32_binlcas(int16_T u0, const int16_T bp0[], const
  int16_T table[], uint32_T maxIndex)
{
  uint32_T bpIdx;
  uint32_T iLeft;
  uint32_T iRght;
  int16_T bpLeftVar;
  int16_T y;
  uint16_T frac;

  /* Column-major Lookup 1-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
     Extrapolation method: 'Clip'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = 0U;
  } else if (u0 < bp0[maxIndex]) {
    /* Binary Search */
    bpIdx = maxIndex >> 1U;
    iLeft = 0U;
    iRght = maxIndex;
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    bpLeftVar = bp0[iLeft];
    frac = (uint16_T)(((uint32_T)(u0 - bpLeftVar) << 16) / (uint32_T)(bp0[iLeft
      + 1U] - bpLeftVar));
  } else {
    iLeft = maxIndex;
    frac = 0U;
  }

  /* Column-major Interpolation 1-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'on'
     Rounding mode: 'simplest'
     Overflow mode: 'wrapping'
   */
  if (iLeft == maxIndex) {
    y = table[iLeft];
  } else {
    bpLeftVar = table[iLeft];
    y = (int16_T)((int16_T)(((table[iLeft + 1U] - bpLeftVar) * frac) >> 16) +
                  bpLeftVar);
  }

  return y;
}

real_T look2_binlxpw(real_T u0, real_T u1, const real_T bp0[], const real_T bp1[],
                     const real_T table[], const uint32_T maxIndex[], uint32_T
                     stride)
{
  real_T fractions[2];
  real_T frac;
  real_T yL_0d0;
  real_T yL_0d1;
  uint32_T bpIndices[2];
  uint32_T bpIdx;
  uint32_T iLeft;
  uint32_T iRght;

  /* Column-major Lookup 2-D
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear point-slope'
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
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = (u0 - bp0[0U]) / (bp0[1U] - bp0[0U]);
  } else if (u0 < bp0[maxIndex[0U]]) {
    /* Binary Search */
    bpIdx = maxIndex[0U] >> 1U;
    iLeft = 0U;
    iRght = maxIndex[0U];
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex[0U] - 1U;
    frac = (u0 - bp0[maxIndex[0U] - 1U]) / (bp0[maxIndex[0U]] - bp0[maxIndex[0U]
      - 1U]);
  }

  fractions[0U] = frac;
  bpIndices[0U] = iLeft;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Linear'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'off'
     Remove protection against out-of-range input in generated code: 'off'
   */
  if (u1 <= bp1[0U]) {
    iLeft = 0U;
    frac = (u1 - bp1[0U]) / (bp1[1U] - bp1[0U]);
  } else if (u1 < bp1[maxIndex[1U]]) {
    /* Binary Search */
    bpIdx = maxIndex[1U] >> 1U;
    iLeft = 0U;
    iRght = maxIndex[1U];
    while (iRght - iLeft > 1U) {
      if (u1 < bp1[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u1 - bp1[iLeft]) / (bp1[iLeft + 1U] - bp1[iLeft]);
  } else {
    iLeft = maxIndex[1U] - 1U;
    frac = (u1 - bp1[maxIndex[1U] - 1U]) / (bp1[maxIndex[1U]] - bp1[maxIndex[1U]
      - 1U]);
  }

  /* Column-major Interpolation 2-D
     Interpolation method: 'Linear point-slope'
     Use last breakpoint for index at or above upper limit: 'off'
     Overflow mode: 'portable wrapping'
   */
  bpIdx = iLeft * stride + bpIndices[0U];
  yL_0d0 = table[bpIdx];
  yL_0d0 += (table[bpIdx + 1U] - yL_0d0) * fractions[0U];
  bpIdx += stride;
  yL_0d1 = table[bpIdx];
  return (((table[bpIdx + 1U] - yL_0d1) * fractions[0U] + yL_0d1) - yL_0d0) *
    frac + yL_0d0;
}

/* Model step function */
void FUEL_FuelControl_step(void)
{
  real_T rtb_Switch1;
  real_T tmp;
  uint16_T rtb_Subtract_p;

  /* If: '<S2>/If' incorporates:
   *  Constant: '<S4>/Constant'
   *  DataStoreRead: '<S3>/Data Store Read2'
   *  DataStoreRead: '<S3>/Data Store Read7'
   *  RelationalOperator: '<S4>/Compare'
   *  Switch: '<S7>/Switch1'
   */
  if (Get_FUEL_ECU_State() == 2) {
    /* Outputs for IfAction SubSystem: '<S2>/If Action Subsystem1' incorporates:
     *  ActionPort: '<S6>/Action Port'
     */
    /* SignalConversion generated from: '<S6>/Out1' incorporates:
     *  Constant: '<S6>/Constant'
     */
    rtb_Switch1 = 10000.0;

    /* End of Outputs for SubSystem: '<S2>/If Action Subsystem1' */
  } else {
    /* Outputs for IfAction SubSystem: '<S2>/If Action Running' incorporates:
     *  ActionPort: '<S5>/Action Port'
     */
    if (Get_FUEL_T_CoolTemp() >= 30) {
      /* Switch: '<S7>/Switch1' incorporates:
       *  Constant: '<S7>/constant2'
       */
      rtb_Switch1 = 1.0;
    } else {
      /* Switch: '<S7>/Switch1' incorporates:
       *  Constant: '<S7>/constant'
       *  DataStoreRead: '<S3>/Data Store Read2'
       *  Product: '<S7>/Product1'
       *  Sum: '<S7>/Subtract'
       */
      rtb_Switch1 = FUEL_FuelControl_ConstB.Add1 - (real_T)Get_FUEL_T_CoolTemp()
        * 0.004;
    }

    /* Switch: '<S12>/Switch' incorporates:
     *  Constant: '<S12>/Constant'
     *  DataStoreRead: '<S3>/Data Store Read3'
     *  DataStoreRead: '<S9>/Data Store Read'
     *  Product: '<S12>/Product1'
     *  Sum: '<S12>/Add1'
     */
    if (Get_FUEL_RPM_EngSpeed() > 500) {
      rtb_Switch1 += (real_T)Get_FUEL_ref_Torque() * 0.004;
    }

    /* End of Switch: '<S12>/Switch' */

    /* Switch: '<S5>/Switch' incorporates:
     *  DataStoreRead: '<S3>/Data Store Read9'
     */
    if (Get_FUEL_Select_TINJ_CALC()) {
      /* Sum: '<S11>/Subtract' incorporates:
       *  DataStoreRead: '<S9>/Data Store Read'
       *  Delay: '<S11>/Delay'
       */
      rtb_Subtract_p = (uint16_T)(Get_FUEL_ref_Torque() -
        FUEL_FuelControl_DW.Delay_DSTATE);

      /* If: '<S12>/If' incorporates:
       *  Constant: '<S13>/Constant'
       *  DataStoreRead: '<S3>/Data Store Read1'
       *  DataStoreRead: '<S3>/Data Store Read3'
       *  SignalConversion generated from: '<S13>/Out1'
       */
      if ((Get_FUEL_RPM_EngSpeed() < 1000) && (Get_FUEL_p_Pedal() > 2) &&
          (rtb_Switch1 < 1.1)) {
        /* Outputs for IfAction SubSystem: '<S12>/If Action Subsystem' incorporates:
         *  ActionPort: '<S13>/Action Port'
         */
        rtb_Switch1 = 1.1;

        /* End of Outputs for SubSystem: '<S12>/If Action Subsystem' */
      }

      /* Switch: '<S11>/Switch1' incorporates:
       *  Constant: '<S11>/Constant1'
       *  Constant: '<S11>/Constant2'
       *  Product: '<S11>/Product'
       */
      if (rtb_Subtract_p > 0) {
        tmp = 0.015 * (real_T)rtb_Subtract_p;
      } else {
        tmp = 0.0;
      }

      /* If: '<S12>/If' incorporates:
       *  Sum: '<S9>/Add'
       *  Switch: '<S11>/Switch1'
       */
      rtb_Switch1 += tmp;

      /* Saturate: '<S9>/Saturation' */
      if (rtb_Switch1 > 1.7) {
        rtb_Switch1 = 1.7;
      } else if (rtb_Switch1 < 0.75) {
        rtb_Switch1 = 0.75;
      }

      /* Product: '<S5>/Product' incorporates:
       *  Constant: '<S10>/Vazão_Inj'
       *  Constant: '<S8>/BR_Stoichiometric'
       *  Constant: '<S8>/Corr_Factor'
       *  Constant: '<S8>/Corr_Factor1'
       *  Constant: '<S8>/Corr_Factor2'
       *  DataStoreRead: '<S3>/Data Store Read4'
       *  DataStoreRead: '<S3>/Data Store Read5'
       *  DataStoreRead: '<S3>/Data Store Read6'
       *  Product: '<S10>/Product'
       *  Product: '<S8>/Divide'
       *  Product: '<S8>/Divide1'
       *  Saturate: '<S9>/Saturation'
       *  Sum: '<S10>/Add'
       *  Sum: '<S8>/Subtract'
       */
      rtb_Switch1 *= (real_T)Get_FUEL_mg_AirMass() * 1.25 / (12.7 - (real_T)
        Get_FUEL_p_EthanolPct() * 3.7 / 100.0) * 300.3 + (real_T)
        Get_FUEL_LambdaControl();

      /* Saturate: '<S5>/Saturation' */
      if (rtb_Switch1 > 16000.0) {
        rtb_Switch1 = 16000.0;
      } else if (rtb_Switch1 < 1500.0) {
        rtb_Switch1 = 1500.0;
      }

      /* End of Saturate: '<S5>/Saturation' */
    } else {
      /* Product: '<S5>/Product2' incorporates:
       *  Constant: '<S5>/Constant2'
       *  DataStoreRead: '<S3>/Data Store Read3'
       *  DataStoreRead: '<S3>/Data Store Read8'
       *  DataTypeConversion: '<Root>/Data Type Conversion'
       *  DataTypeConversion: '<Root>/Data Type Conversion2'
       *  Lookup_n-D: '<S5>/2-D Lookup Table'
       */
      rtb_Switch1 = 1000.0 * look2_binlxpw(Get_FUEL_KPa_MAP(),
        Get_FUEL_RPM_EngSpeed(), C_INJMAPA_PRESSAO_Y_AXIS, C_INJMAPA_RPM_X_AXIS,
        C_INJ_MAPA_DATA, FUEL_FuelControl_ConstP.uDLookupTable_maxIndex, 9U);

      /* Saturate: '<S5>/Saturation2' */
      if (rtb_Switch1 > 16000.0) {
        rtb_Switch1 = 16000.0;
      } else if (rtb_Switch1 < 1500.0) {
        rtb_Switch1 = 1500.0;
      }

      /* End of Saturate: '<S5>/Saturation2' */
    }

    /* End of Switch: '<S5>/Switch' */

    /* Update for Delay: '<S11>/Delay' incorporates:
     *  DataStoreRead: '<S9>/Data Store Read'
     */
    FUEL_FuelControl_DW.Delay_DSTATE = Get_FUEL_ref_Torque();

    /* End of Outputs for SubSystem: '<S2>/If Action Running' */
  }

  /* End of If: '<S2>/If' */

  /* Switch: '<S2>/Switch' incorporates:
   *  DataStoreRead: '<S3>/Data Store Read'
   */
  if (Get_FUEL_IgnitionOn() > 0) {
    /* DataTypeConversion: '<S2>/Data Type Conversion' */
    tmp = floor(rtb_Switch1);
    if (rtIsNaN(tmp)) {
      /* DataStoreWrite: '<Root>/Data Store Write' */
      Set_FUEL_us_InjectionTime(0U);
    } else {
      /* DataStoreWrite: '<Root>/Data Store Write' */
      Set_FUEL_us_InjectionTime((uint16_T)tmp);
    }
  } else {
    /* DataStoreWrite: '<Root>/Data Store Write' incorporates:
     *  Constant: '<S2>/Constant'
     *  DataTypeConversion: '<S2>/Data Type Conversion'
     */
    Set_FUEL_us_InjectionTime(0U);
  }

  /* End of Switch: '<S2>/Switch' */

  /* DataStoreWrite: '<Root>/Data Store Write1' incorporates:
   *  DataStoreRead: '<S3>/Data Store Read3'
   *  DataTypeConversion: '<S1>/Data Type Conversion'
   *  Lookup_n-D: '<S1>/1-D Lookup Table'
   */
  Set_FUEL_deg_InjAdvance(look1_is16lu16n16Ds32_binlcas((int16_T)
    Get_FUEL_RPM_EngSpeed(), C_Brkpt_EngSpeed_InjAdv, C_Vet_deg_InjectionAdvance,
    65U));
}

/* Model initialize function */
void FUEL_FuelControl_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));
}

/* Model terminate function */
void FUEL_FuelControl_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
