/**
 * \file Dio_Cfg.h
 * \brief Configuracao de pinos DIO para STM32H745
 *
 * Mapeia os canais logicos definidos em Dio.h para os pinos fisicos
 * do STM32H745. As atribuicoes abaixo sao PLACEHOLDER e devem ser
 * ajustadas conforme o esquematico final via STM32CubeMX.
 *
 * Plataforma : STM32H745 (Dual-Core Cortex-M7 + M4)
 * HAL        : stm32h7xx_hal_gpio.h
 *
 * \note TODO: Revisar todas as atribuicoes de pino apos geracao do
 *             projeto no CubeMX e conferir com o esquematico de hardware.
 */
#ifndef DIO_CFG_H
#define DIO_CFG_H

#include "stm32h7xx_hal.h"
#include "Dio.h"

/* ------------------------------------------------------------------ */
/* Tipo de entrada na tabela de mapeamento                            */
/* ------------------------------------------------------------------ */

/**
 * Estrutura que associa um canal logico DIO a um pino fisico STM32.
 * Compativel com o formato usado na plataforma TC297B (array de structs).
 */
typedef struct
{
    GPIO_TypeDef* port;     /**< Porta GPIO (ex: GPIOB, GPIOC, ...) */
    uint16_t      pin;      /**< Mascara do pino (ex: GPIO_PIN_0)   */
} Dio_PinConfigType;

/* ------------------------------------------------------------------ */
/* Tabela de mapeamento logico -> fisico                              */
/* Indexada por DIO_CH_* definidos em Dio.h                          */
/* ------------------------------------------------------------------ */

/* TODO: Todos os pinos abaixo sao PLACEHOLDER.
 *       Ajustar conforme esquematico e projeto CubeMX.
 *       Verificar conflitos com TIM, SPI, FDCAN, UART apos alocacao. */

