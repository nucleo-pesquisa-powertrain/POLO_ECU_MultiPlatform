/**
 * \file cdd_tbi.c
 * \brief CDD - Controle da Borboleta Eletronica (TBI - Throttle Body Injector)
 *
 * Implementa o controle do driver MC33186 para a borboleta eletronica.
 * A direcao de movimento e' controlada pela saida digital DIO_CH_MC33186_IN1
 * e a intensidade pelo canal PWM_CH_TBI via MCAL Pwm_SetDutyCycle().
 *
 * Dependencias iLLD/HAL removidas:
 *   - hal_discrete_outputs.h  (substituido por Dio.h)
 *   - CCU6_PWM_Generation.h   (substituido por Pwm.h)
 *   - GTM_ATOM_PWM.h          (substituido por Pwm.h)
 *   - IfxPort.h               (substituido por Dio.h / constantes DIO_HIGH/DIO_LOW)
 *
 * Mapeamento de saidas:
 *   DIO_CH_MC33186_COD  -> enable do driver (LOW = habilitado)
 *   DIO_CH_MC33186_DI1  -> selecao de direcao do motor
 *   DIO_CH_MC33186_DI2  -> controle complementar de direcao
 *   DIO_CH_MC33186_IN1  -> selecao de sentido de rotacao
 *   PWM_CH_TBI          -> canal PWM para controle de posicao
 *
 * Escala do PWM:
 *   Pwm_SetDutyCycle() recebe percentual inteiro (0..100).
 *   A conversao de pwmvalue para percentual e' feita internamente:
 *     duty = |pwmvalue| / 10  (|pwmvalue| em [-999..+999] => 0..99 %)
 *
 * Historico:
 *   2026-03-28  Refatorado: substituidas chamadas iLLD/HAL diretas por
 *               MCAL API (Dio.h para saidas discretas, Pwm.h para PWM).
 *               initGtmATomPwm() substituido por Pwm_Init(). setDutyCycle()
 *               substituido por Pwm_SetDutyCycle(). Logica de controle
 *               preservada identica ao original.
 */

#include "cdd_tbi.h"
#include "Dio.h"
#include "Pwm.h"

void CDD_TBI_Init(void)
{
    /* Inicializa o canal PWM da borboleta via MCAL */
    Pwm_Init();

    /* Configura o driver MC33186: COD=LOW (habilita), direcao inicial */
    Dio_WriteChannel(DIO_CH_MC33186_COD, DIO_LOW);
    Dio_WriteChannel(DIO_CH_MC33186_DI1, DIO_LOW);
    Dio_WriteChannel(DIO_CH_MC33186_DI2, DIO_HIGH);

    /* Inicializa com duty cycle neutro de 50% */
    Pwm_SetDutyCycle(PWM_CH_TBI, 50u);
}

void CDD_TBI_SetPWM(int pwmvalue)
{
    /* Saturacao do valor de controle na faixa [-999 .. +999] */
    if (pwmvalue >= 1000)
    {
        pwmvalue = 999;
    }
    else if (pwmvalue <= (-1000))
    {
        pwmvalue = -999;
    }
    else
    {
        /* Valor dentro da faixa: sem saturacao */
    }

    if (pwmvalue < 0)
    {
        /* Movimento negativo (fecha borboleta): DI1=LOW, duty = |pwmvalue|/10 % */
        pwmvalue *= -1;
        Dio_WriteChannel(DIO_CH_MC33186_IN1, DIO_LOW);
        Pwm_SetDutyCycle(PWM_CH_TBI, (unsigned int)(pwmvalue / 10));
    }
    else if (pwmvalue > 0)
    {
        /* Movimento positivo (abre borboleta): DI1=HIGH, duty = (1000-pwmvalue)/10 % */
        pwmvalue = (1000 - pwmvalue);
        Dio_WriteChannel(DIO_CH_MC33186_IN1, DIO_HIGH);
        Pwm_SetDutyCycle(PWM_CH_TBI, (unsigned int)(pwmvalue / 10));
    }
    else
    {
        /* Zero: sem movimento, duty minimo de 1% para manter driver ativo */
        Dio_WriteChannel(DIO_CH_MC33186_IN1, DIO_LOW);
        Pwm_SetDutyCycle(PWM_CH_TBI, 1u);
    }
}
