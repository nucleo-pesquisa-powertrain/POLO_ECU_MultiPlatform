/**
 * \file cdd_spark.h
 * \brief CDD - Controle de Ignicao (Bobinas)
 *
 * Modulo responsavel pela ativacao e desativacao das bobinas de ignicao
 * para um motor 4 cilindros com duas bobinas de faisca perdida:
 *   - Coil 1/4: cilindros 1 e 4
 *   - Coil 2/3: cilindros 2 e 3
 *
 * Utiliza a MCAL AUTOSAR (Gpt.h, Dio.h) - sem dependencias iLLD.
 *
 * \note DWELL_TIME esta em MICROSEGUNDOS. Nenhuma conversao de ticks
 *       e' necessaria no layer CDD; a MCAL faz isso internamente.
 */
#ifndef CDD_SPARK_H
#define CDD_SPARK_H

#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Parametros de ignicao                                               */
/* ------------------------------------------------------------------ */

/** Tempo de dwell em microssegundos (5 ms) */
#define DWELL_TIME      5000u

/* ------------------------------------------------------------------ */
/* Prototipos publicos                                                 */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo de ignicao
 *
 * Desliga ambas as bobinas e configura o estado inicial dos LEDs
 * de diagnostico.
 */
void CDD_SPARK_Init(void);

/**
 * \brief Inicia a carga da bobina para o cilindro indicado
 *
 * \param cyl Numero do cilindro (1 a 4)
 *
 * Aciona a bobina correspondente e dispara o timer de dwell.
 * Se a outra bobina ja estiver ativa (sobreposicao), salva o
 * tempo restante para recarregar o timer depois da primeira faisca.
 */
void CDD_SPARK_StartIgnition(uint8 cyl);

/**
 * \brief Callback: descarrega a bobina cujo dwell expirou
 *
 * Chamado pela ISR do timer GPT_CH_DWELL. Desliga a bobina que
 * terminou o dwell. Em caso de sobreposicao, reinicia o timer
 * com o tempo residual da outra bobina.
 */
void CDD_SPARK_SparkEvent(void);

/**
 * \brief Retorna se ambas as bobinas estao simultaneamente ativas
 *
 * \return 1 se ambas ocupadas, 0 caso contrario
 */
uint8 CDD_SPARK_SparkBusy(void);

#endif /* CDD_SPARK_H */
