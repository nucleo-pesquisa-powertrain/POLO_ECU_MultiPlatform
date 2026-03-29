/*
 * tbi_calibration.c
 *
 *  Created on: 21 de abr de 2025
 *      Author: u28m43
 */

#include <stdint.h>
#include <stdbool.h>

#include "rte_components.h"
#include "cdd_tbi.h"

extern unsigned long tps_filtered_value;

/* ==============================================================
 * Ajustes de passo e limiares para calibracao da valvula borboleta
 * ==============================================================*/
#define PWM_STEP                        1       /* Passo do PWM por iteracao (em %) */
#define STEP_DELAY_COUNT                100     /* Numero de ciclos de 10ms entre cada passo (100 = 1 segundo) */
#define THRESH_LIMP_EXIT                100     /* Threshold de variacao para identificar saida do modo limp-home */
#define THRESH_MAX_LIMIT_POS            3500    /* Threshold para identificar max do curso da borboleta entre repouso e posicao atual */
#define THRESH_MAX_LIMIT_NEG            200     /* Threshold para identificar min do curso da borboleta entre repouso e posicao atual */
#define N_CALIB_REP                     5       /* Numero de repeticoes para tirar a media final */
#define MAX_SATURATION_CONFIRMATION     5       /* Numero de confirmacoes para considerar que chegou ao threshold max/min */
#define THRESH_MOV_SAIU_MIN             100     /* DELTA ADC > 100 indica que a valvula comecou a sair do repouso */
#define THRESH_MOV_SAIU_MAX             1500    /* DELTA ADC > 1500 indica que a valvula comecou a sair do ponto maximo */
#define OFFSET_ABERTURA                 20      /* % de inicio pois e uma banda morta, no uso e feito (duty = 0 + OFFSET_ABERTURA) */
#define OFFSET_FECHAMENTO               20      /* -% de inicio pois e uma banda morta, no uso e feito (duty = 0 - OFFSET_FECHAMENTO) */
#define THRESH_RETORNO_REPOUSO          100     /* Threshold para considerar que a valvula voltou ao ponto de repouso (ADC) */

/* Resultados da calibracao -----------------------------------------------*/
int8_t   pwm_curso_min,  pwm_curso_max;           /* PWM nos extremos de curso (0 e 90 graus) */
int8_t   pwm_lh_min,     pwm_lh_max;              /* PWM nas bordas do limp-home */
uint16_t adc_curso_min,  adc_curso_max;           /* Leitura ADC nos cursos max e min */
uint16_t adc_lh_min,     adc_lh_max;              /* Leitura ADC nas bordas do limp-home */
uint16_t adc_lh_repouso;                          /* Leitura ADC em repouso (PWM = 0) */

/* Angulos correspondentes */
uint16_t angulo_curso_min, angulo_curso_max;      /* Angulos nos cursos max e min */
uint16_t angulo_lh_min,    angulo_lh_max;         /* Angulos nas bordas do limp-home */
uint16_t angulo_lh_repouso;                       /* Angulo em repouso (PWM = 0) */

/* Maquina de estados da calibracao --------------------------------------*/
typedef enum {
    CALIB_IDLE,                       /* Aguarda repouso antes de iniciar ciclo */
    CALIB_ABRINDO_INIT,              /* (nao utilizado) */
    CALIB_ABRINDO_RUN,               /* Varredura positiva da borboleta */
    CALIB_FECHANDO_INIT,             /* (nao utilizado) */
    CALIB_FECHANDO_RUN,              /* Varredura negativa da borboleta */
    CALIB_FINALIZADO,                /* Calibracao completa */
    CALIB_TRANSICAO_FECHAMENTO,      /* Retorno suave ao neutro entre abertura e fechamento */
    CALIB_TRANSICAO_ABERTURA,        /* Retorno suave do neutro para a posicao de repouso */
    CALIB_MEDIA_FINAL                /* Calculo da media apos N repeticoes */
} CalibState_t;

/* ================================
 * Variaveis internas da calibracao
 * ================================ */

static CalibState_t calib_state = CALIB_IDLE;
static uint16_t adc_prev = 0;
static bool saiu_lh = false;
static int8_t duty = 0;
static uint16_t adc_now, delta;
static int cont = 0;
static int cont_saturado  = 0;
static uint8_t calib_count = 0;

static uint16_t adc_lh_max_array[N_CALIB_REP];
static uint16_t adc_curso_max_array[N_CALIB_REP];
static int8_t   pwm_lh_max_array[N_CALIB_REP];
static int8_t   pwm_curso_max_array[N_CALIB_REP];

static uint16_t adc_lh_repouso_array[N_CALIB_REP];

