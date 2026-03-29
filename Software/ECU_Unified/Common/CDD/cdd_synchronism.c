#include "cdd_synchronism.h"
#include "cdd_crankshaft.h"
#include "cdd_spark.h"
#include "cdd_injectors.h"
#include "Gpt.h"
#include "Dio.h"
#include "rte_components.h"
#include "rte_environment.h"
#include "ECU_State_interface.h"

#define PMS_CYL1_TOOTH  14
#define MIN_ENG_SPEED   50

            char de = 0;

const unsigned char S_CylinderStateFlow[4] = {1, 3, 4, 2};
volatile char count_test = 0;
volatile int count_test2 = 0;
volatile int count_test3 = 0;

unsigned long int S_us_InjectionTime;
long int S_deg_CurrFlywheelPos;
unsigned char S_CylSparkStateFlowIdx = 0;
unsigned char S_CylSparkStateFlowIdx_old = 0;
unsigned char S_b_SparkEnableASW = 1;

unsigned char S_CylInjStateFlowIdx = 2;
unsigned char S_CylInjStateFlowIdx_old = 2;
unsigned char S_b_InjectionEnableASW = 1;
unsigned long int S_us_InjectionTimeASW = 5000;
long int S_deg_InjectTimingASW = 0;//30; //3 graus

unsigned long count_falha_sinc = 0;
long int delta_falha = 0;

char enable_phase = 1;

void CDD_SYNC_SynchronizeCamshaft(unsigned char P_CurrentTooth);
void CDD_SYNC_SparkTiming(unsigned char P_CurrentTooth, long int P_us_ToothTime);
void CDD_SYNC_InjectTiming(unsigned char P_CurrentTooth, long int P_us_ToothTime);
void CDD_SYNC_CalculateTimeToStartIgn(long int P_deg_DegreesToStartIgn, unsigned long int P_us_LastToothTime, unsigned char P_CurrentTooth);
void CDD_SYNC_CalculateTimeToStartInj(long int P_deg_DegreesToStartInj, unsigned long int P_us_LastToothTime, unsigned char P_CurrentTooth);

volatile char test_valor = 58;
volatile unsigned long int min_Speed = 0;
volatile unsigned long int max_Speed = 0;
volatile unsigned long int min_Tooth = 0;
volatile unsigned long int max_Tooth = 0;

unsigned short int count_forcedIgn = 0, count_forcedInj = 0;




