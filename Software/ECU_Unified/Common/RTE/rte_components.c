/**
 * \file rte_components.c
 * \brief RTE - Leitura e processamento dos sinais de componentes do motor
 *
 * Responsavel por:
 *   - Processar a posicao da borboleta (TPS/TBI) a partir de uma variavel
 *     filtrada externamente (tps_filtered_value)
 *   - Encaminhar o avanco de ignicao (SPARK) e os parametros de injecao
 *     (FUEL) para o RTE, atuando como barramento de dados entre os modulos
 *     de controle e o restante da aplicacao
 *
 * Nota sobre o canal ADC do TPS:
 *   O canal ADC_CH_TBI_POS esta disponivel via MCAL (Adc.h), porem a leitura
 *   direta foi substituida por um valor ja filtrado (tps_filtered_value) em
 *   revisao anterior ao refactoring. As linhas comentadas abaixo mostram o
 *   caminho original via HAL, que nao e' reativado nesta revisao.
 *
 * Dependencias MCAL (plataforma-independentes):
 *   - Nenhuma chamada MCAL ativa neste arquivo (TPS usa tps_filtered_value)
 *
 * Dependencias de aplicacao:
 *   - spark_interface.h    : Get16s_SPARK_SparkAdvance()
 *   - fuel_interface.h     : Get_FUEL_deg_InjAdvance(), Get_FUEL_us_InjectionTime()
 *   - cdd_synchronism.h    : CDD_SYNC_TestAdvancesSweepTask() (modo de teste)
 *   - filters.h            : first_order_filter() (nao utilizado ativamente)
 *
 * Historico:
 *   2026-03-28  Refatorado: removido include de hal_adc_inputs.h (iLLD/HAL).
 *               Adicionados includes de Adc.h e Platform_Types.h para
 *               padronizacao. A logica de processamento foi preservada
 *               identica ao original.
 */

#include "rte_components.h"
#include "Mcal_Adc.h"
#include "Platform_Types.h"
#include "spark_interface.h"
#include "fuel_interface.h"
#include "cdd_synchronism.h"
#include "filters.h"

/* ------------------------------------------------------------------ */
/* Constantes de calibracao do TPS / TBI                               */
/* ------------------------------------------------------------------ */

/** Angulo maximo da borboleta em centesimos de grau (100.00 deg) */
#define MAX_TPS_ANGLE           10000   /* Degrees*100 */

/** Angulo minimo da borboleta em centesimos de grau (0.00 deg) */
#define MIN_TPS_ANGLE           0       /* Degrees*100 */

/** Valor ADC correspondente a borboleta fechada (ponto de calibracao minimo) */
#define MIN_TPS_ADC_ANGLE       467

/** Valor ADC correspondente a borboleta totalmente aberta (ponto de calibracao maximo) */
#define MAX_TPS_ADC_ANGLE       4240

/* ------------------------------------------------------------------ */
/* Sinais do RTE (variaveis de estado internas)                        */
/* ------------------------------------------------------------------ */

volatile unsigned short int S_deg_TPSAnglePosition;              /* Posicao angular da borboleta [deg*100] */
volatile unsigned short int S_deg_TBIPositionSetPoint = 0u;      /* Setpoint da TBI [deg*100]              */

volatile short int          S_RTE_deg_SPARKTiming = 0;           /* Avanco de ignicao [deg*100]            */

volatile short              S_RTE_deg_InjectAdvance;             /* Avanco de injecao [deg*100]            */
volatile unsigned short int S_RTE_t_InjectionTime;               /* Tempo de injecao [us]                 */

/* ------------------------------------------------------------------ */
/* Variavel externa: valor ADC do TPS ja filtrado                      */
/* ------------------------------------------------------------------ */

/**
 * Valor filtrado do ADC do TPS, calculado externamente (modulo de filtragem ADC
 * por varredura em background). Escala: valor ADC raw (0..4095 / 12-bit).
 */
extern unsigned long tps_filtered_value;

/* ------------------------------------------------------------------ */
/* Prototipos das funcoes internas                                     */
/* ------------------------------------------------------------------ */

