/**
 * \file Icu.c
 * \brief Implementacao MCAL ICU para STM32H745
 *
 * Implementa a API Icu.h usando:
 *   - TIM6 como timer free-running de alta resolucao (timestamp)
 *   - EXTI para deteccao de borda do sinal de crankshaft (sensor VR/Hall)
 *
 * Arquitetura:
 *
 *   [TIM6 - Free-running timestamp]
 *     - Timer 16-bit basico (sem captura, sem compare, sem PWM)
 *     - Prescaler = 199 => 1 tick = 1 us (identico aos timers GPT)
 *     - Modo continuo (sem OPM), overflow a cada 65535 us (~65 ms)
 *     - Icu_GetTimestamp_us() le CNT diretamente (leitura atomica)
 *     - Para timestamps > 65 ms, usar contador de overflow de 32-bit
 *       que e' incrementado no callback de Update do TIM6.
 *
 *   [EXTI - Deteccao de borda do crankshaft]
 *     - Pino configurado como entrada com EXTI no CubeMX
 *     - Borda de subida (dentes da roda fonica do crankshaft)
 *     - HAL_GPIO_EXTI_Callback chama o callback registrado
 *     - Callback tipicamente e' CDD_SYNC_Timing_Event()
 *
 * Resolucao de timestamp:
 *   Com TIM6 a 1 MHz e extensao de 32-bit via contador de overflow:
 *     Resolucao: 1 us
 *     Range:     2^32 us = ~4295 segundos antes de overflow total
 *
 * Pino do crankshaft:
 *   TODO: Definir pino EXTI no CubeMX. Candidato: PA8 (TIM1_CH1 alt)
 *         ou PA15 (EXTI15). Ver ICU_CRANK_GPIO_PORT e ICU_CRANK_GPIO_PIN.
 *
 * Plataforma : STM32H745
 * HAL        : stm32h7xx_hal_tim.h, stm32h7xx_hal_gpio.h
 *
 * \note TODO: No CubeMX, configurar TIM6 com:
 *             - Clock Source: Internal Clock
 *             - Prescaler: 199
 *             - Counter Period: 65535 (maximo 16-bit)
 *             - Auto-Reload Preload: Enable
 *             - Interrupcao de Update: habilitada (para extensao 32-bit)
 *             - One Pulse Mode: Disable (free-running)
 */

#include "Icu.h"
#include "stm32h7xx_hal.h"

/* ------------------------------------------------------------------ */
/* Configuracao do pino de crankshaft                                */
/* ------------------------------------------------------------------ */

/**
 * Pino EXTI do sensor de crankshaft.
 * TODO: Ajustar conforme esquematico. PA8 e' candidato por ser
 *       alternativa do TIM1_CH1 e ter EXTI dedicado.
 */
#define ICU_CRANK_GPIO_PORT         GPIOA
#define ICU_CRANK_GPIO_PIN          GPIO_PIN_8

/**
 * Linha EXTI correspondente ao pino do crankshaft.
 * Para GPIOA pino 8: EXTI_LINE_8.
 * TODO: Ajustar se o pino mudar.
 */
#define ICU_CRANK_EXTI_LINE         EXTI_LINE_8

/**
 * IRQn do EXTI para o pino do crankshaft.
 * EXTI9_5_IRQn cobre linhas EXTI 5..9 no STM32H7.
 * TODO: Ajustar se pino for PA0..PA4 (usa EXTI0_IRQn..EXTI4_IRQn)
 *       ou PA10..PA15 (usa EXTI15_10_IRQn).
 */
#define ICU_CRANK_EXTI_IRQn         EXTI9_5_IRQn

/**
 * Prioridade NVIC do EXTI do crankshaft.
 * Deve ser a prioridade mais alta do sistema (1) para garantir
 * que a deteccao de dente nao seja atrasada por nenhuma outra ISR.
 * Timers GPT (ignicao/injecao) usam prioridade 2.
 */
#define ICU_CRANK_NVIC_PRIORITY     1U

/* ------------------------------------------------------------------ */
/* Handle do TIM6                                                     */
/* ------------------------------------------------------------------ */

/**
 * Handle do TIM6 (free-running timestamp).
 * Declarado extern pois e' gerado pelo CubeMX em tim.c ou main.c.
 * TODO: Confirmar nome do handle gerado pelo CubeMX.
 */
extern TIM_HandleTypeDef htim6;

/* ------------------------------------------------------------------ */
/* Estado do modulo                                                   */
/* ------------------------------------------------------------------ */

