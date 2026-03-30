/**
 * \file main_ecu.c
 * \brief Ponto de entrada da ECU na plataforma STM32H745 com FreeRTOS
 *
 * Este arquivo NAO e o main.c gerado pelo CubeMX. E a camada de
 * aplicacao chamada a partir do main.c do CubeMX, apos a conclusao
 * de todos os MX_*_Init().
 *
 * Responsabilidades DESTE arquivo:
 *   - Criar as tarefas FreeRTOS que envolvem os corpos unificados
 *   - Chamar EcuTask_Init() antes de iniciar o escalonador
 *   - Iniciar o escalonador FreeRTOS via vTaskStartScheduler()
 *
 * O que NAO pertence a este arquivo:
 *   - Logica de aplicacao (esta em ecu_tasks.c)
 *   - Chamadas HAL (o CubeMX main.c cuida disso)
 *   - Configuracao de perifericos (feita pelo Dio/Adc/Icu/Gpt MCAL)
 *
 * Integracao com CubeMX:
 *   No main.c gerado pelo CubeMX, adicione ao final da funcao main(),
 *   apos todas as chamadas MX_*_Init():
 *
 *       extern void ECU_Main(void);
 *       ECU_Main();   // nunca retorna
 *
 * Escalonamento das tarefas:
 *   Cada EcuTask_*ms() e envolvida por uma tarefa FreeRTOS com
 *   vTaskDelay(pdMS_TO_TICKS(N)) para periodicidade absoluta simples.
 *   Para temporizacao mais precisa (sem drift acumulado), substituir
 *   por vTaskDelayUntil() conforme necessario.
 *
 * Pilhas e prioridades:
 *   Ajustar conforme analise de stack usage real (configMINIMAL_STACK_SIZE
 *   como referencia). Os valores abaixo sao conservadores.
 *
 * \note O XcpCanIf_Handler() e chamado na tarefa de background sem
 *       delay (taskYIELD apos cada iteracao), garantindo latencia
 *       sub-1 ms exigida pelo protocolo XCP.
 */

/* ------------------------------------------------------------------ */
/* FreeRTOS                                                             */
/* ------------------------------------------------------------------ */
#include "FreeRTOS.h"
#include "task.h"

/* ------------------------------------------------------------------ */
/* Tarefas unificadas da ECU                                            */
/* ------------------------------------------------------------------ */
#include "ecu_tasks.h"
#include "main.h"
#include "Dio.h"
#include "cdd_crankshaft.h"

/* ------------------------------------------------------------------ */
/* Handler CAN/XCP (polling de baixa latencia)                          */
/* ------------------------------------------------------------------ */
#include "xcp_can_if.h"
#include "stm32h7xx_hal.h"

/* ------------------------------------------------------------------ */
/* Animacoes de LED (igual TC297B)                                      */
/* ------------------------------------------------------------------ */

/* LED2 tem logica invertida no hardware */
#define LED1_ON()   HAL_GPIO_WritePin(DO_LED1_GPIO_Port, DO_LED1_Pin, GPIO_PIN_SET)
#define LED1_OFF()  HAL_GPIO_WritePin(DO_LED1_GPIO_Port, DO_LED1_Pin, GPIO_PIN_RESET)
#define LED2_ON()   HAL_GPIO_WritePin(DO_LED2_GPIO_Port, DO_LED2_Pin, GPIO_PIN_RESET)
#define LED2_OFF()  HAL_GPIO_WritePin(DO_LED2_GPIO_Port, DO_LED2_Pin, GPIO_PIN_SET)
#define LED4_ON()   HAL_GPIO_WritePin(DO_LED4_GPIO_Port, DO_LED4_Pin, GPIO_PIN_SET)
#define LED4_OFF()  HAL_GPIO_WritePin(DO_LED4_GPIO_Port, DO_LED4_Pin, GPIO_PIN_RESET)

/**
 * Bounce/Knight Rider: LED1 -> LED2 -> LED4 -> LED2
 * Chamada a cada 100ms quando chave OFF
 */
static void Anim_Bounce_Step(void)
{
    static uint8_t step = 0;

    switch (step)
    {
        case 0: LED1_ON();  LED2_OFF(); LED4_OFF(); break;
        case 1: LED1_OFF(); LED2_ON();  LED4_OFF(); break;
        case 2: LED1_OFF(); LED2_OFF(); LED4_ON();  break;
        case 3: LED1_OFF(); LED2_ON();  LED4_OFF(); break;
        default: step = 0; return;
    }

    step = (step >= 3) ? 0 : (step + 1);
}

