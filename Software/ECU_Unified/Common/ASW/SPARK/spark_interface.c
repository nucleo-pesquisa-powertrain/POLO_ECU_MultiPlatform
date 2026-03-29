/*
 * spark_interface.c
 *
 * Modulo de interface ASW para controle de avanco de ignicao (centelha).
 *
 * Refatorado para ECU_Unified: dependencias iLLD removidas.
 * - Bloco SAVE_LOG: substituido por stub MCAL via Uart.h (Uart_Write).
 * - Todas as chamadas IfxAsclin_Asc_write e tipos Ifx_SizeT substituidos.
 * - Logica de aplicacao preservada integralmente.
 *
 * Created on: (original) 2023
 * Refactored: 2026-03-28
 */

#include "spark_interface.h"
#include "rte_environment.h"
#include "mngmt_interface.h"
#include "ECU_State_interface.h"
#include <stdbool.h>

/* Inclusao MCAL para log via UART (substitui ASCLIN_UART.h + IfxAsclin_Asc_write) */
#include "Uart.h"
#include <stdio.h>
#include <string.h>

#define SAVE_LOG

void Control_ml_ign(void);

/*--------------------- ENTRADAS ---------------------*/
static unsigned short S_PARK_rpm_EngineSpeed;
static unsigned short S_SPARK_P_AirPress;
static unsigned short S_SPARK_p_EthanolPct;
/* Estado ECU gerenciado via maquina de estados unificada */
static ECU_State_t S_SPARK_s_ManagementState;

/*--------------------- SAIDAS ---------------------*/
short int S_SPARK_SparkAdvance = 0;

/* Limites de saturacao na saida: valor em decimos de grau
 * Positivo:  35 graus -> 350
 * Negativo: -10 graus -> -100                              */
const short int SAT_SPARK_POS_SAIDA = 350;
const short int SAT_SPARK_NEG_SAIDA = -100;

/*--------------------- PARAMETROS DO CONTROLADOR PI ---------------------*/
/* Ganhos calibrados para borboleta a 5% */
float Kp_spark = 0.231f;       /* Ganho proporcional no tempo continuo */
float Ki_spark = 0.511f;       /* Ganho integral no tempo continuo    */
static const float Ts = 0.02f; /* Periodo de amostragem: 20 ms        */
float sat_spark_pos =  30.0f;  /* Saturacao positiva do controlador   */
float sat_spark_neg =  -5.0f;  /* Saturacao negativa do controlador   */

/* Setpoint e erro de rotacao (RPM) */
float setpoint_rpm = 900.0f;
float erro_rpm     = 0.0f;

/*--------------------- ESTADOS INTERNOS DO PI ---------------------*/
static float u_ant     = 0.0f; /* Saida anterior do controlador         */
static float e_int     = 0.0f; /* Acumulador da parcela integral        */
static float u_ang_ign = 0.0f; /* Saida atual do controlador            */
static float ei;               /* Erro efetivo que alimenta o integrador */
static bool  control_ml_on = 1;

/* ---------------------------------------------------------------
 * SPARK_MainTask20ms
 * Tarefa periodica de 20 ms: atualiza entradas e executa controle.
 * --------------------------------------------------------------- */
void SPARK_MainTask20ms(void)
{
    /* Atualiza entradas a partir do RTE */
    S_PARK_rpm_EngineSpeed   = Get16u_RTE_rpm_EngineSpeeed();
    S_SPARK_P_AirPress       = Get16u_RTE_P_ManAirPress();
    S_SPARK_p_EthanolPct     = Get16u_RTE_p_EthanolPercent();
    /* Estado atual da ECU via maquina de estados unificada */
    S_SPARK_s_ManagementState = Get_ECU_State();

    /* Seleciona logica de controle conforme estado do motor */
    if (S_SPARK_s_ManagementState == ECU_STATE_CRANKING)
    {
        /* Durante partida: avanço fixo (mapa de partida a ser implementado) */
        Set16s_SPARK_SparkAdvance(120);
    }
    else if (S_SPARK_s_ManagementState == ECU_STATE_IDLE)
    {
        /* Em marcha lenta: controle PI fechado */
        Control_ml_ign();
    }
}

/*-------------------------------------------------------*/
/*--------------------- ENTRADAS -----------------------*/