static const Dio_PinConfigType Dio_PinMap[DIO_NUM_CHANNELS] =
{
    /* ----------------------------------------------------------------
     * Saidas - Injetores (GPIOB PB0..PB3)
     * Acionados por driver de alta corrente externo (ex: MC33810 saida
     * ou driver discreto). Nivel ativo: HIGH (confirmar com hardware).
     * TODO: PB0..PB3 podem conflitar com TIM3_CH3/CH4 ou ADC1 - checar.
     * ---------------------------------------------------------------- */
    /* [DIO_CH_INJECTOR1 = 0] */ { GPIOB, GPIO_PIN_0  },
    /* [DIO_CH_INJECTOR2 = 1] */ { GPIOB, GPIO_PIN_1  },
    /* [DIO_CH_INJECTOR3 = 2] */ { GPIOB, GPIO_PIN_2  },
    /* [DIO_CH_INJECTOR4 = 3] */ { GPIOB, GPIO_PIN_3  },

    /* ----------------------------------------------------------------
     * Saidas - Bobinas de Ignicao (GPIOC PC0..PC1)
     * Controlam o transistor de dwell da bobina (ignicao por centelha).
     * Nivel ativo: HIGH (confirmar polaridade do driver).
     * TODO: Verificar conflito com TIM3_CH3 (PC8 alternativo se necessario).
     * ---------------------------------------------------------------- */
    /* [DIO_CH_COIL1 = 4] */ { GPIOC, GPIO_PIN_0  },
    /* [DIO_CH_COIL2 = 5] */ { GPIOC, GPIO_PIN_1  },

    /* ----------------------------------------------------------------
     * Saidas - Atuadores (GPIOD)
     * PD0: Bomba de combustivel
     * PD1: Rele de partida a frio
     * PD2: Ventoinha velocidade baixa
     * PD3: Ventoinha velocidade alta
     * PD4: Aquecedor sonda lambda 1
     * PD5: Aquecedor sonda lambda 2
     * TODO: Verificar que PD0..PD5 nao conflitam com FMC ou SDMMC.
     * ---------------------------------------------------------------- */
    /* [DIO_CH_FUEL_PUMP      =  6] */ { GPIOD, GPIO_PIN_0  },
    /* [DIO_CH_COLDSTART_RELAY =  7] */ { GPIOD, GPIO_PIN_1  },
    /* [DIO_CH_FAN_LOW         =  8] */ { GPIOD, GPIO_PIN_2  },
    /* [DIO_CH_FAN_HIGH        =  9] */ { GPIOD, GPIO_PIN_3  },
    /* [DIO_CH_LAMBDA_HEATER1  = 10] */ { GPIOD, GPIO_PIN_4  },
    /* [DIO_CH_LAMBDA_HEATER2  = 11] */ { GPIOD, GPIO_PIN_5  },

    /* ----------------------------------------------------------------
     * Saidas - MC33810 (controlador de saidas de ignicao/injecao NXP)
     * PD6: Chip Select SPI do MC33810
     * PD7: Enable de saida do MC33810 (ENOUT)
     * TODO: CS do SPI geralmente pertence ao bloco SPI - avaliar se
     *       o CubeMX deve gerenciar como NSS ou como GPIO manual.
     * ---------------------------------------------------------------- */
    /* [DIO_CH_MC33810_CS    = 12] */ { GPIOD, GPIO_PIN_6  },
    /* [DIO_CH_MC33810_ENOUT = 13] */ { GPIOD, GPIO_PIN_7  },

    /* ----------------------------------------------------------------
     * Saidas - MC33186 H-Bridge (driver de borboleta eletronica NXP)
     * PD8 : DI1 - Entrada de direcao 1
     * PD9 : DI2 - Entrada de direcao 2
     * PD10: COD - Codigo de operacao
     * PD11: IN1 - Enable da ponte H
     * TODO: PD8..PD11 sao alternativas para USART3 no STM32H7 - checar.
     * ---------------------------------------------------------------- */
    /* [DIO_CH_MC33186_DI1 = 14] */ { GPIOD, GPIO_PIN_8  },
    /* [DIO_CH_MC33186_DI2 = 15] */ { GPIOD, GPIO_PIN_9  },
    /* [DIO_CH_MC33186_COD = 16] */ { GPIOD, GPIO_PIN_10 },
    /* [DIO_CH_MC33186_IN1 = 17] */ { GPIOD, GPIO_PIN_11 },

    /* ----------------------------------------------------------------
     * Saidas - GIN (MC33810 Gate Ignition outputs, saidas de centelha)
     * PE0..PE3: GIN0..GIN3
     * TODO: Confirmar que PE0..PE3 estao disponiveis (sem conflito ETH).
     * ---------------------------------------------------------------- */
    /* [DIO_CH_GIN0 = 18] */ { GPIOE, GPIO_PIN_0  },
    /* [DIO_CH_GIN1 = 19] */ { GPIOE, GPIO_PIN_1  },
    /* [DIO_CH_GIN2 = 20] */ { GPIOE, GPIO_PIN_2  },
    /* [DIO_CH_GIN3 = 21] */ { GPIOE, GPIO_PIN_3  },

    /* ----------------------------------------------------------------
     * Entradas Digitais (GPIOE PE4..PE10)
     * PE4 : Chave de ignicao (IGNITION_ON)
     * PE5 : Sensor de fase (came) - fase do motor
     * PE6 : Sensor de pedal de freio 1
     * PE7 : Sensor de pedal de freio 2
     * PE8 : Sensor de embreagem
     * PE9 : Botao/chave A/C
     * PE10: Status fault MC33186 (ativo LOW na maioria dos drivers)
     * TODO: Entradas com EXTI: configurar pull-up/pull-down no CubeMX.
     * ---------------------------------------------------------------- */
    /* [DIO_CH_IGNITION_ON = 22] */ { GPIOE, GPIO_PIN_4  },
    /* [DIO_CH_PHASE_STATE = 23] */ { GPIOE, GPIO_PIN_5  },
    /* [DIO_CH_BRAKE_SW1   = 24] */ { GPIOE, GPIO_PIN_6  },
    /* [DIO_CH_BRAKE_SW2   = 25] */ { GPIOE, GPIO_PIN_7  },
    /* [DIO_CH_CLUTCH_SW   = 26] */ { GPIOE, GPIO_PIN_8  },
    /* [DIO_CH_AC_SW       = 27] */ { GPIOE, GPIO_PIN_9  },
    /* [DIO_CH_MC33186_SF  = 28] */ { GPIOE, GPIO_PIN_10 },

    /* ----------------------------------------------------------------
     * LEDs de Debug (GPIOG PG0..PG6)
     * Nivel ativo: HIGH (anodo conectado ao pino via resistor).
     * TODO: LEDs podem ser realocados para qualquer GPIO disponivel.
     * ---------------------------------------------------------------- */
    /* LEDs de ignição -> DO_LED4 (PD12) */
    /* [DIO_CH_LED1    = 29] */ { GPIOD, GPIO_PIN_12 },  /* Bobina 1/4 -> DO_LED4 */
    /* [DIO_CH_LED2    = 30] */ { GPIOD, GPIO_PIN_12 },  /* Bobina 2/3 -> DO_LED4 */
    /* [DIO_CH_LED3    = 31] */ { GPIOD, GPIO_PIN_12 },  /* (reserva)  -> DO_LED4 */
    /* [DIO_CH_LED4    = 32] */ { GPIOD, GPIO_PIN_12 },  /* (reserva)  -> DO_LED4 */
    /* LEDs de injeção -> DO_LED1 (PA0) */
    /* [DIO_CH_LED_HIGH= 33] */ { GPIOA, GPIO_PIN_0  },  /* Injetor 3  -> DO_LED1 */
    /* [DIO_CH_LED_MID = 34] */ { GPIOA, GPIO_PIN_0  },  /* Injetor 2  -> DO_LED1 */
    /* [DIO_CH_LED_LOW = 35] */ { GPIOA, GPIO_PIN_0  },  /* Injetor 1  -> DO_LED1 */
    /* [DIO_CH_LED_GND = 36] */ { GPIOA, GPIO_PIN_0  },  /* Injetor 4  -> DO_LED1 */
};

