/**
 * \file Pwm.c
 * \brief Implementacao MCAL PWM para STM32H745
 *
 * Implementa a API Pwm.h usando TIM HAL em modo PWM (Output Compare).
 *
 * Canais PWM implementados:
 *   PWM_CH_THROTTLE (0): Ponte H da borboleta eletronica (MC33186)
 *                        TIM8 CH1 (ou TIM1 CH1 - 32-bit avancado)
 *                        Frequencia: 20 kHz (imperceptivel ao ouvido humano)
 *                        TODO: Ajustar frequencia conforme MC33186 datasheet
 *
 *   PWM_CH_CANISTER (1): Valvula do canister de carvao ativo
 *                        TIM12 CH1 (ou outro timer disponivel)
 *                        Frequencia: 50 Hz (tipico para solenoide)
 *                        TODO: Confirmar frequencia de controle da valvula
 *
 * Escala de duty cycle: 0..10000 = 0.00%..100.00% (resolucao 0.01%)
 * Formula: CCR = (duty * Period) / 10000
 *
 * Configuracao de frequencia (TIM8, APB2 clock = 200 MHz, Prescaler = 0):
 *   f_PWM = ClockFreq / ((Prescaler+1) * (Period+1))
 *   Para 20 kHz: Period = 200MHz / 20kHz - 1 = 9999
 *   Para 50 Hz:  Period = 200MHz / 50Hz - 1 = 3999999 (excede 16-bit!)
 *                Usar prescaler: PSC=199, Period=19999
 *                f = 200MHz / (200 * 20000) = 50 Hz [OK]
 *
 * Plataforma : STM32H745
 * HAL        : stm32h7xx_hal_tim.h
 *
 * \note TODO: No CubeMX, configurar os timers PWM com:
 *             TIM8 CH1: PWM Generation, Prescaler=0, Period=9999
 *             TIM12 CH1: PWM Generation, Prescaler=199, Period=19999
 *             Habilitar os canais de saida (Output Compare Channel)
 *             e mapear para os pinos corretos no esquematico.
 */

#include "Pwm.h"
#include "stm32h7xx_hal.h"

/* ------------------------------------------------------------------ */
/* Handles dos timers PWM (gerados pelo CubeMX)                      */
/* ------------------------------------------------------------------ */

/**
 * Handles dos timers usados para PWM.
 * TODO: Confirmar nomes gerados pelo CubeMX (htim8, htim12).
 *       Ajustar se timers diferentes forem usados.
 */
extern TIM_HandleTypeDef htim8;   /* PWM_CH_THROTTLE - 20 kHz */
extern TIM_HandleTypeDef htim12;  /* PWM_CH_CANISTER - 50 Hz  */

/* ------------------------------------------------------------------ */
/* Constantes de configuracao PWM                                    */
/* ------------------------------------------------------------------ */

/**
 * Periodo do timer para PWM da borboleta (20 kHz).
 * TIM8, APB2 = 200 MHz, Prescaler = 0:
 * Period = 200.000.000 / 20.000 - 1 = 9999
 * Resolucao do duty: 1/10000 = 0.01%
 */
#define PWM_THROTTLE_PERIOD         9999U

/**
 * Periodo do timer para valvula do canister (50 Hz).
 * TIM12, APB1 = 200 MHz, Prescaler = 199:
 * f_timer = 200MHz / 200 = 1 MHz
 * Period = 1.000.000 / 50 - 1 = 19999
 */
#define PWM_CANISTER_PERIOD         19999U

/**
 * Canal HAL de Output Compare para cada timer.
 * TIM8 CH1 e TIM12 CH1.
 * TODO: Ajustar se o CubeMX usar canais diferentes.
 */
#define PWM_THROTTLE_CHANNEL        TIM_CHANNEL_1
#define PWM_CANISTER_CHANNEL        TIM_CHANNEL_1

/**
 * Escala maxima de duty cycle da API (equivale a 100.00%).
 */
#define PWM_DUTY_MAX                10000U

