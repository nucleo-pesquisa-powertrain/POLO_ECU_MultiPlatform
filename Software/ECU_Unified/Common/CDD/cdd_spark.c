/**
 * \file cdd_spark.c
 * \brief CDD - Controle de Ignicao (Bobinas)
 *
 * Implementa a logica de ativacao das bobinas de faisca perdida para
 * motor 4 cilindros. Utiliza exclusivamente a MCAL AUTOSAR:
 *   - Gpt.h  : timer one-shot de dwell (GPT_CH_DWELL) em microsegundos
 *   - Dio.h  : controle digital de bobinas e LEDs de diagnostico
 *
 * Dependencias iLLD removidas:
 *   - GPT12_Timer_Interrupt.h  (substituido por Gpt.h)
 *   - hal_discrete_outputs.h   (substituido por Dio.h)
 *
 * Mapeamento de canais DIO:
 *   DIO_CH_COIL1   -> bobina cilindros 1 e 4
 *   DIO_CH_COIL2   -> bobina cilindros 2 e 3
 *   DIO_CH_LED1    -> LED diagnostico Coil 1/4 em carga
 *   DIO_CH_LED2    -> LED diagnostico Coil 2/3 em carga
 *   DIO_CH_LED_LOW -> LED diagnostico init
 *
 * \note Todos os valores de tempo estao em MICROSEGUNDOS.
 *       A conversao para ticks do hardware e' responsabilidade
 *       da implementacao de Gpt.h em cada plataforma.
 */

#include "cdd_spark.h"
#include "Gpt.h"
#include "Dio.h"
#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Variaveis de estado do modulo (escopo de arquivo)                   */
/* ------------------------------------------------------------------ */

/** Cilindro que gerou o ultimo pedido de ignicao */
static uint8  S_CylinderSpark = 0u;

/** Flag: bobina 1/4 esta em carga */
static uint8  S_Coil14Busy = 0u;

/** Flag: bobina 2/3 esta em carga */
static uint8  S_Coil23Busy = 0u;

/**
 * Tempo residual em microsegundos quando ha sobreposicao de bobinas.
 *
 * Calculado em CDD_SPARK_StartIgnition() quando as duas bobinas
 * entram em carga simultaneamente. Armazena quanto tempo a primeira
 * bobina ainda precisava quando a segunda foi ligada. Utilizado em
 * CDD_SPARK_SparkEvent() para recarregar o timer com o delta correto.
 */
static uint32 S_CoilNextTimeValue_us = 0u;

/* ------------------------------------------------------------------ */
/* Implementacao publica                                               */
/* ------------------------------------------------------------------ */

void CDD_SPARK_Init(void)
{
    /* Garante bobinas desligadas na inicializacao */
    Dio_WriteChannel(DIO_CH_COIL1, DIO_LOW);
    Dio_WriteChannel(DIO_CH_COIL2, DIO_LOW);

    /* LED de diagnostico: estado de repouso */
    Dio_WriteChannel(DIO_CH_LED_LOW, DIO_HIGH);
}

void CDD_SPARK_StartIgnition(uint8 cyl)
{
    /* Aciona a bobina correspondente ao cilindro solicitado */
    if((cyl == 1u) || (cyl == 4u))
    {
        Dio_WriteChannel(DIO_CH_COIL1, DIO_HIGH);
        S_Coil14Busy = 1u;
        Dio_WriteChannel(DIO_CH_LED1, DIO_HIGH);  /* LD1: Coil 1/4 em carga */
    }
    else if((cyl == 2u) || (cyl == 3u))
    {
        Dio_WriteChannel(DIO_CH_COIL2, DIO_HIGH);
        S_Coil23Busy = 1u;
        Dio_WriteChannel(DIO_CH_LED2, DIO_HIGH);  /* LD2: Coil 2/3 em carga */
    }
    else
    {
        /* Cilindro invalido - ignora */
        return;
    }

    if(S_Coil14Busy && S_Coil23Busy)
    {
        /*
         * Sobreposicao: ambas as bobinas estao ativas ao mesmo tempo.
         *
         * Calcula o tempo que ja decorreu desde que o timer foi iniciado
         * para a primeira bobina. O delta resultante representa quanto
         * tempo a primeira bobina ainda precisaria dwell antes que o
         * timer expire. Isso e' salvo para recarregar o timer quando
         * CDD_SPARK_SparkEvent() desligar a segunda bobina.
         *
         * S_CoilNextTimeValue_us = DWELL_TIME - tempo_ja_decorrido
         */
        S_CoilNextTimeValue_us = (uint32)DWELL_TIME - Gpt_GetElapsedTime_us(GPT_CH_DWELL);
    }
    else
    {
        /*
         * Caso simples: apenas uma bobina ativa.
         * Inicia o timer de dwell diretamente com o tempo configurado.
         * O valor e' passado em microsegundos; a MCAL converte para
         * ticks internamente conforme o prescaler do hardware.
         */
        Gpt_StartTimer(GPT_CH_DWELL, (uint32)DWELL_TIME);
    }

    S_CylinderSpark = cyl;
}

