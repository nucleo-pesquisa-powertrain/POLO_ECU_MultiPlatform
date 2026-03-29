/*
 * File: SPK_SparkAdvance.c
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

#include "SPK_SparkAdvance.h"
#include "SPK_SparkAdvance_private.h"

/* Exported block parameters */
int32_T C_Tbl_deg_SparkAdvance[660] = { -2000, 150, 150, 150, 150, 150, 150, 150,
  150, 150, 27, 41, 67, 97, 129, 157, 179, 194, 203, 208, 150, 157, 164, 172,
  179, 185, 193, 199, 206, 213, 220, 227, 235, 242, 250, 257, 264, 271, 278, 284,
  290, 295, 300, 304, 308, 310, 312, 313, 313, 312, 310, 310, 310, 310, 310, 310,
  310, 310, 310, 310, 310, 310, 310, 310, 310, 310, -2000, 150, 150, 150, 150,
  150, 150, 150, 150, 150, 27, 42, 67, 98, 130, 157, 178, 190, 197, 200, 150,
  157, 163, 170, 176, 181, 187, 193, 199, 204, 210, 216, 222, 229, 235, 241, 247,
  253, 259, 265, 270, 275, 279, 283, 286, 289, 290, 292, 292, 292, 290, 290, 290,
  290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, -2000, 150,
  150, 150, 150, 150, 150, 150, 150, 150, 27, 40, 61, 88, 116, 142, 164, 180,
  191, 197, 150, 154, 158, 163, 168, 173, 179, 184, 190, 194, 200, 205, 209, 214,
  218, 222, 226, 230, 233, 236, 240, 243, 247, 250, 253, 256, 258, 261, 264, 267,
  270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270, 270,
  -2000, 150, 150, 150, 150, 150, 150, 150, 150, 150, 30, 43, 64, 90, 118, 142,
  162, 176, 187, 192, 150, 154, 157, 161, 166, 170, 175, 179, 183, 187, 190, 193,
  196, 198, 200, 202, 204, 205, 207, 208, 210, 212, 214, 215, 218, 220, 223, 227,
  230, 235, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
  240, 240, -2000, 150, 150, 150, 150, 150, 150, 150, 150, 150, 30, 43, 64, 90,
  118, 142, 162, 176, 186, 192, 150, 152, 153, 154, 155, 157, 158, 158, 160, 161,
  162, 163, 165, 167, 169, 170, 172, 174, 176, 178, 180, 182, 184, 185, 187, 188,
  190, 190, 191, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
  192, 192, 192, 192, -2000, 150, 150, 150, 150, 150, 150, 150, 150, 150, 30, 40,
  58, 80, 104, 127, 148, 165, 178, 188, 150, 152, 153, 154, 155, 157, 158, 158,
  160, 161, 162, 163, 165, 167, 169, 170, 172, 174, 176, 178, 180, 182, 184, 185,
  187, 188, 190, 190, 191, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
  192, 192, 192, 192, 192, 192, -2000, 150, 150, 150, 150, 150, 150, 150, 150,
  150, 30, 40, 58, 80, 104, 127, 148, 165, 179, 188, 150, 152, 153, 154, 155,
  157, 158, 158, 160, 161, 162, 163, 165, 167, 169, 170, 172, 174, 176, 178, 180,
  182, 184, 185, 187, 188, 190, 190, 191, 192, 192, 192, 192, 192, 192, 192, 192,
  192, 192, 192, 192, 192, 192, 192, 192, 192, -2000, 150, 150, 150, 150, 150,
  150, 150, 150, 150, 30, 42, 61, 85, 109, 127, 139, 145, 147, 148, 150, 152,
  153, 154, 155, 157, 158, 158, 160, 161, 162, 163, 165, 167, 169, 170, 172, 174,
  176, 178, 180, 182, 184, 185, 187, 188, 190, 190, 191, 192, 192, 192, 192, 192,
  192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, -2000, 150, 150,
  150, 150, 150, 150, 150, 150, 150, 30, 40, 58, 80, 101, 120, 133, 140, 145,
  147, 140, 143, 146, 149, 151, 154, 155, 157, 159, 160, 162, 164, 166, 167, 169,
  171, 173, 175, 176, 178, 180, 182, 184, 185, 186, 188, 189, 190, 191, 191, 192,
  192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
  -2000, 150, 150, 150, 150, 150, 150, 150, 150, 150, 30, 40, 58, 80, 101, 120,
  133, 140, 145, 147, 130, 134, 139, 142, 146, 149, 152, 155, 157, 160, 162, 164,
  166, 169, 170, 172, 174, 175, 177, 179, 180, 181, 183, 184, 185, 187, 188, 189,
  190, 191, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
  192, 192 } ;                         /* Variable: C_Tbl_deg_SparkAdvance
                                        * Referenced by: '<Root>/2-D Lookup E0'
                                        */

