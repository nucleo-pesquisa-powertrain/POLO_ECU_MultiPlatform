#ifndef _CDD_CRANKSHAFT_H
#define _CDD_CRANKSHAFT_H

#include "Platform_Types.h"

void CDD_CrankshaftPosition_Init(void);
void CDD_TriggerWheel_Event(void);
uint32 CDD_Get_LastToothTime(void);
unsigned char CDD_Get_CurrentTooth(void);
uint32 CDD_Get_EngineSpeed_RAW(void);
unsigned long CDD_Get_CylinderEvents(void);
unsigned long CDD_Get_LastToothTime_us_Filtered(void);

#endif