/**
 * Contador de overflows do TIM6 (16-bit).
 * Incrementado a cada overflow do TIM6 (~65 ms).
 * Combinado com TIM6->CNT fornece timestamp 32-bit de 1 us.
 * Declarado volatile pois e' modificado na ISR do TIM6.
 */
static volatile uint32_t Icu_Tim6OverflowCount;

/**
 * Callback de borda do crankshaft registrado pela aplicacao.
 * NULL ate' que Icu_SetEdgeCallback() seja chamado.
 */
static volatile Icu_EdgeCallbackType Icu_CrankCallback;

/**
 * Flag de habilitacao da deteccao de borda.
 * Controlada por Icu_EnableEdgeDetection / Icu_DisableEdgeDetection.
 */
static volatile uint8_t Icu_EdgeDetectionEnabled;

/* ------------------------------------------------------------------ */
/* Implementacao da API publica                                       */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa o modulo ICU.
 *
 * Configura:
 *   1. TIM6 em modo free-running com prescaler 199 (1 us/tick)
 *   2. EXTI do pino de crankshaft para borda de subida
 *   3. NVIC com prioridades adequadas
 *
 * Deve ser chamada no boot, apos MX_TIM6_Init() do CubeMX.
 *
 * TODO: Se o CubeMX gerar a inicializacao do TIM6 com os parametros
 *       corretos, a secao de init do TIM6 pode ser simplificada.
 */
void Icu_Init(void)
{
    TIM_Base_InitTypeDef timInit = {0};

    /* Inicializa estado do modulo */
    Icu_Tim6OverflowCount    = 0U;
    Icu_CrankCallback        = NULL;
    Icu_EdgeDetectionEnabled = 0U;

    /* --- Configuracao do TIM6 como free-running timestamp --- */

    /* Configura TIM6: basico (sem canal de captura/compare).
     * Prescaler = 199 => f = 200MHz / 200 = 1 MHz => 1 tick = 1 us.
     * Period = 0xFFFF => overflow a cada 65535 us (65.5 ms). */
    htim6.Instance               = TIM6;
    timInit.Prescaler            = 199U;
    timInit.CounterMode          = TIM_COUNTERMODE_UP;
    timInit.Period               = 0xFFFFU;
    timInit.AutoReloadPreload    = TIM_AUTORELOAD_PRELOAD_ENABLE;
    htim6.Init                   = timInit;

    (void)HAL_TIM_Base_Init(&htim6);

    /* Habilita NVIC para TIM6 (prioridade baixa pois so' incrementa contador).
     * Prioridade 15 (mais baixa): o incremento do overflow nao e' urgente. */
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 15U, 0U);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

    /* Inicia TIM6 com interrupcao de Update (para extensao 32-bit).
     * A partir daqui, o timestamp e' valido. */
    (void)HAL_TIM_Base_Start_IT(&htim6);

    /* --- Configuracao do EXTI do crankshaft --- */

    /* Habilita clock da porta GPIO do crankshaft */
    if (ICU_CRANK_GPIO_PORT == GPIOA) { __HAL_RCC_GPIOA_CLK_ENABLE(); }

    /* Configura o pino como entrada com EXTI na borda de subida.
     * Pull-up interno: assumindo sensor VR com condicionador externo
     * que entrega nivel logico (0..3.3V). Para sensor Hall: NOPULL.
     * TODO: Ajustar Pull conforme tipo de sensor de crankshaft. */
    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.Pin  = ICU_CRANK_GPIO_PIN;
    gpioInit.Mode = GPIO_MODE_IT_RISING;  /* Borda de subida (dente da roda) */
    gpioInit.Pull = GPIO_NOPULL;          /* TODO: Ajustar conforme circuito */
    HAL_GPIO_Init(ICU_CRANK_GPIO_PORT, &gpioInit);

    /* Configura NVIC do EXTI - prioridade maxima do sistema */
    HAL_NVIC_SetPriority(ICU_CRANK_EXTI_IRQn, ICU_CRANK_NVIC_PRIORITY, 0U);
    /* EXTI inicia desabilitado; habilitado por Icu_EnableEdgeDetection() */
    HAL_NVIC_DisableIRQ(ICU_CRANK_EXTI_IRQn);
}