void CDD_SPARK_SparkEvent(void)
{
    /* Para o timer antes de qualquer operacao de saida */
    Gpt_StopTimer(GPT_CH_DWELL);

    if(S_Coil14Busy && !S_Coil23Busy)
    {
        /* Apenas Coil 1/4 estava ativa - desliga normalmente */
        Dio_WriteChannel(DIO_CH_COIL1, DIO_LOW);
        S_Coil14Busy = 0u;
        Dio_WriteChannel(DIO_CH_LED1, DIO_LOW);   /* LD1 OFF: Coil 1/4 descarregada */
    }
    else if(!S_Coil14Busy && S_Coil23Busy)
    {
        /* Apenas Coil 2/3 estava ativa - desliga normalmente */
        Dio_WriteChannel(DIO_CH_COIL2, DIO_LOW);
        S_Coil23Busy = 0u;
        Dio_WriteChannel(DIO_CH_LED2, DIO_LOW);   /* LD2 OFF: Coil 2/3 descarregada */
    }
    else if((S_CylinderSpark == 1u) || (S_CylinderSpark == 4u))
    {
        /*
         * Sobreposicao - ultimo pedido foi cil. 1 ou 4:
         * O timer expirou para Coil 2/3 (a que foi ligada primeiro).
         * Desliga Coil 2/3 e reinicia o timer com o tempo residual
         * de Coil 1/4, calculado em CDD_SPARK_StartIgnition().
         */
        Dio_WriteChannel(DIO_CH_COIL2, DIO_LOW);
        S_Coil23Busy = 0u;
        Dio_WriteChannel(DIO_CH_LED2, DIO_LOW);   /* LD2 OFF: Coil 2/3 descarregada */

        /* Recarrega timer com tempo residual de Coil 1/4 em microsegundos */
        Gpt_StartTimer(GPT_CH_DWELL, S_CoilNextTimeValue_us);
    }
    else if((S_CylinderSpark == 2u) || (S_CylinderSpark == 3u))
    {
        /*
         * Sobreposicao - ultimo pedido foi cil. 2 ou 3:
         * O timer expirou para Coil 1/4 (a que foi ligada primeiro).
         * Desliga Coil 1/4 e reinicia o timer com o tempo residual
         * de Coil 2/3.
         */
        Dio_WriteChannel(DIO_CH_COIL1, DIO_LOW);
        S_Coil14Busy = 0u;
        Dio_WriteChannel(DIO_CH_LED1, DIO_LOW);   /* LD1 OFF: Coil 1/4 descarregada */

        /* Recarrega timer com tempo residual de Coil 2/3 em microsegundos */
        Gpt_StartTimer(GPT_CH_DWELL, S_CoilNextTimeValue_us);
    }
    else
    {
        /*
         * Seguranca: estado invalido - desliga tudo.
         * Pela logica normal do modulo este bloco nao deve ser atingido.
         */
        Dio_WriteChannel(DIO_CH_COIL1, DIO_LOW);
        Dio_WriteChannel(DIO_CH_COIL2, DIO_LOW);
        S_Coil14Busy = 0u;
        S_Coil23Busy = 0u;
        Dio_WriteChannel(DIO_CH_LED1, DIO_LOW);   /* LD1 OFF */
        Dio_WriteChannel(DIO_CH_LED2, DIO_LOW);   /* LD2 OFF */
    }
}

uint8 CDD_SPARK_SparkBusy(void)
{
    return (uint8)(S_Coil14Busy && S_Coil23Busy);
}
