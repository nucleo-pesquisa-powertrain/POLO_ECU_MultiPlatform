/**
 * \file Dio.c
 * \brief Implementacao MCAL DIO para STM32H745
 *
 * Implementa a API Dio.h usando HAL_GPIO do STM32H7.
 * O mapeamento de canais logicos para pinos fisicos e' feito
 * pela tabela Dio_PinMap[] definida em Dio_Cfg.h.
 *
 * Notas de implementacao:
 *   - A inicializacao dos pinos (clock, modo, velocidade, pull)
 *     e' normalmente feita pelo codigo gerado pelo CubeMX (MX_GPIOx_Init).
 *     A funcao Dio_Init() abaixo serve como fallback e para configurar
 *     pinos nao incluidos no projeto CubeMX.
 *   - Todas as funcoes sao thread-safe em relacao a leitura/escrita de
 *     pinos distintos (HAL_GPIO e' atomico por bit banding no STM32).
 *     Acesso concorrente ao MESMO pino de ISR e task deve ser protegido
 *     externamente com seccao critica se necessario.
 *
 * Plataforma : STM32H745
 * HAL        : stm32h7xx_hal_gpio.h
 */

#include "Dio.h"
#include "Dio_Cfg.h"
#include "stm32h7xx_hal.h"

/* ------------------------------------------------------------------ */
/* Validacao de canal                                                 */
/* ------------------------------------------------------------------ */

/**
 * Verifica se o canal e' valido antes de acessar a tabela.
 * Em builds de producao, a verificacao pode ser removida com -DNDEBUG
 * para economizar ciclos. Por ora, falha silenciosa (sem assert).
 */
#define DIO_IS_VALID_CHANNEL(ch)    ((ch) < DIO_NUM_CHANNELS)

/* ------------------------------------------------------------------ */
/* Funcoes privadas                                                   */
/* ------------------------------------------------------------------ */

/**
 * Habilita o clock da porta GPIO necessaria.
 * O CubeMX gera chamadas equivalentes em MX_GPIO_Init(); esta funcao
 * e' usada como fallback caso Dio_Init seja chamada isoladamente.
 *
 * \param port Ponteiro para a instancia GPIO
 */
static void Dio_EnablePortClock(GPIO_TypeDef* port)
{
    if      (port == GPIOA) { __HAL_RCC_GPIOA_CLK_ENABLE(); }
    else if (port == GPIOB) { __HAL_RCC_GPIOB_CLK_ENABLE(); }
    else if (port == GPIOC) { __HAL_RCC_GPIOC_CLK_ENABLE(); }
    else if (port == GPIOD) { __HAL_RCC_GPIOD_CLK_ENABLE(); }
    else if (port == GPIOE) { __HAL_RCC_GPIOE_CLK_ENABLE(); }
    else if (port == GPIOF) { __HAL_RCC_GPIOF_CLK_ENABLE(); }
    else if (port == GPIOG) { __HAL_RCC_GPIOG_CLK_ENABLE(); }
    else if (port == GPIOH) { __HAL_RCC_GPIOH_CLK_ENABLE(); }
    else if (port == GPIOI) { __HAL_RCC_GPIOI_CLK_ENABLE(); }
    else if (port == GPIOJ) { __HAL_RCC_GPIOJ_CLK_ENABLE(); }
    else if (port == GPIOK) { __HAL_RCC_GPIOK_CLK_ENABLE(); }
    /* GPIOB/C/D etc: adicionar outros bancos se o STM32H745 suportar */
}

/* ------------------------------------------------------------------ */
/* Implementacao da API publica                                       */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa todos os pinos digitais.
 *
 * Percorre a tabela Dio_PinMap[], habilita o clock das portas e
 * configura cada pino no modo correto (saida push-pull ou entrada).
 *
 * Se o CubeMX estiver sendo usado, esta funcao deve ser chamada APOS
 * o MX_GPIO_Init() gerado pelo CubeMX, ou omitida se todos os pinos
 * ja estiverem configurados corretamente pelo CubeMX.
 *
 * TODO: Avaliar se e' necessario chamar Dio_Init() no fluxo de boot
 *       quando o CubeMX ja gerou a inicializacao dos pinos.
 */
