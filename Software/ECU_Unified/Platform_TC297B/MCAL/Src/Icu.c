/**
 * \file Icu.c
 * \brief Implementacao MCAL ICU para Infineon AURIX TC297B
 *
 * Fornece duas funcionalidades distintas:
 *
 * 1. TIMESTAMP DE ALTA RESOLUCAO (IfxStm):
 *    O modulo STM0 (System Timer Module) opera como contador livre de
 *    64 bits a 100 MHz. Esta implementacao usa apenas os 32 bits inferiores
 *    (IfxStm_getLower), suficientes para ~42 segundos antes de overflow.
 *    Conversao: us = ticks_stm / 100
 *
 * 2. DETECCAO DE BORDA DO CRANKSHAFT (ERU):
 *    O ERU (Event Request Unit) detecta bordas no sinal do sensor de
 *    rotacao (roda fonica 60-2 dentes). A cada borda, o ISR chama o
 *    callback registrado via Icu_SetEdgeCallback().
 *
 *    Configuracao ERU:
 *    - Sinal de entrada: pino do sensor (ver Dio_Cfg.h - PHASE_STATE ou
 *      pino dedicado ao VR sensor, dependendo do hardware)
 *    - Borda detectada: subida (frente de subida do dente)
 *    - Servico: CPU0, prioridade ICU_IRQ_PRIO_CRANK
 *
 * O callback registrado deve ser CDD_SYNC_Timing_Event() que processa
 * o evento de dente da roda fonica internamente.
 *
 * Plataforma: Infineon AURIX TC297B
 * Driver iLLD: IfxStm (timestamp), IfxScuEru (deteccao de borda)
 */

/* ------------------------------------------------------------------ */
/* Includes                                                            */
/* ------------------------------------------------------------------ */

#include "Icu.h"
#include "Mcal_Compiler.h"

/* iLLD - STM (System Timer Module) */
#include "IfxStm.h"
#include "IfxStm_reg.h"

/* iLLD - ERU (Event Request Unit) */
#include "IfxScuEru.h"

/* ------------------------------------------------------------------ */
/* Configuracao de interrupcao ERU                                     */
/* ------------------------------------------------------------------ */

/** Prioridade da interrupcao ERU para deteccao de borda do crankshaft */
#define ICU_IRQ_PRIO_CRANK      40u

/**
 * \brief Canal ERU utilizado para o crankshaft
 *
 * ERU0 possui 8 canais de entrada (ETL0..ETL7) e 4 canais de saida
 * (OGU0..OGU3) com respectivos servi pos de interrupcao (SCUERU SRC).
 * Canal ETL1 -> OGU1 -> SRC_SCU_ERU1 (configuravel no BSP).
 */
#define ICU_ERU_INPUT_CHANNEL   IfxScuEru_InputChannel_1
#define ICU_ERU_OUTPUT_CHANNEL  IfxScuEru_OutputChannel_1
#define ICU_ERU_INPUT_LINE      IfxScuEru_InputLine_0   /* Mapeado ao pino do sensor */

/* ------------------------------------------------------------------ */
/* Estado interno                                                      */
/* ------------------------------------------------------------------ */

/** Callback para borda do crankshaft */
static Icu_EdgeCallbackType Icu_CrankCallback = (Icu_EdgeCallbackType)0;

/** Flag para controle de habilitacao da interrupcao ERU */
static boolean Icu_EdgeDetectionEnabled = FALSE;

/* ------------------------------------------------------------------ */
/* Implementacao da API                                                */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo ICU
 *
 * Configura o STM0 (ja ativo por default apos reset do TC297B) e
 * inicializa o ERU para deteccao de borda no sinal do crankshaft.
 *
 * O STM0 e' iniciado pelo hardware apos reset e nao requer configuracao
 * adicional para uso como contador livre de 100 MHz.
 *
 * O ERU e' configurado para borda de subida. A interrupcao fica
 * desabilitada ate Icu_EnableEdgeDetection() ser chamado.
 */