/* ------------------------------------------------------------------ */
/* Tipo de configuracao interna de canal PWM                         */
/* ------------------------------------------------------------------ */

/**
 * Estrutura que associa um canal logico PWM ao timer fisico.
 */
typedef struct
{
    TIM_HandleTypeDef* handle;   /**< Handle HAL do timer              */
    uint32_t           channel;  /**< Canal TIM (TIM_CHANNEL_x)        */
    uint32_t           period;   /**< Periodo do timer (valor do ARR)  */
} Pwm_ChannelConfigType;

/**
 * Tabela de configuracao indexada por PWM_CH_*.
 * TODO: Ajustar handles e periodos conforme projeto CubeMX.
 */
static const Pwm_ChannelConfigType Pwm_ChannelCfg[PWM_NUM_CHANNELS] =
{
    /* [PWM_CH_THROTTLE = 0] */
    {
        &htim8,
        PWM_THROTTLE_CHANNEL,
        PWM_THROTTLE_PERIOD
    },

    /* [PWM_CH_CANISTER = 1] */
    {
        &htim12,
        PWM_CANISTER_CHANNEL,
        PWM_CANISTER_PERIOD
    },
};

/* ------------------------------------------------------------------ */
/* Estado do modulo                                                   */
/* ------------------------------------------------------------------ */

/** Armazena o ultimo duty cycle configurado por canal (0..10000) */
static uint16_t Pwm_CurrentDuty[PWM_NUM_CHANNELS];

/** Flag de inicializacao */
static uint8_t Pwm_Initialized = 0U;

/* ------------------------------------------------------------------ */
/* Funcoes privadas                                                   */
/* ------------------------------------------------------------------ */

/**
 * Converte duty cycle (0..10000) para valor CCR do timer.
 * Formula: CCR = (duty * (Period + 1)) / PWM_DUTY_MAX
 *
 * Evita overflow: duty <= 10000, Period <= 19999.
 * Produto maximo: 10000 * 20000 = 200.000.000 < 2^32 => sem overflow uint32.
 *
 * \param duty   Duty cycle (0..10000)
 * \param period Periodo do timer (ARR value)
 * \return Valor de comparacao (CCR)
 */
static uint32_t Pwm_DutyCycle_to_CCR(uint16_t duty, uint32_t period)
{
    uint32_t ccr;

    if (duty >= (uint16_t)PWM_DUTY_MAX)
    {
        return period; /* 100% duty: CCR = Period (sinal sempre alto) */
    }

    if (duty == 0U)
    {
        return 0U; /* 0% duty: CCR = 0 (sinal sempre baixo) */
    }

    /* CCR = duty * (period + 1) / 10000 */
    ccr = ((uint32_t)duty * (period + 1U)) / (uint32_t)PWM_DUTY_MAX;

    return ccr;
}

/* ------------------------------------------------------------------ */
/* Implementacao da API publica                                       */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo PWM.
 *
 * Configura os timers PWM e carrega duty cycle inicial = 0% (saidas inativas).
 * Nao inicia a geracao de PWM (chamar Pwm_Start() por canal).
 *
 * TODO: O CubeMX gera MX_TIM8_Init() e MX_TIM12_Init(). Esta funcao
 *       apenas configura o estado inicial e nao sobrescreve a init do CubeMX.
 */
void Pwm_Init(void)
{
    uint8_t ch;

    if (Pwm_Initialized != 0U)
    {
        return;
    }

    /* Inicializa duty cycle em zero para todas as saidas PWM */
    for (ch = 0U; ch < PWM_NUM_CHANNELS; ch++)
    {
        Pwm_CurrentDuty[ch] = 0U;

        /* Carrega CCR = 0 (duty 0%) nos timers antes de iniciar.
         * Previne pulso espurio ao ligar o PWM pela primeira vez. */
        __HAL_TIM_SET_COMPARE(Pwm_ChannelCfg[ch].handle,
                               Pwm_ChannelCfg[ch].channel,
                               0U);
    }

    Pwm_Initialized = 1U;
}