void Dio_Init(void)
{
    uint8_t             ch;
    GPIO_InitTypeDef    gpioInit = {0};

    for (ch = 0U; ch < DIO_NUM_CHANNELS; ch++)
    {
        /* Habilita clock da porta antes de qualquer operacao no pino */
        Dio_EnablePortClock(Dio_PinMap[ch].port);

        gpioInit.Pin   = Dio_PinMap[ch].pin;
        gpioInit.Speed = GPIO_SPEED_FREQ_LOW;  /* Baixa velocidade reduz EMI */

        /* Verifica se o canal e' entrada ou saida pela mascara de bits */
        if ((DIO_INPUT_CHANNEL_MASK & (1UL << ch)) != 0U)
        {
            /* Entrada digital com pull-up interno.
             * TODO: Ajustar Pull conforme hardware (pode ser PULLDOWN ou NOPULL).
             *       MC33186_SF e' ativo-LOW: usar PULLUP. Chaves podem variar. */
            gpioInit.Mode = GPIO_MODE_INPUT;
            gpioInit.Pull = GPIO_PULLUP;
        }
        else
        {
            /* Saida push-pull, inicia em nivel baixo (atuadores desligados).
             * TODO: Se algum atuador precisar de open-drain, alterar aqui. */
            gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
            gpioInit.Pull = GPIO_NOPULL;

            /* Garante estado inicial seguro antes de habilitar o pino */
            HAL_GPIO_WritePin(Dio_PinMap[ch].port,
                              Dio_PinMap[ch].pin,
                              GPIO_PIN_RESET);
        }

        HAL_GPIO_Init(Dio_PinMap[ch].port, &gpioInit);
    }
}

/**
 * \brief Escreve nivel logico num canal DIO.
 *
 * Mapeia o canal logico para o pino fisico e chama HAL_GPIO_WritePin.
 * A operacao BSRR do Cortex-M e' atomica (bit set/reset register),
 * portanto nao ha risco de corrupcao em escritas concorrentes em pinos
 * distintos da mesma porta.
 *
 * \param ch    Canal logico (DIO_CH_*)
 * \param level DIO_HIGH ou DIO_LOW
 */
void Dio_WriteChannel(Dio_ChannelType ch, uint8 level)
{
    if (!DIO_IS_VALID_CHANNEL(ch))
    {
        return; /* Canal invalido: nenhuma acao (fail-safe) */
    }

    HAL_GPIO_WritePin(Dio_PinMap[ch].port,
                      Dio_PinMap[ch].pin,
                      (level != DIO_LOW) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * \brief Le nivel logico de um canal DIO.
 *
 * \param ch Canal logico (DIO_CH_*)
 * \return DIO_HIGH (1) ou DIO_LOW (0)
 */
uint8 Dio_ReadChannel(Dio_ChannelType ch)
{
    GPIO_PinState state;

    if (!DIO_IS_VALID_CHANNEL(ch))
    {
        return DIO_LOW; /* Canal invalido: retorna LOW (fail-safe) */
    }

    state = HAL_GPIO_ReadPin(Dio_PinMap[ch].port, Dio_PinMap[ch].pin);

    return (state == GPIO_PIN_SET) ? DIO_HIGH : DIO_LOW;
}

/**
 * \brief Inverte o nivel logico de um canal DIO.
 *
 * Usa HAL_GPIO_TogglePin que faz leitura-modificacao-escrita no ODR.
 * Nao e' atomica em relacao a outros threads que escrevem no mesmo pino.
 * Para uso em ISR junto com task, proteger externamente.
 *
 * \param ch Canal logico (DIO_CH_*)
 */
void Dio_ToggleChannel(Dio_ChannelType ch)
{
    if (!DIO_IS_VALID_CHANNEL(ch))
    {
        return;
    }

    HAL_GPIO_TogglePin(Dio_PinMap[ch].port, Dio_PinMap[ch].pin);
}
