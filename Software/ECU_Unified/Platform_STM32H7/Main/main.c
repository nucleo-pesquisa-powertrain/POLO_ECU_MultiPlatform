/**
 * \file main.c
 * \brief Entry point do STM32H7 - inicializa HAL/CubeMX e chama ECU_Main
 */

#include "main.h"
#include "ecu_tasks.h"
#include "FreeRTOS.h"
#include "task.h"

/* LEDs da Nucleo-H755ZI-Q (nao definidos no CubeMX) */
#ifndef LD1_Pin
#define LD1_Pin   GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#endif
#ifndef LD2_Pin
#define LD2_Pin   GPIO_PIN_1
#define LD2_GPIO_Port GPIOE
#endif

/* Declaracoes CubeMX (geradas pelo .ioc) */
extern void MX_GPIO_Init(void);
extern void MX_ADC1_Init(void);
extern void MX_ADC2_Init(void);
extern void MX_ADC3_Init(void);
extern void MX_FDCAN1_Init(void);
extern void MX_SPI1_Init(void);
extern void MX_TIM2_Init(void);
extern void MX_TIM3_Init(void);
extern void MX_TIM15_Init(void);
extern void MX_TIM7_Init(void);
extern void MX_USART3_UART_Init(void);

/* ECU_Main de main_ecu.c */
extern void ECU_Main(void);

/* Stubs para timers nao configurados no CubeMX original */
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;

/* Stubs para PWM timers nao configurados */
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim12;

/* SystemClock_Config - copiada do main.c original do CubeMX */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 60;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                                |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) { Error_Handler(); }
}

/* Error_Handler - chamada pelo CubeMX em caso de erro de init */
void Error_Handler(void)
{
    __disable_irq();
    while (1) { }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* Init perifericos CubeMX */
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_ADC2_Init();
    MX_ADC3_Init();
    MX_FDCAN1_Init();
    MX_SPI1_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_TIM15_Init();
    MX_TIM7_Init();
    MX_USART3_UART_Init();

    /* ---- Animacao de startup Knight Rider (4 LEDs) ---- */
    {
        int i, j;
        volatile uint32_t d;
        #define DELAY_MS(ms) for (d = 0; d < ((ms) * 3000UL); d++) { __NOP(); }

        GPIO_TypeDef* ledPorts[] = { DO_LED1_GPIO_Port, DO_LED2_GPIO_Port, LD3_GPIO_Port, DO_LED4_GPIO_Port };
        uint16_t      ledPins[]  = { DO_LED1_Pin,       DO_LED2_Pin,       LD3_Pin,       DO_LED4_Pin };

        /* 2 varridas ida e volta: 1->2->3->4->3->2 */
        for (i = 0; i < 2; i++)
        {
            /* Ida: 0 -> 3 */
            for (j = 0; j < 4; j++)
            {
                HAL_GPIO_WritePin(ledPorts[j], ledPins[j], GPIO_PIN_SET);
                DELAY_MS(80);
                HAL_GPIO_WritePin(ledPorts[j], ledPins[j], GPIO_PIN_RESET);
            }
            /* Volta: 2 -> 1 */
            for (j = 2; j >= 1; j--)
            {
                HAL_GPIO_WritePin(ledPorts[j], ledPins[j], GPIO_PIN_SET);
                DELAY_MS(80);
                HAL_GPIO_WritePin(ledPorts[j], ledPins[j], GPIO_PIN_RESET);
            }
        }

        /* Flash final: todos acendem juntos */
        for (j = 0; j < 4; j++)
            HAL_GPIO_WritePin(ledPorts[j], ledPins[j], GPIO_PIN_SET);
        DELAY_MS(300);

        /* Apaga tudo */
        for (j = 0; j < 4; j++)
            HAL_GPIO_WritePin(ledPorts[j], ledPins[j], GPIO_PIN_RESET);
        DELAY_MS(150);

        #undef DELAY_MS
    }

    /* Inicia o software da ECU (EcuTask_Init + FreeRTOS scheduler) */
    ECU_Main();

    while (1) { }
}
