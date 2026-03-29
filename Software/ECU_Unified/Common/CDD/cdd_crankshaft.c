#include "cdd_crankshaft.h"
#include "Icu.h"
#include "Dio.h"
#include "Platform_Types.h"
#include "filters.h"

uint32 S_EngineSpeed_rpm;
uint32 S_LastToothTime_us;
 unsigned char S_CurrentTooth;
 unsigned long S_CylinderEvents;
 unsigned long int S_LastFailTime_us;

/* Timestamp do último dente, em microssegundos (retornado por Icu_GetTimestamp_us) */
static uint32 S_PreviousTime_us = 0;

unsigned long S_LastToothTime_us_Filtered;

void CDD_CrankshaftPosition_Init(void)
{
    Icu_Init();
    S_LastToothTime_us = 0;
    S_CurrentTooth = 0;
    S_CylinderEvents = 0;
    S_EngineSpeed_rpm = 0;
    S_PreviousTime_us = Icu_GetTimestamp_us();
}

// Variáveis para armazenar o menor e maior tempo de dente
volatile unsigned long int min_ToothTime = 0xFFFF; // Inicializado com o maior valor possível
volatile unsigned long int max_ToothTime = 0;      // Inicializado com o menor valor possível

void CDD_TriggerWheel_Event(void)
{
    static unsigned long int L_PreviousToothTime_us;
    unsigned long int L_CurrentToothTime_us;

    /* Obtém timestamp atual em us e calcula o tempo desde o último dente.
     * A subtração sem sinal trata corretamente o overflow do timer. */
    uint32 S_CurrentTime_us = Icu_GetTimestamp_us();
    L_CurrentToothTime_us = S_CurrentTime_us - S_PreviousTime_us;
    S_PreviousTime_us = S_CurrentTime_us;

    /*Just for test*/
    if (L_CurrentToothTime_us < min_ToothTime)
    {
        min_ToothTime = L_CurrentToothTime_us;
    }
    if (L_CurrentToothTime_us > max_ToothTime)
    {
        max_ToothTime = L_CurrentToothTime_us;
    }
    /*Just for test*/

    if(L_CurrentToothTime_us >= ((unsigned long)L_PreviousToothTime_us*5/3))
    {
        S_CurrentTooth = 1;
        S_CylinderEvents++;
        S_LastFailTime_us = L_CurrentToothTime_us;
    }
    else
    {
        S_CurrentTooth++;
        S_LastToothTime_us = L_CurrentToothTime_us;
    }

    S_LastToothTime_us_Filtered = first_order_filter(S_LastToothTime_us, S_LastToothTime_us_Filtered, 0.4);
    L_PreviousToothTime_us = L_CurrentToothTime_us;

    Dio_ToggleChannel(DIO_CH_LED3);  /* LD3: toggle a cada dente do crankshaft */

}

#define MAX_TOOTH_TIMEOUT_US 200000  // se ficou 200ms sem pulso, rotação deve ser 0.

uint32 CDD_Get_EngineSpeed_RAW(void)
{
    /* Calcula tempo decorrido desde o último dente sem conversão de frequência */
    uint32 timeSinceLastTooth_us = Icu_GetTimestamp_us() - S_PreviousTime_us;

    if(timeSinceLastTooth_us > MAX_TOOTH_TIMEOUT_US)
    {
        S_EngineSpeed_rpm = 0;
    }
    else if(S_CurrentTooth > 2 && S_LastToothTime_us != 0)
    {
        S_EngineSpeed_rpm = (uint32)(1000000 / S_LastToothTime_us);
    }

    return S_EngineSpeed_rpm;
}

unsigned long CDD_Get_LastToothTime_us_Filtered(void)
{
    return S_LastToothTime_us_Filtered;
}

uint32 CDD_Get_LastToothTime(void)
{
    return (uint32) S_LastToothTime_us;
}

unsigned char CDD_Get_CurrentTooth(void)
{
    return S_CurrentTooth;
}
unsigned long CDD_Get_CylinderEvents(void)
{
    return S_CylinderEvents;
}