static void Update_RTE_deg_TPSAnglePosition(void);

/* ------------------------------------------------------------------ */
/* Funcao principal de atualizacao                                      */
/* ------------------------------------------------------------------ */

/**
 * \brief Atualiza todos os sinais de componentes num unico ciclo de task.
 *
 * Chamada periodicamente (ex.: 10 ms) pela task de controle principal.
 */
void Update_RTE_ComponentsSignals(void)
{
    /* Sinais analogicos / posicao */
    Update_RTE_deg_TPSAnglePosition();

    /* Sinais de controle (ignicao e injecao) */
    Update_RTE_deg_SPARKTiming();
    Update_RTE_deg_InjectAdvance();
    Update_RTE_t_InjectionTime();
}

/* ------------------------------------------------------------------ */
/* TPS / TBI - Posicao da borboleta                                    */
/* ------------------------------------------------------------------ */

/**
 * \brief Processa o valor filtrado do ADC do TPS e converte para angulo.
 *
 * Utiliza o valor pre-filtrado (tps_filtered_value) em lugar de ler o
 * ADC diretamente. A conversao e' linear entre os pontos de calibracao:
 *
 *   angle = (MAX_ANGLE - MIN_ANGLE) * (adc - MIN_ADC) / (MAX_ADC - MIN_ADC) + MIN_ANGLE
 *
 * Saturacao: MIN_TPS_ADC_ANGLE aplicada antes da conversao; resultado
 * saturado em MIN_TPS_ANGLE .. MAX_TPS_ANGLE apos a conversao.
 *
 * Nota: leitura direta via MCAL disponivel como:
 *   Adc_ReadChannel_mV(ADC_CH_TBI_POS) ou Adc_ReadChannel_Raw(ADC_CH_TBI_POS)
 * porem nao ativada nesta revisao (usando valor filtrado em background).
 */
static void Update_RTE_deg_TPSAnglePosition(void)
{
    unsigned short int L_deg_TPSAnglePosition;

    /* Usa o valor ADC filtrado pelo modulo de varredura em background */
    /* Alternativa via MCAL: L_deg_TPSAnglePosition = (unsigned short int)Adc_ReadChannel_Raw(ADC_CH_TBI_POS); */
    L_deg_TPSAnglePosition = (unsigned short int) tps_filtered_value;

    /* Saturacao do valor ADC no ponto de calibracao minimo */
    if (L_deg_TPSAnglePosition < MIN_TPS_ADC_ANGLE)
    {
        L_deg_TPSAnglePosition = MIN_TPS_ADC_ANGLE;
    }

    /* Conversao linear ADC -> angulo em centesimos de grau */
    L_deg_TPSAnglePosition = ((MAX_TPS_ANGLE - MIN_TPS_ANGLE) *
                               (L_deg_TPSAnglePosition - MIN_TPS_ADC_ANGLE) /
                               (MAX_TPS_ADC_ANGLE - MIN_TPS_ADC_ANGLE)) + MIN_TPS_ANGLE;

    /* Saturacao do angulo calculado */
    if (L_deg_TPSAnglePosition > MAX_TPS_ANGLE)
    {
        L_deg_TPSAnglePosition = MAX_TPS_ANGLE;
    }
    else if (L_deg_TPSAnglePosition < MIN_TPS_ANGLE)
    {
        L_deg_TPSAnglePosition = MIN_TPS_ANGLE;
    }

    /* Filtro de primeira ordem desabilitado intencionalmente */
    /* L_deg_TPSAnglePosition = first_order_filter(L_deg_TPSAnglePosition, S_deg_TPSAnglePosition, 0.2); */

    S_deg_TPSAnglePosition = L_deg_TPSAnglePosition;
}

/**
 * \brief Retorna a posicao angular da borboleta em centesimos de grau.
 */
unsigned short int Get16u_RTE_deg_TPSAnglePosition(void)
{
    return S_deg_TPSAnglePosition;
}

/**
 * \brief Retorna o setpoint de posicao da TBI em centesimos de grau.
 */
unsigned short int Get16u_RTE_deg_TBIPositionSetPoint(void)
{
    return S_deg_TBIPositionSetPoint;
}