/**
 * \brief Configura duty cycle de um canal PWM.
 *
 * Atualiza o registrador CCR do timer correspondente.
 * Se o PWM ja' estiver rodando (Pwm_Start chamado), a alteracao
 * tem efeito no proximo periodo (com PRELOAD habilitado no CubeMX).
 *
 * \param ch   Canal (PWM_CH_*)
 * \param duty Duty cycle (0..10000 = 0.00%..100.00%)
 */
void Pwm_SetDutyCycle(Pwm_ChannelType ch, uint16 duty)
{
    uint32_t ccr;

    if (ch >= PWM_NUM_CHANNELS)
    {
        return;
    }

    /* Satura o duty cycle dentro do range valido */
    if (duty > (uint16_t)PWM_DUTY_MAX)
    {
        duty = (uint16_t)PWM_DUTY_MAX;
    }

    /* Salva o duty atual (para leitura de estado se necessario) */
    Pwm_CurrentDuty[ch] = duty;

    /* Calcula e carrega o valor de comparacao */
    ccr = Pwm_DutyCycle_to_CCR(duty, Pwm_ChannelCfg[ch].period);

    __HAL_TIM_SET_COMPARE(Pwm_ChannelCfg[ch].handle,
                           Pwm_ChannelCfg[ch].channel,
                           ccr);
}

/**
 * \brief Inicia a geracao de PWM num canal.
 *
 * Chama HAL_TIM_PWM_Start() para habilitar a saida PWM do canal.
 * O duty cycle deve ser configurado antes via Pwm_SetDutyCycle().
 *
 * Para timers avancados (TIM8), e' necessario chamar tambem
 * HAL_TIMEx_PWMN_Start() para o canal complementar se usado.
 *
 * \param ch Canal (PWM_CH_*)
 */
void Pwm_Start(Pwm_ChannelType ch)
{
    if (ch >= PWM_NUM_CHANNELS)
    {
        return;
    }

    if (Pwm_Initialized == 0U)
    {
        Pwm_Init();
    }

    /* Inicia a geracao de PWM. Para timers avancados (TIM1, TIM8),
     * o MOE bit (Main Output Enable) e' ativado pelo HAL automaticamente. */
    (void)HAL_TIM_PWM_Start(Pwm_ChannelCfg[ch].handle,
                              Pwm_ChannelCfg[ch].channel);

    /* TODO: Se usar canal complementar (ex: TIM8 CH1N para half-bridge),
     *       adicionar: HAL_TIMEx_PWMN_Start(handle, channel); */
}

/**
 * \brief Para a geracao de PWM num canal.
 *
 * Para o PWM e mantem a saida no estado definido pelo registrador IDLE
 * do timer (LOW por padrao, configuravel no CubeMX).
 *
 * Para garantir saida LOW segura ao parar o PWM (ex: borboleta fechada),
 * chamar Pwm_SetDutyCycle(ch, 0) antes de Pwm_Stop().
 *
 * \param ch Canal (PWM_CH_*)
 */
void Pwm_Stop(Pwm_ChannelType ch)
{
    if (ch >= PWM_NUM_CHANNELS)
    {
        return;
    }

    /* Para o PWM: a saida vai para o nivel IDLE (geralmente LOW).
     * Para TIM8 (timer avancado), o HAL_TIM_PWM_Stop desabilita
     * o canal mas mantem o MOE ativo ate' que todos os canais parem. */
    (void)HAL_TIM_PWM_Stop(Pwm_ChannelCfg[ch].handle,
                             Pwm_ChannelCfg[ch].channel);

    /* Garante CCR = 0 para evitar comportamento inesperado ao reiniciar */
    __HAL_TIM_SET_COMPARE(Pwm_ChannelCfg[ch].handle,
                           Pwm_ChannelCfg[ch].channel,
                           0U);

    Pwm_CurrentDuty[ch] = 0U;
}