void CDD_SYNC_Timing_Event(void)    //Triggered by ERU Callback
{

	static unsigned char L_CurrentTooth;
	static uint32 L_rpm_EngineSpeed;
	static uint32 L_us_ToothTime;

    if(CDD_Get_CylinderEvents() < 2)
    {
        return;
    }

    count_test++;


    Dio_WriteChannel(DIO_CH_MC33810_ENOUT, DIO_LOW);

    L_CurrentTooth = CDD_Get_CurrentTooth();
    L_rpm_EngineSpeed =  CDD_Get_EngineSpeed_RAW();
    L_us_ToothTime = CDD_Get_LastToothTime_us_Filtered();

    if(L_CurrentTooth == 1)
    {
    	if(count_test != test_valor)
    	{
    		count_test2++;
    		count_test3 = count_test;
    	}
    	count_test = 0;
        return;
    }
    else if(L_CurrentTooth == 2)
    {
        S_deg_CurrFlywheelPos += 240; //grau/10, 4 dentes
        if(S_deg_CurrFlywheelPos > 3000)
        {

        }
    }
    else
    {
        S_deg_CurrFlywheelPos += 60;
    }

    if(count_test > test_valor)
	{
    	count_test3++;
	}

    CDD_SYNC_SynchronizeCamshaft(L_CurrentTooth);

    /*Just for test*/
    if (L_rpm_EngineSpeed < min_Speed)
    {
    	min_Speed = L_rpm_EngineSpeed;
    }
    if (L_rpm_EngineSpeed > max_Speed)
    {
    	max_Speed = L_rpm_EngineSpeed;
    }

    if (L_us_ToothTime < min_Tooth)
    {
    	min_Tooth = L_us_ToothTime;
    }
    if (L_us_ToothTime > max_Tooth)
    {
    	max_Tooth = L_us_ToothTime;
    }
    /*Just for test*/

//    if(L_rpm_EngineSpeed < MIN_ENG_SPEED)
//    {
//        return;
//    }

    CDD_SYNC_SparkTiming(L_CurrentTooth, L_us_ToothTime);
    CDD_SYNC_InjectTiming(L_CurrentTooth, L_us_ToothTime);
}
void CDD_SYNC_SynchronizeCamshaft(unsigned char P_CurrentTooth)
{
    static unsigned char L_PhaseLevel;
    static long int test_pos = 0;

    L_PhaseLevel = Dio_ReadChannel(DIO_CH_PHASE_STATE);

    if(P_CurrentTooth == PMS_CYL1_TOOTH && S_deg_CurrFlywheelPos >= 7200)
    {
        S_deg_CurrFlywheelPos = 0;
    }
    else if(P_CurrentTooth == 3) // Testado na ECU PIC, no dente 3 é possível saber a fase pelo nível do sensor de comando
    {
        test_pos = S_deg_CurrFlywheelPos;
		if (enable_phase)
        {
            if(L_PhaseLevel)
            {
                S_deg_CurrFlywheelPos = 2760 + (3 * 60);
            }
            else
            {
                S_deg_CurrFlywheelPos = 6360 + (3 * 60);
            }
        }
        if(test_pos != S_deg_CurrFlywheelPos)
        {
            count_falha_sinc++;
            delta_falha = S_deg_CurrFlywheelPos - test_pos;
        }
    }
}
void CDD_SYNC_SparkTiming(unsigned char P_CurrentTooth, long int P_us_ToothTime)
{
    static long int L_deg_DWELLTiming;
    static long int L_deg_TotalTiming;
    static long int L_deg_CurrFlywheelAngle;
    static long int L_deg_SparkTiming;
    static long int L_deg_SparkTimingPrev;
    static unsigned char L_b_SparkEnable;
    static long int L_deg_StartIgnitionAngle;
    static long int L_deg_DegreesToStartIgn;

    L_deg_SparkTiming = Get16s_RTE_deg_SPARKTiming();

    L_deg_CurrFlywheelAngle = S_deg_CurrFlywheelPos;

    L_deg_DWELLTiming = 60*DWELL_TIME/P_us_ToothTime; //Calculates Current DWELL Timing in degrees

    if(L_deg_SparkTiming > L_deg_SparkTimingPrev)
    {
        L_deg_SparkTiming = L_deg_SparkTimingPrev;
    }
    L_deg_SparkTimingPrev = L_deg_SparkTiming;

    L_deg_TotalTiming = L_deg_DWELLTiming + L_deg_SparkTiming; //Soma todos os fatores que contribuem para o adiantamento da alimentação nas bobinas


    L_b_SparkEnable = S_b_SparkEnableASW;

    /* Lógica para tentar resolver problema de ignição no limiar dos 5.9 ~6 graus*/
//    S_StartIgnitionForced = 0; //Essa variavel sempre deve ser setado false, exceto logo após o StartIgninition_Event ser chamado fora do Timer
//   if(S_TimerWaitingIgn)
//   {
//	   Gpt_StopTimer(GPT_CH_IGN_TIMING);
//       CDD_SYNC_StartIgnition_Event(); // Força o StartIgnition antes de continuar com a lógica de Ignition em caso de S_TimerWaitingIgn
//       S_StartIgnitionForced = 1;      // Toda vez que o Timer para o StartIgnition é setado, a função de StartIgnition_Event deve obrigatoriamente
//       count_forcedIgn++;
//   }                                   // rodar antes de iniciar a lógica de ignição do próximo dente

    if(!L_b_SparkEnable)        //Spark desabilitado pelo Application Software
    {
        return;
    }
    switch(S_CylinderStateFlow[S_CylSparkStateFlowIdx])     //StateFlow para calcular o ângulo exato em que deverá ser ligado a bobina de acordo com o próximo cilindro a entrar em fase de compressão
    {
        case 1:
            L_deg_StartIgnitionAngle = 7200 - L_deg_TotalTiming; //Calcula Ignition Angle de 0 a 720 com base no avanço (sparkangle)
        break;
        case 3:
            L_deg_StartIgnitionAngle = 1800 - L_deg_TotalTiming; //Calcula Ignition Angle de 0 a 720 com base no avanço (sparkangle)
        break;
        case 4:
            L_deg_StartIgnitionAngle = 3600 - L_deg_TotalTiming; //Calcula Ignition Angle de 0 a 720 com base no avanço (sparkangle)
        break;
        case 2:
            L_deg_StartIgnitionAngle = 5400 - L_deg_TotalTiming; //Calcula Ignition Angle de 0 a 720 com base no avanço (sparkangle)
        break;
        default:
        break;
    }
    if(L_deg_StartIgnitionAngle >= 7200)        //Apenas corrige o valor para que ângulo sempre fique de 0 a 720°
    {
        L_deg_StartIgnitionAngle -= 7200;
    }
    if(L_deg_StartIgnitionAngle < L_deg_CurrFlywheelAngle) // Com a alteração de antecipar 2 dentes para iniciar o timer, sempre que degrees to start ignition estiver negativo, é pq está longe
    {
        L_deg_StartIgnitionAngle += 7200;       //Correção do Delta para que haja erro devido à forma de medida circular do angulo
    }

    L_deg_DegreesToStartIgn = L_deg_StartIgnitionAngle - L_deg_CurrFlywheelAngle;   //Calcula quantos graus faltam para iniciar a alimentação da bobina (ignição)
    //if(L_deg_DegreesToStartIgn < 0) //Lógica não é mais necessária pq esse valor nunca mais será negativo
    //{
    //    L_deg_DegreesToStartIgn = 0;
    //}
    if((L_deg_DegreesToStartIgn < 180) || ((P_CurrentTooth >= 56) && (L_deg_DegreesToStartIgn < 360))) // Alterado para iniciar timer com 2 dentes de antecedência para garantir que não haverá influencia da variação dos tempos
    {
        if(!CDD_SPARK_SparkBusy())
        {
            L_deg_SparkTimingPrev = Get16s_RTE_deg_SPARKTiming();
            //Lógica para converter L_deg_DegreesToStartIgn em tempo (us), e iniciar um timer com este tempo para chamar a função CDD_SPARK_StartIgnition
            CDD_SYNC_CalculateTimeToStartIgn(L_deg_DegreesToStartIgn, P_us_ToothTime, P_CurrentTooth);
        }
    }
}
void CDD_SYNC_InjectTiming(unsigned char P_CurrentTooth, long int P_us_ToothTime)
{
    static long int L_deg_CurrFlywheelAngle;
    static long int L_deg_InjectTiming;
    static long int L_deg_StartInjectionAngle;
    static unsigned char L_b_InjectionEnable;
    static long int L_deg_InjTimeTiming;
    static unsigned long int L_us_InjectionTime;
    static unsigned long int L_us_InjectionTimePrev;
    static long int L_deg_InjectAdvancePrev;
    static long int L_deg_InjectAdvance;

    long int L_deg_DegreesToStartInj;

    L_deg_CurrFlywheelAngle = S_deg_CurrFlywheelPos;
    L_b_InjectionEnable = S_b_InjectionEnableASW;

    L_us_InjectionTime = Get16u_RTE_t_InjectionTime();
    L_deg_InjectAdvance = Get16s_deg_InjectAdvance();

    if((L_us_InjectionTime > L_us_InjectionTimePrev) || (L_deg_InjectAdvance > L_deg_InjectAdvancePrev))
    {
        L_us_InjectionTime = L_us_InjectionTimePrev;
        L_deg_InjectAdvance = L_deg_InjectAdvancePrev;
    }
    L_us_InjectionTimePrev = L_us_InjectionTime;
    L_deg_InjectAdvancePrev = L_deg_InjectAdvance;

    S_us_InjectionTime = L_us_InjectionTime;

    L_deg_InjTimeTiming = 60*L_us_InjectionTime/P_us_ToothTime; //Calculates Current DWELL Timing in degrees

    L_deg_InjectTiming = L_deg_InjectAdvance + L_deg_InjTimeTiming; //S_deg_InjectTimingASW;

//    S_StartInjForced = 0;
//   if(S_TimerWaitingInj)
//   {
//	   Gpt_StopTimer(GPT_CH_INJ_TIMING);
//       CDD_SYNC_StartInjection_Event();
//       S_StartInjForced = 1;
//       count_forcedInj++;
//   }

    switch(S_CylinderStateFlow[S_CylInjStateFlowIdx])     //StateFlow para calcular o ângulo exato em que deverá ser ligado a bobina de acordo com o próximo cilindro a entrar em fase de compressão
    {
        case 4:
            L_deg_StartInjectionAngle = 7200 - L_deg_InjectTiming; //Calcula Injeection Angle de 0 a 720 com base no avanço (sparkangle)
        break;
        case 2:
            L_deg_StartInjectionAngle = 1800 - L_deg_InjectTiming; //Calcula Injection Angle de 0 a 720 com base no avanço (sparkangle)
        break;
        case 1:
            L_deg_StartInjectionAngle = 3600 - L_deg_InjectTiming; //Calcula Injecction Angle de 0 a 720 com base no avanço (sparkangle)
        break;
        case 3:
            L_deg_StartInjectionAngle = 5400 - L_deg_InjectTiming; //Calcula Injection Angle de 0 a 720 com base no avanço (sparkangle)
        break;
        default:
        break;
    }
    if(!L_b_InjectionEnable)        //Injeção desabilitado pelo Application Software
    {
        return;
    }

    if(L_deg_StartInjectionAngle >= 7200)        //Apenas corrige o valor para que ângulo sempre fique de 0 a 720°
    {
        L_deg_StartInjectionAngle -= 7200;
    }

    if(L_deg_StartInjectionAngle < L_deg_CurrFlywheelAngle)
    {
        L_deg_StartInjectionAngle += 7200;       //Correção do Delta para que haja erro devido à forma de medida circular do angulo
    }

    L_deg_DegreesToStartInj = L_deg_StartInjectionAngle - L_deg_CurrFlywheelAngle;   //Calcula quantos graus faltam para iniciar a injeção
    //if(L_deg_DegreesToStartInj < 0)
    //{
    //    L_deg_DegreesToStartInj = 0;
    //}


    if((L_deg_DegreesToStartInj < 180) || ((P_CurrentTooth >= 56) && (L_deg_DegreesToStartInj < 360))) // Se falta menos de 6°, ou 18° caso seja o dente antecedente à falha, o timer para iniciar a ignição deve ser iniciado
    {

        CDD_SYNC_CalculateTimeToStartInj(L_deg_DegreesToStartInj, P_us_ToothTime, P_CurrentTooth);

        L_us_InjectionTimePrev = Get16u_RTE_t_InjectionTime();
        L_deg_InjectAdvancePrev =  Get16s_deg_InjectAdvance();
    }
}
void CDD_SYNC_CalculateTimeToStartInj(long int P_deg_DegreesToStartInj, unsigned long int P_us_LastToothTime, unsigned char P_CurrentTooth)
{
    uint32 L_timeout_us;

    // if(P_deg_DegreesToStartInj < 5)
    // {
    // 	CDD_SYNC_StartInjection_Event();
    // 	return;
    // }
    S_CylInjStateFlowIdx_old = S_CylInjStateFlowIdx;
    if(S_CylInjStateFlowIdx >= 3)
    {
        S_CylInjStateFlowIdx = 0;
    }
    else
    {
        S_CylInjStateFlowIdx++;
    }

    /* Calcula timeout em microssegundos. Gpt_StartTimer recebe us diretamente,
     * sem necessidade de conversão para ticks — isso é feito internamente pelo MCAL. */
    L_timeout_us = (uint32)P_deg_DegreesToStartInj * P_us_LastToothTime / 60u;

    if(P_CurrentTooth == 1) //REVISAR ESSA LOGICA
    {
        L_timeout_us = 1000000u / CDD_Get_EngineSpeed_RAW(); //Tempo da falha é 3x maior que os outros dentes
    }

    if(L_timeout_us == 0)
    {
        L_timeout_us = 1;
    }

    Gpt_StartTimer(GPT_CH_INJ_TIMING, L_timeout_us);

}
void CDD_SYNC_CalculateTimeToStartIgn(long int P_deg_DegreesToStartIgn, unsigned long int P_us_LastToothTime, unsigned char P_CurrentTooth)
{
    uint32 L_timeout_us;

    // if(P_deg_DegreesToStartIgn < 5)
    // {
    // 	CDD_SYNC_StartIgnition_Event();
    // 	return;
    // }

    S_CylSparkStateFlowIdx_old = S_CylSparkStateFlowIdx;
    if(S_CylSparkStateFlowIdx >= 3)
    {
        S_CylSparkStateFlowIdx = 0;
    }
    else
    {
        S_CylSparkStateFlowIdx++;
    }

    /* Calcula timeout em microssegundos. Gpt_StartTimer recebe us diretamente,
     * sem necessidade de conversão para ticks — isso é feito internamente pelo MCAL. */
    L_timeout_us = (uint32)P_deg_DegreesToStartIgn * P_us_LastToothTime / 60u;

    if(P_CurrentTooth == 1)
    {
        L_timeout_us = 1000000u / CDD_Get_EngineSpeed_RAW(); //Tempo da falha é 3x maior que os outros dentes
    }

    if(L_timeout_us == 0)
    {
        L_timeout_us = 1;
    }

    Gpt_StartTimer(GPT_CH_IGN_TIMING, L_timeout_us);

}
void CDD_SYNC_StartIgnition_Event(void) //Triggered by Timer2 Callback
{
    /* O MCAL Gpt opera em modo one-shot: o timer para automaticamente ao expirar.
     * Não é necessário parar o timer manualmente aqui. */
    CDD_SPARK_StartIgnition(S_CylinderStateFlow[S_CylSparkStateFlowIdx_old]);
}
void CDD_SYNC_StartInjection_Event(void) //Triggered by Timer4 Callback
{
    static unsigned long int L_us_InjectionTime;
    L_us_InjectionTime = S_us_InjectionTime;

    /* O MCAL Gpt opera em modo one-shot: o timer para automaticamente ao expirar.
     * Não é necessário parar o timer manualmente aqui. */

    CDD_INJ_PerformSeqFuelInj(S_CylinderStateFlow[S_CylInjStateFlowIdx_old], (unsigned short int)L_us_InjectionTime);
}
