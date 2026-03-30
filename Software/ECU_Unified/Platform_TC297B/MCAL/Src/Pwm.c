/**
 * \file Pwm.c
 * \brief Implementacao MCAL PWM para TC297B - delega para GTM_ATOM_PWM.c (HardwareAdp)
 */

#include "Pwm.h"
#include "GTM_ATOM_PWM.h"
#include "CCU6_PWM_Generation.h"

void Pwm_Init(void)
{
    initGtmATomPwm();
}

void Pwm_SetDutyCycle(Pwm_ChannelType ch, uint16 duty)
{
    switch (ch)
    {
        case PWM_CH_THROTTLE:
            /* GTM ATOM PWM para borboleta - duty em % direto */
            setDutyCycle((unsigned long)duty);
            break;

        case PWM_CH_CANISTER:
            /* TODO: implementar canister via CCU6 se necessario */
            break;

        default:
            break;
    }
}

void Pwm_Start(Pwm_ChannelType ch)
{
    (void)ch; /* GTM ATOM ja inicia no init */
}

void Pwm_Stop(Pwm_ChannelType ch)
{
    if (ch == PWM_CH_THROTTLE)
    {
        setDutyCycle(0u);
    }
}
