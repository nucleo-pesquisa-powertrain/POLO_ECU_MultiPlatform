/**
 * \file Gpt.c
 * \brief Implementacao MCAL GPT para STM32H745
 *
 * Implementa a API Gpt.h usando TIM2..TIM5 do STM32H7 em modo one-shot
 * (contador up, interrupcao de Update, auto-reload nao repetitivo).
 *
 * Arquitetura dos timers:
 *   - TIM2 (32-bit, APB1): GPT_CH_IGN_TIMING  - avanco de ignicao
 *   - TIM3 (16-bit, APB1): GPT_CH_INJ_DURATION - duracao de injecao
 *   - TIM4 (16-bit, APB1): GPT_CH_INJ_TIMING  - atraso de injecao
 *   - TIM5 (32-bit, APB1): GPT_CH_DWELL       - dwell da bobina
 *
 * Resolucao: 1 us por tick (prescaler = 199, clock = 200 MHz)
 * Jitter esperado: < 1 us (interrupcao servida em < 10 us conforme spec)
 *
 * Modo de operacao one-shot:
 *   1. Gpt_StartTimer(ch, timeout_us): configura ARR = timeout_us,
 *      reseta CNT = 0, habilita TIMx (TIM_CR1_CEN).
 *   2. Ao atingir ARR, TIM gera evento Update (UEV).
 *   3. HAL_TIM_PeriodElapsedCallback e' chamado no contexto da ISR.
 *   4. O callback registrado pelo usuario e' disparado.
 *   5. O timer para automaticamente (OPM bit no CR1).
 *
 * CRITICO - Prioridade NVIC:
 *   Os timers de ignicao/injecao devem ter prioridade mais alta que
 *   todas as tasks de software para garantir jitter < 10 us.
 *   Usar prioridade 2 (abaixo apenas do EXTI do crankshaft em prio 1).
 *
 * Plataforma : STM32H745
 * HAL        : stm32h7xx_hal_tim.h
 *
 * \note TODO: No CubeMX, configurar TIM2..TIM5 com:
 *             - Clock Source: Internal Clock
 *             - Prescaler: 199 (= GPT_PRESCALER_VALUE)
 *             - Counter Mode: Up
 *             - One Pulse Mode: Enable
 *             - Auto-Reload Preload: Disable (para carga imediata do ARR)
 *             - Habilitar interrupcao de Update
 */

#include "Gpt.h"
#include "Gpt_Cfg.h"
#include "stm32h7xx_hal.h"

/* ------------------------------------------------------------------ */
/* Tabela de callbacks registrados                                    */
/* ------------------------------------------------------------------ */

/**
 * Array de callbacks indexado por canal GPT.
 * Inicializado com NULL; populado via Gpt_SetNotification().
 * Acessado dentro da ISR: deve ser lido atomicamente (ponteiro 32-bit
 * no Cortex-M e' leitura atomica).
 */
static volatile Gpt_NotificationType Gpt_Callbacks[GPT_NUM_CHANNELS];

/* ------------------------------------------------------------------ */
/* Estado dos timers                                                  */
/* ------------------------------------------------------------------ */

/**
 * Marca o instante (tick do timer) em que cada canal foi iniciado.
 * Usado por Gpt_GetElapsedTime_us para calcular tempo decorrido.
 * Os timers em modo one-shot incrementam de 0 ate ARR, entao param.
 * O valor do CNT ja e' o tempo decorrido diretamente (1 tick = 1 us).
 */
/* Nao necessario guardar start time pois o CNT ja representa us decorridos */

/* ------------------------------------------------------------------ */
/* Funcoes privadas                                                   */
/* ------------------------------------------------------------------ */

/**
 * Retorna o handle HAL correspondente ao canal GPT.
 * Inlining pelo compilador em builds otimizados.
 *
 * \param ch Canal GPT (GPT_CH_*)
 * \return Ponteiro para TIM_HandleTypeDef, ou NULL se invalido
 */