int32_T C_Tbl_deg_SparkAdvanceE100[660] = { -2000, 150, 150, 150, 150, 150, 150,
  150, 150, 150, 190, 210, 220, 220, 220, 210, 210, 210, 210, 220, 220, 220, 220,
  210, 200, 190, 190, 190, 200, 210, 210, 220, 220, 220, 220, 220, 220, 220, 220,
  220, 220, 220, 220, 210, 200, 200, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 180, 170, 160, -2000, 150, 150, 150,
  150, 150, 150, 150, 150, 150, 180, 200, 200, 200, 200, 200, 200, 200, 210, 210,
  220, 230, 230, 230, 230, 230, 230, 230, 230, 240, 240, 240, 240, 230, 230, 220,
  220, 220, 220, 220, 220, 220, 220, 210, 200, 200, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 180, 170, 160, -2000,
  150, 150, 150, 150, 150, 150, 150, 150, 150, 180, 190, 190, 190, 180, 170, 170,
  190, 220, 250, 280, 300, 320, 330, 330, 330, 340, 350, 360, 380, 390, 390, 390,
  390, 380, 380, 380, 380, 390, 400, 390, 380, 350, 310, 260, 220, 190, 170, 170,
  170, 180, 190, 190, 200, 190, 190, 190, 190, 190, 190, 190, 190, 190, 180, 170,
  160, -2000, 150, 150, 150, 150, 150, 150, 150, 150, 150, 170, 170, 170, 170,
  170, 180, 190, 210, 230, 260, 290, 310, 330, 340, 350, 360, 360, 350, 350, 340,
  340, 330, 330, 330, 340, 350, 360, 380, 390, 400, 400, 390, 350, 310, 270, 220,
  190, 170, 170, 170, 180, 190, 190, 200, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 180, 170, 160, -2000, 150, 150, 150, 150, 150, 150, 150, 150, 150, 170,
  170, 170, 160, 160, 160, 170, 190, 210, 240, 260, 290, 310, 330, 350, 360, 360,
  360, 350, 340, 330, 330, 330, 340, 350, 360, 380, 380, 390, 380, 370, 350, 320,
  290, 250, 220, 190, 180, 170, 180, 180, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 180, 170, 160, -2000, 150, 150, 150, 150, 150, 150, 150,
  150, 150, 170, 170, 170, 160, 160, 160, 170, 190, 210, 230, 260, 280, 300, 310,
  320, 320, 330, 330, 330, 330, 330, 330, 330, 330, 340, 340, 350, 350, 350, 350,
  350, 330, 310, 270, 240, 210, 190, 180, 180, 180, 180, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 180, 170, 160, -2000, 150, 150, 150, 150,
  150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 160, 170, 190, 210, 230, 250,
  270, 290, 300, 300, 310, 320, 320, 320, 330, 330, 330, 330, 330, 330, 320, 320,
  320, 330, 330, 320, 320, 300, 280, 250, 230, 210, 200, 200, 200, 200, 200, 200,
  200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 190, 180, 160, -2000, 150,
  150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 140, 140, 140, 150, 170,
  190, 220, 250, 270, 280, 290, 300, 300, 310, 310, 320, 320, 330, 330, 330, 320,
  320, 320, 320, 320, 320, 320, 320, 310, 290, 270, 240, 220, 200, 190, 190, 190,
  200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 190, 180, 160,
  -2000, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 140, 120, 100,
  80, 70, 70, 70, 80, 100, 120, 140, 160, 180, 190, 210, 220, 230, 240, 240, 230,
  210, 190, 170, 140, 130, 130, 130, 140, 160, 170, 180, 190, 190, 200, 200, 200,
  200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 190,
  180, 160, -2000, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 140,
  120, 90, 80, 70, 80, 100, 130, 150, 170, 180, 180, 170, 170, 170, 180, 190,
  210, 220, 220, 210, 190, 170, 140, 130, 130, 130, 140, 160, 170, 180, 180, 180,
  180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180,
  180, 180, 170, 160, 150 } ;          /* Variable: C_Tbl_deg_SparkAdvanceE100
                                        * Referenced by: '<Root>/2-D Lookup E100'
                                        */

