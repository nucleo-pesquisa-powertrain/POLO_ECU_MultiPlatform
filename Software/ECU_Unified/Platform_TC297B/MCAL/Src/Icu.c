/**
 * \file Icu.c
 * \brief Implementacao MCAL ICU para Infineon AURIX TC297B
 *
 * Delega a configuracao do ERU para initPeripheralsAndERU() em
 * HardwareAdp/ERU_Interrupt.c, que ja contem a ISR funcional
 * chamando CDD_TriggerWheel_Event() e CDD_SYNC_Timing_Event().
 *
 * O timestamp de alta resolucao usa o STM0 free-running a 100 MHz.
 *
 * Plataforma: Infineon AURIX TC297B
 */

#include "Icu.h"

/* iLLD - STM (System Timer Module) */
#include "IfxStm.h"
#include "IfxStm_reg.h"

/* HardwareAdp - ERU ja configurado e funcional */
#include "ERU_Interrupt.h"

/* ------------------------------------------------------------------ */
/* Estado interno                                                      */
/* ------------------------------------------------------------------ */

static Icu_EdgeCallbackType Icu_CrankCallback = (Icu_EdgeCallbackType)0;

/* ------------------------------------------------------------------ */
/* API                                                                 */
/* ------------------------------------------------------------------ */

void Icu_Init(void)
{
    /* STM0 ja ativo por default apos reset do TC297B (100 MHz).
     * ERU configurado por initPeripheralsAndERU() que tambem
     * registra a ISR (SCUERU_Int0_Handler) com as chamadas
     * CDD_TriggerWheel_Event() e CDD_SYNC_Timing_Event(). */
    initPeripheralsAndERU();
}

uint32 Icu_GetTimestamp_us(void)
{
    return IfxStm_getLower(&MODULE_STM0) / 100u;
}

void Icu_SetEdgeCallback(Icu_EdgeCallbackType callback)
{
    /* No TC297B, o callback e' hardcoded em ERU_Interrupt.c.
     * Armazena o ponteiro para compatibilidade com a API. */
    Icu_CrankCallback = callback;
    (void)Icu_CrankCallback; /* suppress unused warning */
}

void Icu_EnableEdgeDetection(void)
{
    /* ERU ja habilitado por initPeripheralsAndERU() */
}

void Icu_DisableEdgeDetection(void)
{
    /* Desabilitar o ERU em runtime nao e' necessario neste projeto */
}
