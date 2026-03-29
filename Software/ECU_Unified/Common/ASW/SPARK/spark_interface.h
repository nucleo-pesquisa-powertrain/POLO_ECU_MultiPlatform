#ifndef _SPARK_INTERFACE_H
#define _SPARK_INTERFACE_H

void SPARK_MainTask20ms(void);

/*---------------------------------------------------*/
/*--------------------- INPUTS ---------------------*/
unsigned short int Get16u_SPARK_RPM_EngineSpeed(void);
unsigned short int Get16u_SPARK_P_AirPress(void);
unsigned short int Get16u_SPARK_p_EthanolPct(void);
unsigned short int Get16u_SPARK_s_ManagementState(void);

/*----------------------------------------------------*/
/*--------------------- OUTPUTS ---------------------*/
void Set16s_SPARK_SparkAdvance(short int value);
short int Get16s_SPARK_SparkAdvance(void);

#endif /* _SPARK_INTERFACE_H */