void Icu_Init(void)
{
    IfxScuEru_InputConfig  eruInputCfg;
    IfxScuEru_OutputConfig eruOutputCfg;

    /* --- STM0 --- */
    /*
     * O STM0 e' habilitado por default no TC297B e opera a f_STM = f_SPB = 100 MHz.
     * Nenhuma configuracao adicional e' necessaria para uso como free-running timer.
     */

    /* --- ERU - Configuracao do canal de entrada (ETL) --- */
    /*
     * ETL (Event Trigger Logic): filtragem e deteccao de borda.
     * Configurado para borda de subida do sinal do crankshaft.
     */
    IfxScuEru_initInputConfig(&eruInputCfg);
    eruInputCfg.inputChannel        = ICU_ERU_INPUT_CHANNEL;
    eruInputCfg.inputLine           = ICU_ERU_INPUT_LINE;
    eruInputCfg.risingEdgeEnabled   = TRUE;     /* Detecta borda de subida */
    eruInputCfg.fallingEdgeEnabled  = FALSE;    /* Ignora borda de descida */
    IfxScuEru_initInput(&eruInputCfg);

    /* --- ERU - Configuracao do canal de saida (OGU) --- */
    /*
     * OGU (Output Gating Unit): gera a requisicao de interrupcao.
     */
    IfxScuEru_initOutputConfig(&eruOutputCfg);
    eruOutputCfg.inputChannel       = ICU_ERU_INPUT_CHANNEL;
    eruOutputCfg.outputChannel      = ICU_ERU_OUTPUT_CHANNEL;
    eruOutputCfg.interruptGating    = IfxScuEru_InterruptGating_always;
    IfxScuEru_initOutput(&eruOutputCfg);

    /* --- Configura vetor de interrupcao ERU --- */
    /*
     * SRC_SCUERU1 corresponde ao OGU1 do ERU0.
     * Atribuido ao CPU0, prioridade ICU_IRQ_PRIO_CRANK.
     * A interrupcao permanece desabilitada ate Icu_EnableEdgeDetection().
     */
    volatile Ifx_SRC_SRCR* eruSrc = &MODULE_SRC.SCU.SCU.ERU[(uint32)ICU_ERU_OUTPUT_CHANNEL];
    IfxSrc_init(eruSrc, IfxSrc_Tos_cpu0, ICU_IRQ_PRIO_CRANK);
    /* Nao habilita aqui - aguarda Icu_EnableEdgeDetection() */

    /* Inicializa estado */
    Icu_CrankCallback       = (Icu_EdgeCallbackType)0;
    Icu_EdgeDetectionEnabled = FALSE;
}

/**
 * \brief Retorna timestamp livre em microssegundos
 *
 * Le o registrador STM0_TIM0 (lower 32 bits do contador de 64 bits)
 * e converte para us dividindo por 100 (100 MHz / 100 = 1 tick/us).
 *
 * Overflow a cada ~42.9 segundos (2^32 / 100 us).
 *
 * \return Timestamp em us
 */
uint32 Icu_GetTimestamp_us(void)
{
    /*
     * IfxStm_getLower() le o registrador TIM0 (32 bits inferiores do STM).
     * Divisao por 100 converte de ticks a 100MHz para microssegundos.
     * A divisao e' inteira, portanto a resolucao efetiva e' 1 us.
     */
    return IfxStm_getLower(&MODULE_STM0) / 100u;
}

/**
 * \brief Registra callback para borda do crankshaft
 *
 * \param callback Funcao chamada a cada dente detectado (contexto ISR)
 *
 * Tipicamente aponta para CDD_SYNC_Timing_Event().
 */
void Icu_SetEdgeCallback(Icu_EdgeCallbackType callback)
{
    Mcal_DisableAllInterrupts();
    Icu_CrankCallback = callback;
    Mcal_EnableAllInterrupts();
}

/**
 * \brief Habilita a deteccao de borda do crankshaft
 *
 * Habilita o SRC do ERU para que as bordas gerem interrupcoes.
 */
void Icu_EnableEdgeDetection(void)
{
    volatile Ifx_SRC_SRCR* eruSrc = &MODULE_SRC.SCU.SCU.ERU[(uint32)ICU_ERU_OUTPUT_CHANNEL];
    IfxSrc_enable(eruSrc);
    Icu_EdgeDetectionEnabled = TRUE;
}

/**
 * \brief Desabilita a deteccao de borda do crankshaft
 *
 * Desabilita o SRC do ERU. Bordas subsequentes nao gerarao interrupcoes
 * ate Icu_EnableEdgeDetection() ser chamado novamente.
 */
void Icu_DisableEdgeDetection(void)
{
    volatile Ifx_SRC_SRCR* eruSrc = &MODULE_SRC.SCU.SCU.ERU[(uint32)ICU_ERU_OUTPUT_CHANNEL];
    IfxSrc_disable(eruSrc);
    Icu_EdgeDetectionEnabled = FALSE;
}

/* ------------------------------------------------------------------ */
/* Rotina de servico de interrupcao (ISR) do ERU                      */
/* ------------------------------------------------------------------ */

/**
 * \brief ISR de deteccao de borda do crankshaft (ERU OGU1)
 *
 * Chamado a cada borda de subida detectada no sinal do sensor de rotacao.
 * Apenas encaminha para o callback registrado.
 *
 * Vetor: SRC_SCUERU1 (ERU0 OGU1)
 * Prioridade: ICU_IRQ_PRIO_CRANK
 *
 * ATENCAO: Esta ISR deve ter latencia minima. O callback (CDD_SYNC_Timing_Event)
 * e' responsavel por toda a logica de sincronismo.
 */
IFX_INTERRUPT(Icu_IsrCrankEdge, 0, ICU_IRQ_PRIO_CRANK)
{
    if (Icu_CrankCallback != (Icu_EdgeCallbackType)0)
    {
        Icu_CrankCallback();
    }
}