static uint16_t adc_lh_min_array[N_CALIB_REP];
static uint16_t adc_curso_min_array[N_CALIB_REP];
static int8_t   pwm_lh_min_array[N_CALIB_REP];
static int8_t   pwm_curso_min_array[N_CALIB_REP];

static uint16_t angulo_lh_max_array[N_CALIB_REP];
static uint16_t angulo_curso_max_array[N_CALIB_REP];
static uint16_t angulo_lh_min_array[N_CALIB_REP];
static uint16_t angulo_curso_min_array[N_CALIB_REP];
static uint16_t angulo_lh_repouso_array[N_CALIB_REP];

/* PWM quando sai da posicao minima ou maxima */
int8_t pwm_saiu_min = 0;
int8_t pwm_saiu_max = 0;

static int8_t pwm_saiu_min_array[N_CALIB_REP];
static int8_t pwm_saiu_max_array[N_CALIB_REP];

static bool detectou_saida_min = false;
static bool detectou_saida_max = false;

static unsigned short int angulo_prev = 0;

void TBI_Calibration_Step(void)
{
    switch (calib_state)
    {
        case CALIB_IDLE:
            /* Garante que a valvula esteja em repouso antes de iniciar a calibracao */
            CDD_TBI_SetPWM(0);
            cont++;
            if (cont >= STEP_DELAY_COUNT)
            {
                /* Salva valor do ADC e passa para varredura positiva */
                duty = 0 + OFFSET_ABERTURA;
                cont = 0;
                adc_lh_repouso = (uint16_t)tps_filtered_value;
                adc_lh_repouso_array[calib_count] = adc_lh_repouso;
                angulo_lh_repouso = Get16u_RTE_deg_TPSAnglePosition();
                angulo_lh_repouso_array[calib_count] = angulo_lh_repouso;
                detectou_saida_min = false;
                detectou_saida_max = false;
                calib_state = CALIB_ABRINDO_RUN;
                saiu_lh = false;
            }
            break;

        case CALIB_ABRINDO_RUN:
            /* Executa uma varredura positiva, aumentando gradualmente o PWM */
            cont++;
            if (cont >= STEP_DELAY_COUNT)
            {
                cont = 0;
                adc_now = (uint16_t)tps_filtered_value;

                /* Calculo da diferenca com relacao ao valor de repouso (sempre positiva) */
                if (adc_now > adc_lh_repouso)
                {
                    delta = adc_now - adc_lh_repouso;
                }
                else
                {
                    delta = adc_lh_repouso - adc_now;
                }

                /* Se delta excede o threshold, registramos o ponto de saida do limp-home */
                if((saiu_lh == false) && (delta >= THRESH_LIMP_EXIT))
                {
                    pwm_lh_max = duty - PWM_STEP;
                    adc_lh_max = adc_prev;
                    angulo_lh_max = angulo_prev;
                    saiu_lh = true;
                }

                /* Se delta ultrapassa o limite de saturacao, contamos confirmacoes */
                if (delta > THRESH_MAX_LIMIT_POS)
                {
                    cont_saturado++;
                    if (cont_saturado >= MAX_SATURATION_CONFIRMATION)
                    {
                        cont_saturado = 0;

                        /* Curso superior atingido, salvar valores */
                        pwm_curso_max = duty - PWM_STEP;
                        adc_curso_max = adc_now;
                        angulo_curso_max = Get16u_RTE_deg_TPSAnglePosition();

                        adc_lh_max_array[calib_count] = adc_lh_max;
                        adc_curso_max_array[calib_count] = adc_curso_max;
                        pwm_lh_max_array[calib_count] = pwm_lh_max;
                        pwm_curso_max_array[calib_count] = pwm_curso_max;
                        angulo_lh_max_array[calib_count] = angulo_lh_max;
                        angulo_curso_max_array[calib_count] = angulo_curso_max;

                        calib_state = CALIB_TRANSICAO_FECHAMENTO;
                    }
                }

                adc_prev = adc_now;
                angulo_prev = Get16u_RTE_deg_TPSAnglePosition();
                duty += PWM_STEP;

                /* Saturacao do PWM (limita valores maximos) */
                if (duty < -70)
                {
                    duty = -70;
                }
                if (duty > 70)
                {
                    duty = 70;
                }

                /* Aplica PWM */
                CDD_TBI_SetPWM(duty * 10);
            }
            break;

        case CALIB_TRANSICAO_FECHAMENTO:
            cont++;
            if (cont >= STEP_DELAY_COUNT)
            {
                cont = 0;
                adc_now = (uint16_t)tps_filtered_value;

                /* Calculo da diferenca com relacao ao valor de repouso (sempre positiva) */
                if (adc_now > adc_curso_max)
                {
                    delta = adc_now - adc_curso_max;
                }
                else
                {
                    delta = adc_curso_max - adc_now;
                }

                if (delta > THRESH_MOV_SAIU_MAX && !detectou_saida_max)
                {
                    pwm_saiu_max = duty;
                    pwm_saiu_max_array[calib_count] = pwm_saiu_max;
                    detectou_saida_max = true;
                }

                /* Verifica o delta entre o adc_now e o limp home */
                if (adc_now > adc_lh_repouso)
                {
                    delta = adc_now - adc_lh_repouso;
                }
                else
                {
                    delta = adc_lh_repouso - adc_now;
                }

                /* Quando chega a zero ou proximo ao limp home, passa para varredura negativa */
                if ((duty == 0) || (delta < THRESH_RETORNO_REPOUSO))
                {
                    CDD_TBI_SetPWM(0);
                    duty = 0 - OFFSET_FECHAMENTO;
                    saiu_lh = false;
                    cont = 0;
                    calib_state = CALIB_FECHANDO_RUN;
                }
                else
                {
                    duty -= PWM_STEP;
                    CDD_TBI_SetPWM(duty * 10);
                }
            }
            break;

        case CALIB_FECHANDO_RUN:
            /* Executa varredura negativa (fechando borboleta) */
            cont++;
            if (cont >= STEP_DELAY_COUNT)
            {
                cont = 0;
                adc_now = (uint16_t)tps_filtered_value;

                /* Diferenca absoluta com relacao ao valor de repouso */
                if (adc_now > adc_lh_repouso)
                {
                    delta = adc_now - adc_lh_repouso;
                }
                else
                {
                    delta = adc_lh_repouso - adc_now;
                }

                /* Registra ponto de saida do limp-home inferior */
                if ((saiu_lh == false) && (delta >= THRESH_LIMP_EXIT))
                {
                    pwm_lh_min = duty + PWM_STEP;
                    adc_lh_min = adc_prev;
                    angulo_lh_min = angulo_prev;
                    saiu_lh = true;
                }

                /* Quando chega ao curso inferior, salva os dados */
                if (delta > THRESH_MAX_LIMIT_NEG)
                {
                    cont_saturado++;
                    if (cont_saturado >= MAX_SATURATION_CONFIRMATION)
                    {
                        cont_saturado = 0;

                        pwm_curso_min = duty + PWM_STEP;
                        adc_curso_min = adc_now;
                        angulo_curso_min = Get16u_RTE_deg_TPSAnglePosition();

                        adc_lh_min_array[calib_count] = adc_lh_min;
                        adc_curso_min_array[calib_count] = adc_curso_min;
                        pwm_lh_min_array[calib_count] = pwm_lh_min;
                        pwm_curso_min_array[calib_count] = pwm_curso_min;
                        angulo_lh_min_array[calib_count] = angulo_lh_min;
                        angulo_curso_min_array[calib_count] = angulo_curso_min;

                        calib_state = CALIB_TRANSICAO_ABERTURA;
                    }
                }

                angulo_prev = Get16u_RTE_deg_TPSAnglePosition();
                adc_prev = adc_now;
                duty -= PWM_STEP;

                /* Saturacao do PWM (limita valores minimos) */
                if (duty < -70)
                {
                    duty = -70;
                }
                if (duty > 70)
                {
                    duty = 70;
                }

                /* Aplica PWM */
                CDD_TBI_SetPWM(duty * 10);
            }
            break;

        case CALIB_TRANSICAO_ABERTURA:
            cont++;
            if (cont >= STEP_DELAY_COUNT)
            {
                cont = 0;
                adc_now = (uint16_t)tps_filtered_value;

                /* Calculo da diferenca com relacao ao valor de repouso (sempre positiva) */
                if (adc_now > adc_curso_min)
                {
                    delta = adc_now - adc_curso_min;
                }
                else
                {
                    delta = adc_curso_min - adc_now;
                }

                if (delta > THRESH_MOV_SAIU_MIN && !detectou_saida_min)
                {
                    pwm_saiu_min = duty;
                    pwm_saiu_min_array[calib_count] = pwm_saiu_min;
                    detectou_saida_min = true;
                }

                /* Verifica o delta entre o adc_now e o limp home */
                if (adc_now > adc_lh_repouso)
                {
                    delta = adc_now - adc_lh_repouso;
                }
                else
                {
                    delta = adc_lh_repouso - adc_now;
                }

                /* Quando chega a zero ou proximo ao limp home, terminou o processo */
                if ((duty == 0) || (delta < THRESH_RETORNO_REPOUSO))
                {
                    duty = 0;
                    CDD_TBI_SetPWM(duty * 10);
                    saiu_lh = false;
                    cont = 0;
                    calib_state = CALIB_MEDIA_FINAL;
                }
                else
                {
                    duty += PWM_STEP;
                    CDD_TBI_SetPWM(duty * 10);
                }
            }
            break;

        case CALIB_MEDIA_FINAL:
            /* Apos cada ciclo completo (abertura + fechamento), avanca contador */
            calib_count++;
            if (calib_count >= N_CALIB_REP)
            {
                /* Realiza a media dos dados acumulados */
                uint32_t adc_lh_max_sum = 0, adc_curso_max_sum = 0;
                uint32_t adc_lh_min_sum = 0, adc_curso_min_sum = 0;
                uint32_t adc_lh_repouso_sum = 0;
                int32_t  pwm_lh_max_sum = 0, pwm_curso_max_sum = 0;
                int32_t  pwm_lh_min_sum = 0, pwm_curso_min_sum = 0;
                uint32_t angulo_lh_max_sum = 0, angulo_curso_max_sum = 0;
                uint32_t angulo_lh_min_sum = 0, angulo_curso_min_sum = 0;
                uint32_t angulo_lh_repouso_sum = 0;
                int32_t pwm_saiu_min_sum = 0;
                int32_t pwm_saiu_max_sum = 0;

                for (uint8_t i = 0; i < N_CALIB_REP; i++)
                {
                    adc_lh_max_sum     += adc_lh_max_array[i];
                    adc_curso_max_sum  += adc_curso_max_array[i];
                    pwm_lh_max_sum     += pwm_lh_max_array[i];
                    pwm_curso_max_sum  += pwm_curso_max_array[i];
                    adc_lh_min_sum     += adc_lh_min_array[i];
                    adc_curso_min_sum  += adc_curso_min_array[i];
                    pwm_lh_min_sum     += pwm_lh_min_array[i];
                    pwm_curso_min_sum  += pwm_curso_min_array[i];
                    adc_lh_repouso_sum += adc_lh_repouso_array[i];
                    angulo_lh_max_sum     += angulo_lh_max_array[i];
                    angulo_curso_max_sum  += angulo_curso_max_array[i];
                    angulo_lh_min_sum     += angulo_lh_min_array[i];
                    angulo_curso_min_sum  += angulo_curso_min_array[i];
                    angulo_lh_repouso_sum += angulo_lh_repouso_array[i];
                    pwm_saiu_min_sum += pwm_saiu_min_array[i];
                    pwm_saiu_max_sum += pwm_saiu_max_array[i];
                }

                /* Salva valores medios finais */
                adc_lh_max     = adc_lh_max_sum / N_CALIB_REP;
                adc_curso_max  = adc_curso_max_sum / N_CALIB_REP;
                pwm_lh_max     = pwm_lh_max_sum / N_CALIB_REP;
                pwm_curso_max  = pwm_curso_max_sum / N_CALIB_REP;
                adc_lh_min     = adc_lh_min_sum / N_CALIB_REP;
                adc_curso_min  = adc_curso_min_sum / N_CALIB_REP;
                pwm_lh_min     = pwm_lh_min_sum / N_CALIB_REP;
                pwm_curso_min  = pwm_curso_min_sum / N_CALIB_REP;
                adc_lh_repouso = adc_lh_repouso_sum / N_CALIB_REP;
                angulo_lh_max     = angulo_lh_max_sum / N_CALIB_REP;
                angulo_curso_max  = angulo_curso_max_sum / N_CALIB_REP;
                angulo_lh_min     = angulo_lh_min_sum / N_CALIB_REP;
                angulo_curso_min  = angulo_curso_min_sum / N_CALIB_REP;
                angulo_lh_repouso = angulo_lh_repouso_sum / N_CALIB_REP;
                pwm_saiu_min = pwm_saiu_min_sum / N_CALIB_REP;
                pwm_saiu_max = pwm_saiu_max_sum / N_CALIB_REP;

                calib_count = 0;
                calib_state = CALIB_FINALIZADO;
            }
            else
            {
                /* Reinicia novo ciclo se ainda faltam repeticoes */
                calib_state = CALIB_IDLE;
            }
            break;

        case CALIB_FINALIZADO:
            /* Mantem PWM retornando suavemente para zero */
            if (duty > 0)
            {
                duty -= 1;
            }
            if (duty < 0)
            {
                duty += 1;
            }
            CDD_TBI_SetPWM(duty * 10);
            break;

        case CALIB_ABRINDO_INIT:
        case CALIB_FECHANDO_INIT:
        default:
            break;
    }
}
