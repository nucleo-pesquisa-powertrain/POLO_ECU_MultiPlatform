#ifndef _CDD_SYNCHRONISM_H
#define _CDD_SYNCHRONISM_H

#define TESTADVANCES_ENABLED FALSE

void CDD_SYNC_Timing_Event(void);
void CDD_SYNC_StartIgnition_Event(void);
void CDD_SYNC_StartInjection_Event(void); //Triggered by Timer4 Callback

#if (TESTADVANCES_ENABLED == TRUE)
void CDD_SYNC_TestAdvancesSweepTask(void);
#endif

#endif