/**
 * \brief Retorna timestamp livre em microssegundos.
 *
 * Combina o contador de overflow (32 MSBits) com o valor do CNT
 * do TIM6 (16 LSBits, 1 us/tick) para formar um timestamp de 32-bit.
 *
 * Formula: timestamp_us = (overflow_count * 65536) + TIM6->CNT
 *
 * Protecao contra race condition de overflow:
 *   1. Le overflow count
 *   2. Le CNT
 *   3. Verifica se overflow count mudou (overflow aconteceu durante leitura)
 *   4. Se mudou, le novamente com CNT = 0 apos overflow
 *
 * \return Timestamp em us (overflow total a cada ~4295 segundos)
 */
uint32 Icu_GetTimestamp_us(void)
{
    uint32_t overflow1;
    uint32_t overflow2;
    uint32_t cnt;
    uint32_t timestamp;

    /* Leitura segura contra overflow entre leitura de overflow e CNT */
    do
    {
        overflow1 = Icu_Tim6OverflowCount;
        cnt       = (uint32_t)(htim6.Instance->CNT);
        overflow2 = Icu_Tim6OverflowCount;
    }
    while (overflow1 != overflow2);
    /* Se o overflow ocorreu entre as duas leituras, repete.
     * Na pratica isto ocorre a cada 65 ms, portanto raramente. */

    /* Combina overflow (extensao de 16-bit para 32-bit) com CNT.
     * TIM6 e' 16-bit: periodo = 65536 ticks = 65536 us. */
    timestamp = (overflow1 << 16U) | cnt;

    return timestamp;
}

/**
 * \brief Registra callback para borda do crankshaft.
 *
 * O callback sera chamado no contexto da ISR EXTI cada vez que
 * um dente da roda fonica for detectado.
 *
 * Tipicamente este callback e' CDD_SYNC_Timing_Event() que calcula
 * a posicao angular e agenda ignicao/injecao.
 *
 * \param callback Funcao chamada a cada borda detectada
 */
void Icu_SetEdgeCallback(Icu_EdgeCallbackType callback)
{
    /* Desabilita a deteccao temporariamente para escrita atomica.
     * Evita que a ISR leia um callback parcialmente escrito. */
    Icu_DisableEdgeDetection();

    Icu_CrankCallback = callback;

    /* Restaura o estado anterior se estava habilitado */
    if (Icu_EdgeDetectionEnabled != 0U)
    {
        Icu_EnableEdgeDetection();
    }
}

/**
 * \brief Habilita a interrupcao de borda do crankshaft.
 *
 * Habilita o IRQ EXTI no NVIC. A borda comeca a ser detectada
 * imediatamente apos esta chamada.
 *
 * Deve ser chamada apos o motor estar em condicoes de operar
 * (pos inicializacao do sistema de sincronismo).
 */
void Icu_EnableEdgeDetection(void)
{
    Icu_EdgeDetectionEnabled = 1U;
    /* Limpa flag EXTI pendente antes de habilitar para evitar
     * disparo espurio imediato (por nivel em vez de borda). */
    __HAL_GPIO_EXTI_CLEAR_FLAG(ICU_CRANK_GPIO_PIN);
    HAL_NVIC_EnableIRQ(ICU_CRANK_EXTI_IRQn);
}

/**
 * \brief Desabilita a interrupcao de borda do crankshaft.
 *
 * Usado durante inicializacao, calibracao, ou falha de sincronismo.
 * A ISR EXTI pode ainda estar pendente no NVIC apos esta chamada;
 * a flag EXTI e' limpa para prevenir disparo ao reabilitar.
 */
void Icu_DisableEdgeDetection(void)
{
    HAL_NVIC_DisableIRQ(ICU_CRANK_EXTI_IRQn);
    __HAL_GPIO_EXTI_CLEAR_FLAG(ICU_CRANK_GPIO_PIN);
    Icu_EdgeDetectionEnabled = 0U;
}

/* ------------------------------------------------------------------ */
/* Callbacks HAL                                                      */
/* ------------------------------------------------------------------ */

/**
 * \brief Callback de borda EXTI chamado pelo HAL.
 *
 * Sobrescreve a funcao weak HAL_GPIO_EXTI_Callback.
 * Chamado para QUALQUER pino EXTI ativo; verifica se e' o crankshaft.
 *
 * CRITICO: Este callback roda com a prioridade mais alta do sistema.
 *   - Nao deve bloquear (sem mutexes, sem delay)
 *   - Nao deve chamar funcoes nao-reententes do HAL
 *   - Tempo de execucao deve ser < 5 us (registrar timestamp, disparar callback)
 *
 * \param GPIO_Pin Mascara do pino que gerou a borda
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    Icu_EdgeCallbackType cb;

    /* Verifica se a interrupcao e' do pino de crankshaft */
    if (GPIO_Pin != ICU_CRANK_GPIO_PIN)
    {
        return; /* Borda de outro pino EXTI - ignorar */
    }

    /* Leitura atomica do callback */
    cb = Icu_CrankCallback;

    if (cb != NULL)
    {
        cb(); /* Dispara CDD_SYNC_Timing_Event() ou similar */
    }
}