static TIM_HandleTypeDef* Gpt_GetHandle(Gpt_ChannelType ch)
{
    if (ch >= GPT_NUM_CHANNELS)
    {
        return NULL;
    }
    return Gpt_ChannelCfg[ch].handle;
}

/**
 * Configura o NVIC para o timer informado.
 * As prioridades sao definidas em Gpt_Cfg.h.
 *
 * \param instance Instancia do periferico TIM
 */
static void Gpt_ConfigNVIC(TIM_TypeDef* instance)
{
    IRQn_Type irqn;

    /* Mapeamento instancia -> IRQ number para APB1 timers */
    if      (instance == TIM2)  { irqn = TIM2_IRQn;  }
    else if (instance == TIM3)  { irqn = TIM3_IRQn;  }
    else if (instance == TIM4)  { irqn = TIM4_IRQn;  }
    else if (instance == TIM5)  { irqn = TIM5_IRQn;  }
    else
    {
        return; /* Timer nao suportado */
    }

    HAL_NVIC_SetPriority(irqn, GPT_NVIC_PREEMPT_PRIORITY, GPT_NVIC_SUB_PRIORITY);
    HAL_NVIC_EnableIRQ(irqn);
}

/* ------------------------------------------------------------------ */
/* Implementacao da API publica                                       */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo GPT.
 *
 * Configura o prescaler de todos os timers GPT via HAL_TIM_Base_Init()
 * e habilita as interrupcoes NVIC.
 *
 * O CubeMX normalmente gera a inicializacao dos timers (MX_TIMx_Init).
 * Esta funcao complementa configurando o prescaler padrao e NVIC.
 *
 * TODO: Se o CubeMX gerenciar a inicializacao completa dos timers,
 *       esta funcao pode ser reduzida apenas a configuracao do NVIC.
 */
void Gpt_Init(void)
{
    uint8_t ch;

    /* Inicializa tabela de callbacks */
    for (ch = 0U; ch < GPT_NUM_CHANNELS; ch++)
    {
        Gpt_Callbacks[ch] = NULL;
    }

    /* Configura o prescaler e modo base para cada canal.
     * TIM_AUTORELOAD_PRELOAD_DISABLE garante que a escrita em ARR
     * tem efeito imediato (sem esperar proximo ciclo). */
    for (ch = 0U; ch < GPT_NUM_CHANNELS; ch++)
    {
        TIM_HandleTypeDef*          htim = Gpt_ChannelCfg[ch].handle;
        TIM_Base_InitTypeDef*       init = &(htim->Init);

        init->Prescaler         = GPT_PRESCALER_VALUE;
        init->CounterMode       = TIM_COUNTERMODE_UP;
        init->Period            = 0xFFFFFFFFU;  /* Sera' sobrescrito em Gpt_StartTimer */
        init->ClockDivision     = TIM_CLOCKDIVISION_DIV1;
        init->AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

        /* Inicializa o timer em modo base (sem captura/compare).
         * Resultado ignorado pois o CubeMX pode ja ter inicializado. */
        (void)HAL_TIM_Base_Init(htim);

        /* Configura NVIC para este timer */
        Gpt_ConfigNVIC(Gpt_ChannelCfg[ch].instance);
    }
}

/**
 * \brief Registra callback para quando o timer expirar.
 *
 * O callback e' chamado diretamente do contexto da ISR.
 * Deve ser curto e nao bloquear (max algumas dezenas de us).
 * Para operacoes longas, usar flag e processar em task.
 *
 * \param ch       Canal do timer (GPT_CH_*)
 * \param callback Funcao chamada na expiracao
 */
void Gpt_SetNotification(Gpt_ChannelType ch, Gpt_NotificationType callback)
{
    if (ch < GPT_NUM_CHANNELS)
    {
        /* Escrita atomica em ponteiro 32-bit no Cortex-M.
         * Sem necessidade de secao critica para esta operacao isolada. */
        Gpt_Callbacks[ch] = callback;
    }
}