/**
 * \brief Define o setpoint de posicao da TBI.
 * \param value Angulo desejado em centesimos de grau (0 .. 10000)
 */
void Set16u_RTE_deg_TBIPositionSetPoint(unsigned short int value)
{
    S_deg_TBIPositionSetPoint = value;
}

/* ------------------------------------------------------------------ */
/* SPARK - Avanco de ignicao                                           */
/* ------------------------------------------------------------------ */

/**
 * \brief Atualiza o avanco de ignicao a partir do modulo SPARK.
 *
 * Em modo de teste (TESTADVANCES_ENABLED == TRUE), o valor e' gerado
 * internamente pela funcao CDD_SYNC_TestAdvancesSweepTask() e nao
 * e' sobrescrito aqui.
 */
void Update_RTE_deg_SPARKTiming(void)
{
#if (TESTADVANCES_ENABLED == FALSE)
    S_RTE_deg_SPARKTiming = Get16s_SPARK_SparkAdvance();
#endif
}

/**
 * \brief Retorna o avanco de ignicao atual em centesimos de grau.
 */
short int Get16s_RTE_deg_SPARKTiming(void)
{
    return S_RTE_deg_SPARKTiming;
}

/* ------------------------------------------------------------------ */
/* FUEL - Avanco e tempo de injecao                                    */
/* ------------------------------------------------------------------ */

/**
 * \brief Atualiza o avanco de injecao a partir do modulo FUEL.
 *
 * Em modo de teste (TESTADVANCES_ENABLED == TRUE), o valor e' gerado
 * internamente e nao e' sobrescrito aqui.
 */
void Update_RTE_deg_InjectAdvance(void)
{
#if (TESTADVANCES_ENABLED == FALSE)
    S_RTE_deg_InjectAdvance = Get_FUEL_deg_InjAdvance();
#endif
}

/**
 * \brief Retorna o avanco de injecao atual em centesimos de grau.
 */
short int Get16s_deg_InjectAdvance(void)
{
    return S_RTE_deg_InjectAdvance;
}

/**
 * \brief Atualiza o tempo de injecao a partir do modulo FUEL.
 */
void Update_RTE_t_InjectionTime(void)
{
    S_RTE_t_InjectionTime = Get_FUEL_us_InjectionTime();
}

/**
 * \brief Retorna o tempo de injecao atual em microsegundos.
 */
short int Get16u_RTE_t_InjectionTime(void)
{
    return S_RTE_t_InjectionTime;
}

/* ------------------------------------------------------------------ */
/* Modo de teste: varredura de avancos (TESTADVANCES_ENABLED == TRUE)  */
/* ------------------------------------------------------------------ */

#if (TESTADVANCES_ENABLED == TRUE)

#include <stdlib.h>  /* Para rand(), srand() */

/*
 * Se TESTADVANCES_RANDOM == TRUE: gera variacoes incrementais aleatorias
 * dentro de um pulo maximo definido (RANDOM_MAX_*_JUMP).
 * Se TESTADVANCES_RANDOM == FALSE: usa varredura ping-pong (vai-e-volta).
 */
#define TESTADVANCES_RANDOM   FALSE

/* Faixa de avanco de ignicao: -70 deg .. +20 deg (escala *10) */
#define SPARK_MIN_ANGLE   (-70 * 10)
#define SPARK_MAX_ANGLE   ( 20 * 10)

/* Faixa de avanco de injecao: -300 deg .. +20 deg (escala *10) */
#define INJ_MIN_ANGLE     (-300 * 10)
#define INJ_MAX_ANGLE     (  20 * 10)

/* Passo maximo do delta aleatorio (3 deg = 30 na escala *10) */
#define RANDOM_MAX_SPARK_JUMP  (3 * 10)
#define RANDOM_MAX_INJ_JUMP    (3 * 10)

/* Angulos atuais mantidos entre chamadas */
static long int s_sparkCurrAngle = 0;
static long int s_injCurrAngle   = 0;