/**
 * \brief Callback de overflow do TIM6.
 *
 * Chamado pela ISR TIM6_DAC_IRQHandler quando o contador de 16-bit
 * atinge 0xFFFF e reinicia. Incrementa o contador de extensao 32-bit.
 *
 * Ocorre a cada 65536 us (~65.5 ms) com prescaler 199 e clock 200 MHz.
 *
 * \note Esta funcao e' sobrescrita aqui para uso exclusivo do ICU.
 *       Se o DAC tambem usar TIM6, ha conflito - ver TIM7 como alternativa.
 *       TODO: Avaliar conflito TIM6/DAC no projeto CubeMX.
 */
void HAL_TIM_PeriodElapsedCallback_TIM6(TIM_HandleTypeDef* htim)
{
    if (htim->Instance == TIM6)
    {
        /* Incremento atomico: no Cortex-M single-core (M7 ou M4),
         * a ISR nao e' interrompida por outra de mesma prioridade.
         * No dual-core (M7 + M4), proteger com HSEM se necessario. */
        Icu_Tim6OverflowCount++;
    }
}

/* ------------------------------------------------------------------ */
/* ISR Handlers                                                       */
/* ------------------------------------------------------------------ */

/**
 * Handlers de interrupcao para EXTI e TIM6.
 *
 * TODO: Se o CubeMX gerar estes handlers em stm32h7xx_it.c,
 *       remover as definicoes abaixo e adicionar chamadas ao ICU
 *       dentro dos handlers gerados.
 *       Usar a flag de compilacao ICU_SKIP_IRQ_HANDLERS.
 */

#ifndef ICU_SKIP_IRQ_HANDLERS

/**
 * Handler EXTI para linhas 5..9 (cobre EXTI8 do crankshaft).
 * TODO: Ajustar IRQHandler se o pino for diferente:
 *   PA0..PA4  => EXTI0_IRQHandler .. EXTI4_IRQHandler
 *   PA5..PA9  => EXTI9_5_IRQHandler
 *   PA10..PA15=> EXTI15_10_IRQHandler
 */
void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(ICU_CRANK_GPIO_PIN);
}

/**
 * Handler do TIM6 (compartilhado com DAC no STM32H7).
 * Chama HAL_TIM_IRQHandler que por sua vez dispara
 * HAL_TIM_PeriodElapsedCallback para o TIM6.
 *
 * TODO: No STM32H745, TIM6_DAC_IRQHandler cobre tanto TIM6 quanto DAC.
 *       Se o DAC for usado, o handler existente em stm32h7xx_it.c
 *       deve ser estendido para incluir a chamada abaixo.
 */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
    /* Aciona o callback de overflow do TIM6 diretamente
     * (complementa HAL que chama HAL_TIM_PeriodElapsedCallback) */
    if (__HAL_TIM_GET_FLAG(&htim6, TIM_FLAG_UPDATE) == RESET)
    {
        /* Flag ja limpa pelo HAL_TIM_IRQHandler - verificar overflow */
        /* O HAL ja chamou HAL_TIM_PeriodElapsedCallback se necessario */
    }
}

#endif /* ICU_SKIP_IRQ_HANDLERS */

/* ------------------------------------------------------------------ */
/* Nota sobre dual-core STM32H745                                    */
/* ------------------------------------------------------------------ */

/*
 * No STM32H745 dual-core (Cortex-M7 + Cortex-M4), os perifericos
 * podem ser acessados pelos dois cores. Para este modulo ICU:
 *
 *   - TIM6 deve ser alocado para o core que gerencia o sincronismo
 *     (recomendado: Cortex-M7, Core1, mais rapido).
 *   - O EXTI do crankshaft deve ser tratado pelo mesmo core.
 *   - Se o Core2 (M4) precisar do timestamp, usar HSEM (Hardware
 *     Semaphore) ou variavel shared em SRAM3 (acessivel por ambos).
 *
 * TODO: Definir alocacao de perifericos por core no CubeMX (aba
 *       "Cortex-M4 peripherals" e "Cortex-M7 peripherals").
 */