unsigned short int Get16u_SPARK_RPM_EngineSpeed(void)
{
    return S_PARK_rpm_EngineSpeed;
}

unsigned short int Get16u_SPARK_P_AirPress(void)
{
    return S_SPARK_P_AirPress;
}

unsigned short int Get16u_SPARK_p_EthanolPct(void)
{
    return S_SPARK_p_EthanolPct;
}

unsigned short int Get16u_SPARK_s_ManagementState(void)
{
    return (unsigned short int)S_SPARK_s_ManagementState;
}

/*------------------------------------------------------*/
/*--------------------- SAIDAS -----------------------*/

/* Escreve o avanço de ignicao com saturacao de seguranca.
 * Valor em decimos de grau: ex. 100 = 10 graus.          */
void Set16s_SPARK_SparkAdvance(short int value)
{
    if (value > SAT_SPARK_POS_SAIDA)
    {
        value = SAT_SPARK_POS_SAIDA;
    }
    else if (value < SAT_SPARK_NEG_SAIDA)
    {
        value = SAT_SPARK_NEG_SAIDA;
    }

    S_SPARK_SparkAdvance = value;
}

short int Get16s_SPARK_SparkAdvance(void)
{
    return S_SPARK_SparkAdvance;
}

/* ---------------------------------------------------------------
 * PI_Antiwindup
 * Controlador PI discreto com anti-windup por congelamento
 * do integrador quando a saida esta saturada.
 * --------------------------------------------------------------- */
float PI_Antiwindup(float erro)
{
    /* 1) Anti-windup: congela integrador em caso de saturacao */
    if ((u_ant >= sat_spark_pos) || (u_ant <= sat_spark_neg))
    {
        ei = 0.0f;
    }
    else
    {
        ei = erro;
    }

    /* 2) Atualiza acumulador integral */
    e_int += (ei * Ts);

    /* 3) Calcula saida do controlador PI */
    float u = (Kp_spark * erro) + (Ki_spark * e_int);

    /* 4) Saturacao da saida do controlador */
    if (u > sat_spark_pos)
    {
        u = sat_spark_pos;
    }
    else if (u < sat_spark_neg)
    {
        u = sat_spark_neg;
    }

    /* 5) Armazena saida para proximo ciclo */
    u_ant = u;

    return u;
}

/* ---------------------------------------------------------------
 * UART_Print_ControlVars  (disponivel apenas com SAVE_LOG ativo)
 *
 * Substituicao iLLD -> MCAL:
 *   Antes : IfxAsclin_Asc_write(&g_ascHandle, ...) com Ifx_SizeT
 *   Depois: Uart_Write(UART_CHANNEL_DEBUG, ...) com uint16
 *
 * O canal UART_CHANNEL_DEBUG deve ser configurado no arquivo
 * de configuracao MCAL (Uart_Cfg.h / EcucPartition).
 * --------------------------------------------------------------- */
#ifdef SAVE_LOG

void UART_Print_ControlVars(unsigned short int setpoint,
                             unsigned short int rpm,
                             float              ang_ign)
{
    static int contador = 0;
    char buffer[64];
    uint16 len;

    snprintf(buffer, sizeof(buffer),
             "%d,%d,%.2f,%d\r\n",
             setpoint, rpm, ang_ign, contador++);

    len = (uint16)strlen(buffer);

    /* MCAL UART write: canal de debug configurado no projeto */
    Uart_Write(UART_CHANNEL_DEBUG, (const uint8 *)buffer, len);
}

#endif /* SAVE_LOG */

/* ---------------------------------------------------------------
 * Control_ml_ign
 * Funcao de controle de avanço em marcha lenta (ML).
 * Executa o PI fechado sobre o erro de rotacao.
 * --------------------------------------------------------------- */
void Control_ml_ign(void)
{
    if (control_ml_on)
    {
        erro_rpm   = setpoint_rpm - (float)Get16u_RTE_rpm_EngineSpeeed();
        u_ang_ign  = PI_Antiwindup(erro_rpm);
        Set16s_SPARK_SparkAdvance((short)(u_ang_ign * 10.0f));
    }

#ifdef SAVE_LOG
    UART_Print_ControlVars((unsigned short int)setpoint_rpm,
                            Get16u_RTE_rpm_EngineSpeeed(),
                            Get16s_SPARK_SparkAdvance());
#endif
}