/* ------------------------------------------------------------------ */
/* Configuracao de direcao dos pinos                                  */
/* ------------------------------------------------------------------ */

/**
 * Mascara de bits indicando quais canais sao ENTRADA.
 * Bit N = 1  => DIO_CH_N e' entrada (GPIO_MODE_INPUT)
 * Bit N = 0  => DIO_CH_N e' saida  (GPIO_MODE_OUTPUT_PP)
 *
 * Canais de entrada: 22..28 (IGNITION_ON ate MC33186_SF)
 *
 * TODO: Ajustar apos definicao final do hardware.
 *       Entradas com interrupcao externa (EXTI) sao configuradas no ICU.
 */
#define DIO_INPUT_CHANNEL_MASK      ((1UL << DIO_CH_IGNITION_ON) | \
                                     (1UL << DIO_CH_PHASE_STATE)  | \
                                     (1UL << DIO_CH_BRAKE_SW1)    | \
                                     (1UL << DIO_CH_BRAKE_SW2)    | \
                                     (1UL << DIO_CH_CLUTCH_SW)    | \
                                     (1UL << DIO_CH_AC_SW)        | \
                                     (1UL << DIO_CH_MC33186_SF))

/** Estado inicial das saidas (0 = LOW, 1 = HIGH).
 *  Todos os atuadores iniciam desligados (LOW). */
#define DIO_INITIAL_OUTPUT_STATE    0u

#endif /* DIO_CFG_H */