/**
 * Pares alternados: LED1+LED4 <-> LED2
 * Chamada a cada 100ms quando chave ON sem motor
 */
static void Anim_AlternatePairs_Step(void)
{
    static uint8_t toggle = 0;

    if (toggle)
    {
        LED1_ON();  LED2_OFF(); LED4_ON();
    }
    else
    {
        LED1_OFF(); LED2_ON();  LED4_OFF();
    }

    toggle ^= 1;
}

/* ================================================================== */
/* Tamanhos de pilha das tarefas FreeRTOS (em palavras de 32 bits)     */
/* ================================================================== */

/** Tarefa 5 ms: apenas toggle de debug - pilha minima */
#define STACK_5MS_WORDS     256u

/**
 * Tarefa 10 ms: Task0_Run (ASCET) + FUEL + XcpBackground.
 * ASCET pode ter profundidade de chamada consideravel.
 */
#define STACK_10MS_WORDS    512u

/** Tarefa 20 ms: MNGT + SPARK */
#define STACK_20MS_WORDS    512u

/** Tarefa 100 ms: LEDs CAN status + hook display */
#define STACK_100MS_WORDS   256u

/**
 * Background: EcuTask_Background (ADC filter) + XcpCanIf_Handler.
 * XcpCanIf_Handler pode ter profundidade variavel dependendo da impl.
 */
#define STACK_BG_WORDS      512u

/* ================================================================== */
/* Prioridades FreeRTOS das tarefas                                    */
/*                                                                      */
/* Escala: 0 = mais baixa, configMAX_PRIORITIES-1 = mais alta          */
/* Regra: tarefas mais rapidas recebem prioridade mais alta para        */
/* garantir que o jitter de ativacao seja minimizado.                   */
/*                                                                      */
/*   5  ms  -> prioridade 4  (mais rapida entre as periodicas)          */
/*   10 ms  -> prioridade 3                                             */
/*   20 ms  -> prioridade 2                                             */
/*   100 ms -> prioridade 1                                             */
/*   BG     -> prioridade 0  (background, cede para qualquer periodica) */
/* ================================================================== */
#define PRIORITY_5MS        4u
#define PRIORITY_10MS       3u
#define PRIORITY_20MS       2u
#define PRIORITY_100MS      1u
#define PRIORITY_BG         0u

/* ================================================================== */
/* Task de inicializacao (roda uma vez, depois se deleta)               */
/* Garante que EcuTask_Init() roda com SysTick funcional               */
/* ================================================================== */
static void Task_Init(void *pvParameters)
{
    (void)pvParameters;

    EcuTask_Init();

    /* Inicializacao completa - deleta esta tarefa */
    vTaskDelete(NULL);
}

/* ================================================================== */
/* Wrappers FreeRTOS das tarefas periodicas                            */
/*                                                                      */
/* Cada wrapper e uma funcao de tarefa FreeRTOS estatica (nao exportada)*/
/* que chama o corpo unificado correspondente e aguarda o proximo       */
/* periodo via vTaskDelay.                                               */
/* ================================================================== */

/**
 * Tarefa 5 ms: debug toggle.
 * vTaskDelay(5) assegura que o corpo seja chamado a cada 5 ms
 * (com possivel jitter de +/-1 tick do FreeRTOS).
 */
static void Task_5ms(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        
        EcuTask_5ms();
        vTaskDelay(pdMS_TO_TICKS(5u));
    }
}

/**
 * Tarefa 10 ms: ASCET Task0, FUEL, XcpBackground.
 * E a tarefa de maior carga computacional nas periodicas.
 */
static void Task_10ms(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        
        EcuTask_10ms();
        vTaskDelay(pdMS_TO_TICKS(10u));
    }
}

/**
 * Tarefa 20 ms: Management + Spark advance.
 */
static void Task_20ms(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        
        EcuTask_20ms();
        vTaskDelay(pdMS_TO_TICKS(20u));
    }
}

/**
 * Tarefa 100 ms: status CAN nos LEDs + hook display.
 */