/**
 * \brief Inicia timer one-shot com timeout em microssegundos.
 *
 * Sequencia de operacao:
 *   1. Verifica canal valido e handle disponivel
 *   2. Para o timer se ja estiver rodando
 *   3. Carrega ARR com o timeout em ticks (1 tick = 1 us)
 *   4. Reseta o contador (CNT = 0)
 *   5. Limpa a flag de Update pendente (evita disparo imediato)
 *   6. Habilita interrupcao de Update
 *   7. Inicia o timer com HAL_TIM_Base_Start_IT
 *
 * O bit OPM (One Pulse Mode) deve estar configurado no CubeMX para
 * que o timer pare automaticamente ao atingir ARR.
 *
 * \param ch         Canal do timer (GPT_CH_*)
 * \param timeout_us Tempo ate' expiracao em us (0 < timeout_us <= maxPeriod)
 */
void Gpt_StartTimer(Gpt_ChannelType ch, uint32 timeout_us)
{
    TIM_HandleTypeDef* htim;
    uint32_t           ticks;

    if (ch >= GPT_NUM_CHANNELS)
    {
        return;
    }

    htim = Gpt_ChannelCfg[ch].handle;
    if (htim == NULL)
    {
        return;
    }

    /* Conversao us -> ticks. Com prescaler de 200 e clock de 200 MHz,
     * GPT_US_TO_TICKS(us) = us (trivial, sem multiplicacao). */
    ticks = GPT_US_TO_TICKS(timeout_us);

    /* Satura o valor para o maximo suportado pelo timer */
    if (ticks > Gpt_ChannelCfg[ch].maxPeriod)
    {
        ticks = Gpt_ChannelCfg[ch].maxPeriod;
    }

    /* Garante valor minimo de 1 tick (ARR = 0 dispara imediatamente) */
    if (ticks == 0U)
    {
        ticks = 1U;
    }

    /* Para o timer se estiver rodando (sem disparar callback).
     * Necessario antes de modificar ARR para evitar comportamento
     * indefinido em timers 16-bit com ARR abaixo do CNT atual. */
    (void)HAL_TIM_Base_Stop_IT(htim);

    /* Carrega o periodo (Auto-Reload Register).
     * Com AUTORELOAD_PRELOAD_DISABLE, o valor entra em efeito imediatamente. */
    __HAL_TIM_SET_AUTORELOAD(htim, ticks);

    /* Reseta o contador para 0 antes de iniciar */
    __HAL_TIM_SET_COUNTER(htim, 0U);

    /* Limpa a flag de Update Event (UIF) que pode ter ficado pendente.
     * CRITICO: sem este clear, a ISR pode disparar prematuramente. */
    __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);

    /* Inicia o timer com interrupcao de Update habilitada.
     * O timer conta de 0 ate ARR e gera evento (ISR) ao atingir ARR. */
    (void)HAL_TIM_Base_Start_IT(htim);
}

/**
 * \brief Para o timer sem disparar o callback.
 *
 * Desabilita o timer e a interrupcao de Update.
 * Se o callback estava pendente (timer expirou antes desta chamada),
 * a flag UIF pode ja ter sido setada - e' limpa aqui por seguranca.
 *
 * \param ch Canal do timer (GPT_CH_*)
 */
void Gpt_StopTimer(Gpt_ChannelType ch)
{
    TIM_HandleTypeDef* htim;

    if (ch >= GPT_NUM_CHANNELS)
    {
        return;
    }

    htim = Gpt_ChannelCfg[ch].handle;
    if (htim == NULL)
    {
        return;
    }

    (void)HAL_TIM_Base_Stop_IT(htim);

    /* Limpa flag pendente para nao disparar na proxima habilitacao */
    __HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_UPDATE);
}

/**
 * \brief Retorna o tempo decorrido desde o inicio do timer em us.
 *
 * Le diretamente o registrador CNT do timer.
 * Com prescaler = 199 e clock = 200 MHz, o contador incrementa
 * a 1 MHz, portanto CNT == tempo decorrido em microssegundos.
 *
 * Se o timer ja expirou (OPM: parou em ARR), retorna ARR (maximo).
 * Se o timer foi parado antes de expirar, retorna o valor no momento
 * da parada (timer congelado no HAL_TIM_Base_Stop_IT).
 *
 * \param ch Canal do timer (GPT_CH_*)
 * \return Tempo decorrido em us
 */