/**
 * \brief Inicializa a semente do gerador de numeros pseudo-aleatorios.
 *
 * Deve ser chamada uma vez durante a inicializacao se TESTADVANCES_RANDOM
 * for TRUE. Se nao chamada, a sequencia sera sempre a mesma (reproduzivel).
 */
void CDD_SYNC_TestAdvancesRandomInit(void)
{
    /* Semente variavel baseada em tempo - requer time.h se disponivel */
    srand((unsigned) time(NULL));

    /* Para sequencia reproduzivel em testes: srand(1234U); */
}

/**
 * \brief Gera os valores de avanco de ignicao e injecao para testes.
 *
 * Chamada periodicamente pela task de sincronismo quando em modo de teste.
 * Implementa dois sub-modos controlados por TESTADVANCES_RANDOM:
 *
 *   TRUE  - incrementos aleatorios com pulo maximo definido
 *   FALSE - varredura ping-pong (sobe e desce continuamente)
 */
void CDD_SYNC_TestAdvancesSweepTask(void)
{
#if (TESTADVANCES_RANDOM == TRUE)

    /* --- MODO ALEATORIO (com pulo maximo) --- */

    /* Calcula e aplica variacao aleatoria para ignicao */
    int deltaSpark = (rand() % (2 * RANDOM_MAX_SPARK_JUMP + 1)) - RANDOM_MAX_SPARK_JUMP;
    s_sparkCurrAngle += deltaSpark;
    if (s_sparkCurrAngle > SPARK_MAX_ANGLE)
    {
        s_sparkCurrAngle = SPARK_MAX_ANGLE;
    }
    else if (s_sparkCurrAngle < SPARK_MIN_ANGLE)
    {
        s_sparkCurrAngle = SPARK_MIN_ANGLE;
    }

    /* Calcula e aplica variacao aleatoria para injecao */
    int deltaInj = (rand() % (2 * RANDOM_MAX_INJ_JUMP + 1)) - RANDOM_MAX_INJ_JUMP;
    s_injCurrAngle += deltaInj;
    if (s_injCurrAngle > INJ_MAX_ANGLE)
    {
        s_injCurrAngle = INJ_MAX_ANGLE;
    }
    else if (s_injCurrAngle < INJ_MIN_ANGLE)
    {
        s_injCurrAngle = INJ_MIN_ANGLE;
    }

    /* Atribui aos sinais do sistema */
    S_RTE_deg_SPARKTiming   = s_sparkCurrAngle;
    S_RTE_deg_InjectAdvance = s_injCurrAngle;

#else /* (TESTADVANCES_RANDOM == FALSE) */

    /* --- MODO PING-PONG (varredura vai-e-volta) --- */

    static int s_sparkDirection = +1;
    static int s_injDirection   = +1;

    #define SPARK_STEP_ANGLE  10
    #define INJ_STEP_ANGLE    10

    /* Atualiza avanco de ignicao */
    s_sparkCurrAngle += (s_sparkDirection * SPARK_STEP_ANGLE);
    if (s_sparkCurrAngle >= SPARK_MAX_ANGLE)
    {
        s_sparkCurrAngle = SPARK_MAX_ANGLE;
        s_sparkDirection = -1;
    }
    else if (s_sparkCurrAngle <= SPARK_MIN_ANGLE)
    {
        s_sparkCurrAngle = SPARK_MIN_ANGLE;
        s_sparkDirection = +1;
    }

    /* Atualiza avanco de injecao */
    s_injCurrAngle += (s_injDirection * INJ_STEP_ANGLE);
    if (s_injCurrAngle >= INJ_MAX_ANGLE)
    {
        s_injCurrAngle = INJ_MAX_ANGLE;
        s_injDirection = -1;
    }
    else if (s_injCurrAngle <= INJ_MIN_ANGLE)
    {
        s_injCurrAngle = INJ_MIN_ANGLE;
        s_injDirection = +1;
    }

    /* Atribui aos sinais do sistema */
    S_RTE_deg_SPARKTiming   = s_sparkCurrAngle;
    S_RTE_deg_InjectAdvance = s_injCurrAngle;

#endif /* TESTADVANCES_RANDOM */
}

#endif /* (TESTADVANCES_ENABLED == TRUE) */