static void Task_100ms(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        EcuTask_100ms();
        vTaskDelay(pdMS_TO_TICKS(100u));
    }
}

/**
 * Tarefa de background: amostragem ADC do TPS + polling XCP CAN.
 *
 * Nao usa vTaskDelay com periodo fixo - roda continuamente e cede
 * o processador via taskYIELD() para que tarefas de maior prioridade
 * possam preemptir quando suas flags forem setadas.
 *
 * XcpCanIf_Handler() e chamado aqui para garantir latencia < 1 ms
 * na resposta ao mestre XCP (requisito do protocolo).
 */
static void Task_Background(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        
        EcuTask_Background();
        XcpCanIf_Handler();
        taskYIELD();
    }
}

/* ================================================================== */
/* Hook de display - animacoes de LED baseadas no estado do motor       */
/*                                                                      */
/* Sobrescreve a versao fraca em ecu_tasks.c.                           */
/* Chamada por EcuTask_100ms() a cada 100 ms.                           */
/*                                                                      */
/* Estados:                                                              */
/*   RPM == 0, chave OFF  -> Bounce (Knight Rider)                      */
/*   RPM == 0, chave ON   -> Pares alternados (1+3 <-> 2+4)            */
/*   RPM > 0              -> LEDs controlados pelo CDD (inj/spark)      */
/* ================================================================== */
void EcuTask_Hook_DisplayUpdate(void)
{
    if (CDD_Get_EngineSpeed_RAW() == 0)
    {
        if (Dio_ReadChannel(DIO_CH_IGNITION_ON))
            Anim_AlternatePairs_Step();  /* Chave ON, sem motor */
        else
            Anim_Bounce_Step();          /* Chave OFF */
    }
    else
    {
        /* Motor rodando: LEDs controlados pelo CDD (injecao/ignicao) */
    }
}

/* ================================================================== */
/* ECU_Main: criacao das tarefas e inicio do escalonador               */
/*                                                                      */
/* Chamado pelo main.c do CubeMX apos todos os MX_*_Init().            */
/* Esta funcao NAO retorna (vTaskStartScheduler e bloqueante).          */
/* ================================================================== */
/**
 * FreeRTOS tick hook: chamado a cada tick do scheduler (1 ms).
 * Incrementa o contador HAL para que HAL_GetTick() funcione
 * corretamente com timeouts de SPI, I2C, etc.
 */
void vApplicationTickHook(void)
{
    HAL_IncTick();
}

void ECU_Main(void)
{
    /* Cria task de init + tarefas periodicas e a de background.
     * EcuTask_Init() e chamada dentro da Task_Init para garantir que
     * o SysTick (HAL_GetTick) esteja funcional via vApplicationTickHook.
     * Sem o tick, funcoes HAL com timeout (SPI, ADC) travam. */

    /* Task de init: prioridade maxima para rodar primeiro e se deletar */
    xTaskCreate(Task_Init,
                "Init",
                STACK_10MS_WORDS,
                NULL,
                configMAX_PRIORITIES - 1,
                NULL);

    xTaskCreate(Task_5ms,
                "T5ms",
                STACK_5MS_WORDS,
                NULL,
                PRIORITY_5MS,
                NULL);

    xTaskCreate(Task_10ms,
                "T10ms",
                STACK_10MS_WORDS,
                NULL,
                PRIORITY_10MS,
                NULL);

    xTaskCreate(Task_20ms,
                "T20ms",
                STACK_20MS_WORDS,
                NULL,
                PRIORITY_20MS,
                NULL);

    xTaskCreate(Task_100ms,
                "T100ms",
                STACK_100MS_WORDS,
                NULL,
                PRIORITY_100MS,
                NULL);

    xTaskCreate(Task_Background,
                "BG",
                STACK_BG_WORDS,
                NULL,
                PRIORITY_BG,
                NULL);

    /* Inicia o escalonador FreeRTOS - nunca retorna em operacao normal */
    vTaskStartScheduler();

    /*
     * Chegou aqui somente em caso de falha na alocacao do heap do idle task
     * (configTOTAL_HEAP_SIZE muito pequeno). Nunca deve ocorrer em operacao
     * normal. Travar em loop infinito para facilitar deteccao em debug.
     */
    for (;;)
    {
        /* Falha critica: escalonador nao iniciou */
    }
}