uint32 Gpt_GetElapsedTime_us(Gpt_ChannelType ch)
{
    TIM_HandleTypeDef* htim;
    uint32_t           cnt;

    if (ch >= GPT_NUM_CHANNELS)
    {
        return 0U;
    }

    htim = Gpt_ChannelCfg[ch].handle;
    if (htim == NULL)
    {
        return 0U;
    }

    /* Leitura direta do contador. No Cortex-M, leitura de 32-bit e' atomica. */
    cnt = __HAL_TIM_GET_COUNTER(htim);

    /* GPT_TICKS_TO_US e' trivial (= cnt) mas mantida para portabilidade */
    return GPT_TICKS_TO_US(cnt);
}

/* ------------------------------------------------------------------ */
/* Dispatcher de callbacks - chamado pelas ISRs de cada TIM          */
/* ------------------------------------------------------------------ */

/**
 * Funcao interna que verifica e dispara o callback de um canal GPT.
 * Chamada por HAL_TIM_PeriodElapsedCallback com o canal correspondente.
 *
 * \param ch Canal GPT que expirou
 */
static void Gpt_DispatchCallback(Gpt_ChannelType ch)
{
    Gpt_NotificationType cb;

    if (ch >= GPT_NUM_CHANNELS)
    {
        return;
    }

    /* Leitura atomica do ponteiro de callback */
    cb = Gpt_Callbacks[ch];

    if (cb != NULL)
    {
        cb(); /* Chama o callback no contexto da ISR */
    }
}

/* ------------------------------------------------------------------ */
/* Callback HAL - dispatcha para o canal correto                     */
/* ------------------------------------------------------------------ */

/**
 * \brief Callback de expiracao de timer chamado pelo HAL.
 *
 * Sobrescreve a funcao weak do HAL. Chamado por cada ISR TIMx_IRQHandler
 * via HAL_TIM_IRQHandler() quando o evento Update (overflow) ocorre.
 *
 * Identifica qual canal GPT expirou comparando a instancia do handle
 * com as instancias registradas em Gpt_ChannelCfg[].
 *
 * IMPORTANTE: Esta funcao roda em contexto de ISR.
 * O callback do usuario (Gpt_Callbacks[ch]) tambem roda em ISR.
 * Qualquer comunicacao com tasks deve usar mecanismos ISR-safe
 * (ex: xTaskNotifyFromISR, xQueueSendFromISR no FreeRTOS).
 *
 * \param htim Handle do timer que gerou o evento
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    uint8_t ch;

    /* Percorre a tabela para identificar qual canal expirou */
    for (ch = 0U; ch < GPT_NUM_CHANNELS; ch++)
    {
        if (Gpt_ChannelCfg[ch].handle == htim)
        {
            Gpt_DispatchCallback((Gpt_ChannelType)ch);
            break; /* Cada handle e' unico - sai apos encontrar */
        }
    }
}

/* ------------------------------------------------------------------ */
/* ISR Handlers - wrappers para o HAL                                */
/* ------------------------------------------------------------------ */

/**
 * Handlers de interrupcao dos timers GPT.
 * Chamam HAL_TIM_IRQHandler que por sua vez chama
 * HAL_TIM_PeriodElapsedCallback quando o evento e' de overflow.
 *
 * TODO: Se o CubeMX gerar estes handlers em stm32h7xx_it.c,
 *       remover as definicoes abaixo para evitar duplicacao de simbolo.
 *       Alternativa: usar a flag de compilacao GPT_OWNS_IRQ_HANDLERS.
 */

#ifndef GPT_SKIP_IRQ_HANDLERS  /* Definir no Makefile se o CubeMX gera os handlers */

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim4);
}

void TIM5_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim5);
}

#endif /* GPT_SKIP_IRQ_HANDLERS */