/* Block signals and states (auto storage) */
D_Work rtDWork;
int32_T look2_iu16bs16ls16ts32_aVKf8oUS(uint16_T u0, uint16_T u1, const int16_T
  bp0[], const int16_T bp1[], const int32_T table[], const uint32_T maxIndex[],
  uint32_T stride)
{
  int32_T y;
  uint32_T bpIndices[2];
  int16_T fractions[2];
  int16_T uCast;
  int32_T yR_1d;
  uint32_T iRght;
  uint32_T bpIdx;
  uint32_T iLeft;
  uint16_T tmp;

  /* Lookup 2-D
     Canonical function name: look2_iu16bs16ls16ts32Du32du32_binlcase
     Search method: 'binary'
     Use previous index: 'off'
     Interpolation method: 'Linear'
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
  tmp = u0;
  if (u0 > 32767) {
    tmp = 32767U;
  }

  uCast = (int16_T)tmp;
  if (u0 < bp0[0U]) {
    iLeft = 0U;
    uCast = 0;
  } else if (uCast < bp0[maxIndex[0U]]) {
    /* Binary Search */
    bpIdx = maxIndex[0U] >> 1U;
    iLeft = 0U;
    iRght = maxIndex[0U];
    while (iRght - iLeft > 1U) {
      if (uCast < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    uCast = (int16_T)(((uint32_T)u0 - bp0[iLeft]) / (uint16_T)(bp0[iLeft + 1U] -
      bp0[iLeft]));
  } else {
    iLeft = maxIndex[0U];
    uCast = 0;
  }

  fractions[0U] = uCast;
  bpIndices[0U] = iLeft;

  /* Prelookup - Index and Fraction
     Index Search method: 'binary'
     Extrapolation method: 'Clip'
     Use previous index: 'off'
     Use last breakpoint for index at or above upper limit: 'on'
     Remove protection against out-of-range input in generated code: 'off'
     Rounding mode: 'simplest'
   */
  tmp = u1;
  if (u1 > 32767) {
    tmp = 32767U;
  }

  uCast = (int16_T)tmp;
  if (u1 < bp1[0U]) {
    iLeft = 0U;
    uCast = 0;
  } else if (uCast < bp1[maxIndex[1U]]) {
    /* Binary Search */
    bpIdx = maxIndex[1U] >> 1U;
    iLeft = 0U;
    iRght = maxIndex[1U];
    while (iRght - iLeft > 1U) {
      if (uCast < bp1[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    uCast = (int16_T)(((uint32_T)u1 - bp1[iLeft]) / (uint16_T)(bp1[iLeft + 1U] -
      bp1[iLeft]));
  } else {
    iLeft = maxIndex[1U];
    uCast = 0;
  }

  /* Interpolation 2-D
     Interpolation method: 'Linear'
     Use last breakpoint for index at or above upper limit: 'on'
     Overflow mode: 'wrapping'
   */
  bpIdx = iLeft * stride + bpIndices[0U];
  if (bpIndices[0U] == maxIndex[0U]) {
    y = table[bpIdx];
  } else if (table[bpIdx + 1U] >= table[bpIdx]) {
    y = (int32_T)((uint32_T)table[bpIdx + 1U] - table[bpIdx]) * fractions[0U] +
      table[bpIdx];
  } else {
    y = table[bpIdx] - (int32_T)((uint32_T)table[bpIdx] - table[bpIdx + 1U]) *
      fractions[0U];
  }

  if (iLeft == maxIndex[1U]) {
  } else {
    bpIdx += stride;
    if (bpIndices[0U] == maxIndex[0U]) {
      yR_1d = table[bpIdx];
    } else if (table[bpIdx + 1U] >= table[bpIdx]) {
      yR_1d = (int32_T)((uint32_T)table[bpIdx + 1U] - table[bpIdx]) * fractions
        [0U] + table[bpIdx];
    } else {
      yR_1d = table[bpIdx] - (int32_T)((uint32_T)table[bpIdx] - table[bpIdx + 1U])
        * fractions[0U];
    }

    if (yR_1d >= y) {
      y += (int32_T)((uint32_T)yR_1d - y) * uCast;
    } else {
      y -= (int32_T)((uint32_T)y - yR_1d) * uCast;
    }
  }

  return y;
}

int32_T div_nzp_s32_floor(int32_T numerator, int32_T denominator)
{
  return (((numerator < 0) != (denominator < 0)) && (numerator % denominator !=
           0) ? -1 : 0) + numerator / denominator;
}

/* Model step function */
void SPK_SparkAdvance_step(void)
{
  uint16_T rtb_Sum;
  real32_T rtb_K;
  int32_T rtb_uDLookupE0;
  int32_T rtb_Product_i;
  real32_T rtb_K_0;

  /* Sum: '<S1>/Sum' incorporates:
   *  Constant: '<Root>/Constant2'
   *  DataStoreRead: '<Root>/Data Store Read1'
   */
  rtb_Sum = (uint16_T)(1000 - Get16u_SPARK_RPM_EngineSpeed());

  /* Switch: '<S1>/Switch' incorporates:
   *  Constant: '<S1>/Constant'
   *  Constant: '<S1>/Constant2'
   */
  if (rtb_Sum > 60) {
    rtb_K = 0.44546F;
  } else {
    rtb_K = 0.24546F;
  }

  /* End of Switch: '<S1>/Switch' */

  /* Sum: '<S1>/Sum1' incorporates:
   *  Constant: '<S1>/Constant4'
   *  DataTypeConversion: '<S1>/Data Type Conversion'
   *  Delay: '<S1>/Delay'
   *  Delay: '<S1>/Delay1'
   *  Product: '<S1>/Product'
   *  Product: '<S1>/Product1'
   */
  rtb_K = ((real32_T)rtb_Sum * rtb_K - rtb_K * 0.8921F * rtDWork.Delay_DSTATE) +
    rtDWork.Delay1_DSTATE;

  /* Switch: '<Root>/Switch' incorporates:
   *  Constant: '<Root>/Constant4'
   *  DataStoreRead: '<Root>/Data Store Read6'
   *  RelationalOperator: '<Root>/Relational Operator'
   */
  if (Get16u_SPARK_s_ManagementState() == 0) {
    /* Switch: '<S1>/Switch' incorporates:
     *  Constant: '<S1>/Constant1'
     *  Constant: '<S1>/Constant3'
     */
    if (rtb_Sum > 60) {
      rtb_uDLookupE0 = 35;
    } else {
      rtb_uDLookupE0 = 21;
    }

    /* End of Switch: '<S1>/Switch' */

    /* Switch: '<S2>/Switch2' incorporates:
     *  Constant: '<S1>/Constant6'
     *  RelationalOperator: '<S2>/LowerRelop1'
     *  RelationalOperator: '<S2>/UpperRelop'
     *  Switch: '<S2>/Switch'
     */
    if (rtb_K > rtb_uDLookupE0) {
      rtb_K_0 = (real32_T)rtb_uDLookupE0;
    } else if (rtb_K < 0.0F) {
      /* Switch: '<S2>/Switch' incorporates:
       *  Constant: '<S1>/Constant6'
       */
      rtb_K_0 = 0.0F;
    } else {
      rtb_K_0 = rtb_K;
    }

    /* End of Switch: '<S2>/Switch2' */

    /* DataStoreWrite: '<Root>/Data Store Write' incorporates:
     *  Constant: '<S1>/Constant5'
     *  DataTypeConversion: '<S1>/Data Type Conversion1'
     *  Product: '<S1>/Product2'
     */
    Set16s_SPARK_SparkAdvance((int16_T)(real32_T)floor(rtb_K_0 * 10.0F));
  } else {
    /* Lookup_n-D: '<Root>/2-D Lookup E100' incorporates:
     *  DataStoreRead: '<Root>/Data Store Read2'
     *  DataStoreRead: '<Root>/Data Store Read3'
     */
    rtb_uDLookupE0 = look2_iu16bs16ls16ts32_aVKf8oUS
      (Get16u_SPARK_RPM_EngineSpeed(), Get16u_SPARK_P_AirPress(),
       rtConstP.pooled4, rtConstP.pooled5, C_Tbl_deg_SparkAdvanceE100,
       rtConstP.pooled2, 66U);

    /* Product: '<Root>/Product' incorporates:
     *  DataStoreRead: '<Root>/Data Store Read4'
     */
    rtb_Product_i = rtb_uDLookupE0 * Get16u_SPARK_p_EthanolPct();

    /* Lookup_n-D: '<Root>/2-D Lookup E0' incorporates:
     *  DataStoreRead: '<Root>/Data Store Read'
     *  DataStoreRead: '<Root>/Data Store Read1'
     */
    rtb_uDLookupE0 = look2_iu16bs16ls16ts32_aVKf8oUS
      (Get16u_SPARK_RPM_EngineSpeed(), Get16u_SPARK_P_AirPress(),
       rtConstP.pooled4, rtConstP.pooled5, C_Tbl_deg_SparkAdvance,
       rtConstP.pooled2, 66U);

    /* DataStoreWrite: '<Root>/Data Store Write' incorporates:
     *  Constant: '<Root>/Constant'
     *  Constant: '<Root>/Constant1'
     *  DataStoreRead: '<Root>/Data Store Read5'
     *  Product: '<Root>/Divide'
     *  Product: '<Root>/Product1'
     *  Sum: '<Root>/Add'
     *  Sum: '<Root>/Add1'
     */
    Set16s_SPARK_SparkAdvance((int16_T)div_nzp_s32_floor((int32_T)(1000U -
      Get16u_SPARK_p_EthanolPct()) * rtb_uDLookupE0 + rtb_Product_i, 1000));
  }

  /* End of Switch: '<Root>/Switch' */

  /* Update for Delay: '<S1>/Delay' incorporates:
   *  DataTypeConversion: '<S1>/Data Type Conversion'
   */
  rtDWork.Delay_DSTATE = rtb_Sum;

  /* Update for Delay: '<S1>/Delay1' */
  rtDWork.Delay1_DSTATE = rtb_K;
}

/* Model initialize function */
void SPK_SparkAdvance_initialize(void)
{
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
