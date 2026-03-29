/*
 * File: SetPoint_TBI.c
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

#include "SetPoint_TBI.h"

/* Named constants for Chart: '<S6>/Idle_Speed' */
#define SPTBIC_DisturbSetpoint         (0.0)
#define SPTBIC_LowTempSetpoint         (0.0)
#define SPTBIC_MediumTempSetpoint      (0.0)
#define SPTBIC_WorkTempSetpoint        (0.0)
#define SPTBIIN_Disturb_State          ((uint8_T)1U)
#define SPTBIIN_Low_Temp               ((uint8_T)2U)
#define SPTBIIN_Medium_Temp            ((uint8_T)3U)
#define SPTBIIN_Work_Temp              ((uint8_T)4U)
#define SPTBIcCrank_Time               (0.0)

/* Block signals and states (auto storage) */
SPTBIDW SPTBIrtDW;

/* Real-time model */
SPTBIRT_MODEL SPTBIrtM_;
SPTBIRT_MODEL *const SPTBIrtM = &SPTBIrtM_;

/* Model step function */
void SetPoint_TBI_step(void)
{
  real_T rtb_Switch2;

  /* Outputs for Enabled SubSystem: '<S2>/IdleSpeed' incorporates:
   *  EnablePort: '<S6>/Enable'
   */
  /* RelationalOperator: '<S2>/Relational Operator' incorporates:
   *  Constant: '<S2>/Constant6'
   *  DataStoreRead: '<S1>/Data Store Read3'
   *  Switch: '<S1>/Switch3'
   */
  if (Get_SPTBI_RPM() > 5) {
    /* Chart: '<S6>/Idle_Speed' incorporates:
     *  DataStoreRead: '<S1>/Data Store Read2'
     *  Switch: '<S1>/Switch2'
     */
    /* Gateway: SetPoint_TBI/IdleSpeed/Idle_Speed */
    /* During: SetPoint_TBI/IdleSpeed/Idle_Speed */
    if (SPTBIrtDW.SPTBIis_active_c1_SetPoint_TBI == 0U) {
      /* Entry: SetPoint_TBI/IdleSpeed/Idle_Speed */
      SPTBIrtDW.SPTBIis_active_c1_SetPoint_TBI = 1U;

      /* Entry Internal: SetPoint_TBI/IdleSpeed/Idle_Speed */
      /* Transition: '<S9>:30' */
      SPTBIrtDW.SPTBIis_c1_SetPoint_TBI = SPTBIIN_Disturb_State;

      /* Entry 'Disturb_State': '<S9>:42' */
      /* '<S9>:42:1' ref_vb = C_DisturbSetpoint; */
      SPTBIrtDW.SPTBIref_vb = SPTBIC_DisturbSetpoint;

      /* 37; */
      /* '<S9>:42:3' cont_partida = 0; */
      SPTBIrtDW.SPTBIcont_partida = 0.0;
    } else {
      switch (SPTBIrtDW.SPTBIis_c1_SetPoint_TBI) {
       case SPTBIIN_Disturb_State:
        /* During 'Disturb_State': '<S9>:42' */
        /* '<S9>:32:1' sf_internal_predicateOutput = ... */
        /* '<S9>:32:1' cont_partida > cCrank_Time && T_Water < 40; */
        if ((SPTBIrtDW.SPTBIcont_partida > SPTBIcCrank_Time) &&
            (Get_SPTBI_T_Water() < 40.0)) {
          /* Transition: '<S9>:32' */
          SPTBIrtDW.SPTBIis_c1_SetPoint_TBI = SPTBIIN_Low_Temp;

          /* Entry 'Low_Temp': '<S9>:39' */
          /* '<S9>:39:1' ref_vb = C_LowTempSetpoint; */
          SPTBIrtDW.SPTBIref_vb = SPTBIC_LowTempSetpoint;

          /* 36; */
        } else {
          /* '<S9>:31:1' sf_internal_predicateOutput = ... */
          /* '<S9>:31:1' cont_partida > cCrank_Time && T_Water >= 40 && T_Water < 50; */
          if ((SPTBIrtDW.SPTBIcont_partida > SPTBIcCrank_Time) &&
              (Get_SPTBI_T_Water() >= 40.0) && (Get_SPTBI_T_Water() < 50.0)) {
            /* Transition: '<S9>:31' */
            SPTBIrtDW.SPTBIis_c1_SetPoint_TBI = SPTBIIN_Medium_Temp;

            /* Entry 'Medium_Temp': '<S9>:36' */
            /* '<S9>:36:1' ref_vb =C_MediumTempSetpoint; */
            SPTBIrtDW.SPTBIref_vb = SPTBIC_MediumTempSetpoint;

            /*  35; */
          } else {
            /* '<S9>:29:1' sf_internal_predicateOutput = ... */
            /* '<S9>:29:1' cont_partida > cCrank_Time && T_Water >= 50; */
            if ((SPTBIrtDW.SPTBIcont_partida > SPTBIcCrank_Time) &&
                (Get_SPTBI_T_Water() >= 50.0)) {
              /* Transition: '<S9>:29' */
              SPTBIrtDW.SPTBIis_c1_SetPoint_TBI = SPTBIIN_Work_Temp;

              /* Entry 'Work_Temp': '<S9>:40' */
              /* '<S9>:40:1' ref_vb = C_WorkTempSetpoint; */
              SPTBIrtDW.SPTBIref_vb = SPTBIC_WorkTempSetpoint;

              /* 34; */
            } else {
              /* '<S9>:42:3' cont_partida = cont_partida+1; */
              SPTBIrtDW.SPTBIcont_partida++;
            }
          }
        }
        break;

       case SPTBIIN_Low_Temp:
        /* During 'Low_Temp': '<S9>:39' */
        /* '<S9>:43:1' sf_internal_predicateOutput = ... */
        /* '<S9>:43:1' rot < cTreshold_RPM; */
        /* '<S9>:37:1' sf_internal_predicateOutput = ... */
        /* '<S9>:37:1' T_Water >= 40 && T_Water < 50; */
        if ((Get_SPTBI_T_Water() >= 40.0) && (Get_SPTBI_T_Water() < 50.0)) {
          /* Transition: '<S9>:37' */
          SPTBIrtDW.SPTBIis_c1_SetPoint_TBI = SPTBIIN_Medium_Temp;

          /* Entry 'Medium_Temp': '<S9>:36' */
          /* '<S9>:36:1' ref_vb =C_MediumTempSetpoint; */
          SPTBIrtDW.SPTBIref_vb = SPTBIC_MediumTempSetpoint;

          /*  35; */
        } else {
          /* '<S9>:38:1' sf_internal_predicateOutput = ... */
          /* '<S9>:38:1' T_Water >= 50; */
          if (Get_SPTBI_T_Water() >= 50.0) {
            /* Transition: '<S9>:38' */
            SPTBIrtDW.SPTBIis_c1_SetPoint_TBI = SPTBIIN_Work_Temp;

            /* Entry 'Work_Temp': '<S9>:40' */
            /* '<S9>:40:1' ref_vb = C_WorkTempSetpoint; */
            SPTBIrtDW.SPTBIref_vb = SPTBIC_WorkTempSetpoint;

            /* 34; */
          }
        }
        break;

       case SPTBIIN_Medium_Temp:
        /* During 'Medium_Temp': '<S9>:36' */
        /* '<S9>:44:1' sf_internal_predicateOutput = ... */
        /* '<S9>:44:1' rot < cTreshold_RPM; */
        /* '<S9>:34:1' sf_internal_predicateOutput = ... */
        /* '<S9>:34:1' T_Water >= 50; */
        if (Get_SPTBI_T_Water() >= 50.0) {
          /* Transition: '<S9>:34' */
          SPTBIrtDW.SPTBIis_c1_SetPoint_TBI = SPTBIIN_Work_Temp;

          /* Entry 'Work_Temp': '<S9>:40' */
          /* '<S9>:40:1' ref_vb = C_WorkTempSetpoint; */
          SPTBIrtDW.SPTBIref_vb = SPTBIC_WorkTempSetpoint;

          /* 34; */
        } else {
          /* '<S9>:35:1' sf_internal_predicateOutput = ... */
          /* '<S9>:35:1' T_Water < 40; */
          if (Get_SPTBI_T_Water() < 40.0) {
            /* Transition: '<S9>:35' */
            SPTBIrtDW.SPTBIis_c1_SetPoint_TBI = SPTBIIN_Low_Temp;

            /* Entry 'Low_Temp': '<S9>:39' */
            /* '<S9>:39:1' ref_vb = C_LowTempSetpoint; */
            SPTBIrtDW.SPTBIref_vb = SPTBIC_LowTempSetpoint;

            /* 36; */
          }
        }
        break;

       default:
        /* During 'Work_Temp': '<S9>:40' */
        /* '<S9>:45:1' sf_internal_predicateOutput = ... */
        /* '<S9>:45:1' rot < cTreshold_RPM; */
        /* '<S9>:33:1' sf_internal_predicateOutput = ... */
        /* '<S9>:33:1' T_Water < 40; */
        if (Get_SPTBI_T_Water() < 40.0) {
          /* Transition: '<S9>:33' */
          SPTBIrtDW.SPTBIis_c1_SetPoint_TBI = SPTBIIN_Low_Temp;

          /* Entry 'Low_Temp': '<S9>:39' */
          /* '<S9>:39:1' ref_vb = C_LowTempSetpoint; */
          SPTBIrtDW.SPTBIref_vb = SPTBIC_LowTempSetpoint;

          /* 36; */
        } else {
          /* '<S9>:41:1' sf_internal_predicateOutput = ... */
          /* '<S9>:41:1' T_Water >= 40 && T_Water < 50; */
          if (Get_SPTBI_T_Water() < 50.0) {
            /* Transition: '<S9>:41' */
            SPTBIrtDW.SPTBIis_c1_SetPoint_TBI = SPTBIIN_Medium_Temp;

            /* Entry 'Medium_Temp': '<S9>:36' */
            /* '<S9>:36:1' ref_vb =C_MediumTempSetpoint; */
            SPTBIrtDW.SPTBIref_vb = SPTBIC_MediumTempSetpoint;

            /*  35; */
          }
        }
        break;
      }
    }

    /* End of Chart: '<S6>/Idle_Speed' */
  }

  /* End of RelationalOperator: '<S2>/Relational Operator' */
  /* End of Outputs for SubSystem: '<S2>/IdleSpeed' */

  /* Switch: '<S1>/Switch' incorporates:
   *  DataStoreRead: '<S1>/Data Store Read'
   */
  rtb_Switch2 = Get_SPTBI_IgnitionOn();

  /* Switch: '<S2>/Switch' incorporates:
   *  DataStoreRead: '<S1>/Data Store Read'
   *  Switch: '<S1>/Switch'
   */
  if (Get_SPTBI_IgnitionOn() > 0.0) {
    /* Product: '<S5>/Divide' incorporates:
     *  Constant: '<S5>/Constant'
     *  DataStoreRead: '<S1>/Data Store Read1'
     */
    rtb_Switch2 = (real_T)Get_SPTBI_p_Pedal() / 10.0;

    /* Switch: '<S2>/Switch1' */
    if (rtb_Switch2 > 2.0) {
      /* Product: '<S7>/Product3' incorporates:
       *  Constant: '<S7>/Constant6'
       *  Constant: '<S7>/Constant9'
       *  Product: '<S7>/Product1'
       */
      rtb_Switch2 = rtb_Switch2 * 0.01 * 90.0;

      /* Switch: '<S10>/Switch2' incorporates:
       *  Constant: '<S7>/Constant10'
       *  Constant: '<S7>/Constant11'
       *  RelationalOperator: '<S10>/LowerRelop1'
       *  RelationalOperator: '<S10>/UpperRelop'
       *  Switch: '<S10>/Switch'
       */
      if (rtb_Switch2 > 85.0) {
        rtb_Switch2 = 85.0;
      } else {
        if (rtb_Switch2 < 6.0) {
          /* Switch: '<S10>/Switch' incorporates:
           *  Constant: '<S7>/Constant11'
           */
          rtb_Switch2 = 6.0;
        }
      }

      /* End of Switch: '<S10>/Switch2' */
    } else {
      rtb_Switch2 = SPTBIrtDW.SPTBIref_vb;
    }

    /* End of Switch: '<S2>/Switch1' */
  }

  /* End of Switch: '<S2>/Switch' */

  /* Product: '<S8>/Product' incorporates:
   *  Constant: '<S8>/Constant1'
   */
  rtb_Switch2 *= 10.0;

  /* DataStoreWrite: '<Root>/Data Store Write' incorporates:
   *  DataTypeConversion: '<S8>/Data Type Conversion'
   */
  Set_SPTBI_deg_Setpoint((uint16_T)rtb_Switch2);
}

/* Model initialize function */
void SetPoint_TBI_initialize(void)
{
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
