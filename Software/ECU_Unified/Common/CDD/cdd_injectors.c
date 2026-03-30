/**
 * \file cdd_injectors.c
 * \brief CDD - Controle de Injetores de Combustivel
 *
 * Implementa a logica de injecao sequencial e em grupo para motor
 * 4 cilindros. Utiliza exclusivamente a MCAL AUTOSAR:
 *   - Gpt.h  : timer one-shot de duracao da injecao (GPT_CH_INJ_DURATION)
 *   - Dio.h  : controle digital dos injetores, CS do MC33810 e LEDs
 *   - Spi.h  : comunicacao com o driver de ignicao MC33810
 *
 * Dependencias iLLD removidas:
 *   - SPI_CPU.h                (substituido por Spi.h)
 *   - hal_discrete_outputs.h   (substituido por Dio.h)
 *   - GPT12_Timer_Interrupt.h  (substituido por Gpt.h)
 *   - IfxPort.h                (substituido por Dio.h)
 *   - Bsp.h                    (substituido por Delay_us local)
 *
 * Mapeamento de canais DIO:
 *   DIO_CH_INJECTOR1..4  -> saidas de acionamento dos injetores
 *   DIO_CH_MC33810_CS    -> chip-select do driver MC33810
 *   DIO_CH_LED_LOW       -> LED diagnostico injetor 1
 *   DIO_CH_LED_MID       -> LED diagnostico injetor 2
 *   DIO_CH_LED_HIGH      -> LED diagnostico injetor 3
 *   DIO_CH_LED_GND       -> LED diagnostico injetor 4
 *
 * Mecanismo de fila de injecao:
 *   S_InjTimeQueue_us[]  - tempos de injecao pendentes (em us)
 *   S_InjOrderQueue[]    - injetores correspondentes a cada slot
 *
 * O slot [0] e' sempre o evento em execucao. Quando ele termina,
 * a fila e' avancada e o timer reiniciado com S_InjTimeQueue_us[0].
 *
 * \note Todos os valores de tempo estao em MICROSEGUNDOS.
 *       A conversao para ticks do hardware e' responsabilidade
 *       da implementacao de Gpt.h em cada plataforma.
 */

#include "cdd_injectors.h"
#include "Gpt.h"
#include "Dio.h"
#include "Mcal_Spi.h"
#include "Platform_Types.h"

/* ------------------------------------------------------------------ */
/* Constantes locais                                                   */
/* ------------------------------------------------------------------ */

#define OFF 0u
#define ON  1u

/* ------------------------------------------------------------------ */
/* Mensagens de configuracao do MC33810 (SPI)                         */
/* Formato: 2 bytes por mensagem, MSB primeiro                        */
/* ------------------------------------------------------------------ */

/** Clock Calibration: 0xE000 = 0b1110000000000000 */
static const uint8 msg_ClockCalibration[2] = {0xE0u, 0x00u};

/** Operation Mode: 0x1F00 = 0b0001111100000000 */
static const uint8 msg_OperationMode[2]    = {0x1Fu, 0x00u};

/** LSD Fault Command: 0x2A00 = 0b0010101000000000 */
static const uint8 msg_LSDFaultCommand[2]  = {0x2Au, 0x00u};

/* ------------------------------------------------------------------ */
/* Variaveis de estado do modulo (escopo de arquivo)                   */
/* ------------------------------------------------------------------ */

/** Status individual de cada injetor (ON/OFF) */
static uint8 S_Injector1Status = OFF;
static uint8 S_Injector2Status = OFF;
static uint8 S_Injector3Status = OFF;
static uint8 S_Injector4Status = OFF;

/**
 * Fila de ordem de injecao.
 * Slot [0] = evento em andamento; [1..3] = proximos eventos.
 * Valor 0 indica slot vazio.
 */
static uint8 S_InjOrderQueue[4] = {0u, 0u, 0u, 0u};

/**
 * Fila de tempos de injecao em MICROSEGUNDOS.
 *
 * Slot [0]: tempo do evento em andamento (passado para Gpt_StartTimer).
 * Slot [1]: tempo residual relativo ao fim de slot[0].
 * Slot [2]: tempo residual relativo ao fim de slot[1].
 * Slot [3]: tempo residual relativo ao fim de slot[2].
 *
 * Renomeado de S_InjTimeQueue para S_InjTimeQueue_us para explicitar
 * que os valores sao em microsegundos (sem conversao de ticks).
 */
static uint32 S_InjTimeQueue_us[4] = {0u, 0u, 0u, 0u};

/* ------------------------------------------------------------------ */
/* Funcao auxiliar de delay                                            */
/* ------------------------------------------------------------------ */

/**
 * \brief Delay bloqueante em microsegundos
 *
 * \param us Tempo de espera em microsegundos
 *
 * \note TODO: implementar com Gpt ou ICU quando disponivel.
 *       Por enquanto e' um placeholder para nao bloquear a inicializacao
 *       do MC33810. Na plataforma TC297B o tempo de resposta do SPI
 *       e' suficientemente lento para que estes delays sejam tolerados
 *       como busy-wait simples.
 */
static void Delay_us(uint32 us)
{
    /* TODO: substituir por implementacao baseada em Gpt ou ICU */
    volatile uint32 count = us * 30u; /* Estimativa: ~30 ciclos por us a 300 MHz */
    while(count > 0u)
    {
        count--;
    }
}

/**
 * \brief Delay bloqueante em milissegundos
 *
 * \param ms Tempo de espera em milissegundos
 */
static void Delay_ms(uint32 ms)
{
    Delay_us(ms * 1000u);
}

/* ------------------------------------------------------------------ */
/* Prototipos privados                                                 */
/* ------------------------------------------------------------------ */

static void CDD_INJ_HandleInjectionQueue(void);

/* ------------------------------------------------------------------ */
/* Implementacao publica                                               */
/* ------------------------------------------------------------------ */

void CDD_INJ_Init(void)
{
    /* Inicializa o modulo SPI (master mode) via MCAL */
    Spi_Init();

    /* Aguarda qualquer comunicacao SPI anterior terminar */
    while(Spi_IsBusy())
    {
        /* Aguarda SPI ficar livre */
    }

    /* ----------------------------------------------------------------
     * Sequencia de configuracao do MC33810
     *
     * 1) Clock Calibration Command
     * ---------------------------------------------------------------- */
    Dio_WriteChannel(DIO_CH_MC33810_CS, DIO_LOW);
    Spi_Transmit(msg_ClockCalibration, 2u);
    Dio_WriteChannel(DIO_CH_MC33810_CS, DIO_HIGH);
    Delay_us(20u);

    /* Pulso adicional de CS sem dados (conforme sequencia original) */
    Dio_WriteChannel(DIO_CH_MC33810_CS, DIO_LOW);
    Delay_us(32u);
    Dio_WriteChannel(DIO_CH_MC33810_CS, DIO_HIGH);

    Delay_us(20u);

    /* ----------------------------------------------------------------
     * 2) Pulso de CS vazio antes de Operation Mode
     * ---------------------------------------------------------------- */
    Dio_WriteChannel(DIO_CH_MC33810_CS, DIO_LOW);
    Dio_WriteChannel(DIO_CH_MC33810_CS, DIO_HIGH);
    Delay_us(20u);

    /* ----------------------------------------------------------------
     * 3) Operation Mode Command
     * ---------------------------------------------------------------- */
    Dio_WriteChannel(DIO_CH_MC33810_CS, DIO_LOW);
    Spi_Transmit(msg_OperationMode, 2u);
    Dio_WriteChannel(DIO_CH_MC33810_CS, DIO_HIGH);
    Delay_us(20u);

    /* ----------------------------------------------------------------
     * 4) LSD Fault Command
     * ---------------------------------------------------------------- */
    Dio_WriteChannel(DIO_CH_MC33810_CS, DIO_LOW);
    Spi_Transmit(msg_LSDFaultCommand, 2u);
    Dio_WriteChannel(DIO_CH_MC33810_CS, DIO_HIGH);

    Delay_ms(20u);

    /* Garante todos os injetores desligados apos inicializacao */
    Dio_WriteChannel(DIO_CH_INJECTOR1, DIO_LOW);
    Dio_WriteChannel(DIO_CH_INJECTOR2, DIO_LOW);
    Dio_WriteChannel(DIO_CH_INJECTOR3, DIO_LOW);
    Dio_WriteChannel(DIO_CH_INJECTOR4, DIO_LOW);
}

void CDD_INJ_PerformSeqFuelInj(uint8 inj_num, uint16 inj_time)
{
    /* Se todos os injetores estiverem ativos, rejeita o comando */
    if((S_Injector1Status & S_Injector2Status &
        S_Injector3Status & S_Injector4Status) != 0u)
    {
        return;
    }

    /* Aciona o injetor solicitado e sinaliza o LED de diagnostico */
    switch(inj_num)
    {
        case 1u:
            Dio_WriteChannel(DIO_CH_INJECTOR1, DIO_HIGH);
            S_Injector1Status = ON;
            Dio_WriteChannel(DIO_CH_LED_LOW, DIO_LOW);
            break;

        case 2u:
            Dio_WriteChannel(DIO_CH_INJECTOR2, DIO_HIGH);
            S_Injector2Status = ON;
            Dio_WriteChannel(DIO_CH_LED_MID, DIO_LOW);
            break;

        case 3u:
            Dio_WriteChannel(DIO_CH_INJECTOR3, DIO_HIGH);
            S_Injector3Status = ON;
            Dio_WriteChannel(DIO_CH_LED_HIGH, DIO_LOW);
            break;

        case 4u:
            Dio_WriteChannel(DIO_CH_INJECTOR4, DIO_HIGH);
            S_Injector4Status = ON;
            Dio_WriteChannel(DIO_CH_LED_GND, DIO_LOW);
            break;

        default:
            /* Numero de injetor invalido - ignora */
            break;
    }

    /* Insere o evento na primeira posicao livre da fila */
    if(S_InjTimeQueue_us[0] == 0u)
    {
        /*
         * Fila vazia: inicia o timer imediatamente.
         * inj_time ja esta em microsegundos; nenhuma conversao necessaria.
         */
        S_InjTimeQueue_us[0] = (uint32)inj_time;
        S_InjOrderQueue[0]   = inj_num;
        Gpt_StartTimer(GPT_CH_INJ_DURATION, S_InjTimeQueue_us[0]);
    }
    else if(S_InjTimeQueue_us[1] == 0u)
    {
        /*
         * Slot 1 livre: calcula o tempo residual relativo ao fim do
         * slot 0. O timer ja esta rodando; subtrai o que ja decorreu
         * para obter o delta que falta ao slot 0 expirar.
         *
         * S_InjTimeQueue_us[1] = inj_time - tempo_decorrido_slot0
         */
        S_InjTimeQueue_us[1] = (uint32)inj_time - Gpt_GetElapsedTime_us(GPT_CH_INJ_DURATION);
        S_InjOrderQueue[1]   = inj_num;
    }
    else if(S_InjTimeQueue_us[2] == 0u)
    {
        /*
         * Slot 2 livre: tempo residual relativo ao fim do slot 1.
         * Desconta tempo ja decorrido e o delta acumulado do slot 1.
         */
        S_InjTimeQueue_us[2] = (uint32)inj_time
                               - Gpt_GetElapsedTime_us(GPT_CH_INJ_DURATION)
                               - S_InjTimeQueue_us[1];
        S_InjOrderQueue[2]   = inj_num;
    }
    else if(S_InjTimeQueue_us[3] == 0u)
    {
        /*
         * Slot 3 livre: tempo residual relativo ao fim do slot 2.
         * Desconta tempo decorrido e os deltas acumulados de slots 1 e 2.
         */
        S_InjTimeQueue_us[3] = (uint32)inj_time
                               - Gpt_GetElapsedTime_us(GPT_CH_INJ_DURATION)
                               - S_InjTimeQueue_us[1]
                               - S_InjTimeQueue_us[2];
        S_InjOrderQueue[3]   = inj_num;
    }
    else
    {
        /* Fila cheia (4 injetores ativos) - evento descartado */
    }
}

void CDD_INJ_StopFuelInjEvent(void)
{
    /* Desliga o injetor do slot [0] (evento que acaba de expirar) */
    switch(S_InjOrderQueue[0])
    {
        case 1u:
            Dio_WriteChannel(DIO_CH_INJECTOR1, DIO_LOW);
            S_Injector1Status = OFF;
            Dio_WriteChannel(DIO_CH_LED_LOW, DIO_HIGH);
            break;

        case 2u:
            Dio_WriteChannel(DIO_CH_INJECTOR2, DIO_LOW);
            S_Injector2Status = OFF;
            Dio_WriteChannel(DIO_CH_LED_MID, DIO_HIGH);
            break;

        case 3u:
            Dio_WriteChannel(DIO_CH_INJECTOR3, DIO_LOW);
            S_Injector3Status = OFF;
            Dio_WriteChannel(DIO_CH_LED_HIGH, DIO_HIGH);
            break;

        case 4u:
            Dio_WriteChannel(DIO_CH_INJECTOR4, DIO_LOW);
            S_Injector4Status = OFF;
            Dio_WriteChannel(DIO_CH_LED_GND, DIO_HIGH);
            break;

        default:
            /* Estado invalido: desliga todos como seguranca */
            Dio_WriteChannel(DIO_CH_INJECTOR1, DIO_LOW);
            Dio_WriteChannel(DIO_CH_INJECTOR2, DIO_LOW);
            Dio_WriteChannel(DIO_CH_INJECTOR3, DIO_LOW);
            Dio_WriteChannel(DIO_CH_INJECTOR4, DIO_LOW);
            S_Injector1Status = S_Injector2Status = OFF;
            S_Injector3Status = S_Injector4Status = OFF;
            break;
    }

    /* Avanca a fila e (re)inicia o timer se houver proximo evento */
    CDD_INJ_HandleInjectionQueue();
}

void CDD_INJ_PerformFullGroupInjection(uint16 inj_time)
{
    /* Aciona todos os 4 injetores simultaneamente */
    Dio_WriteChannel(DIO_CH_INJECTOR1, DIO_HIGH);
    Dio_WriteChannel(DIO_CH_INJECTOR2, DIO_HIGH);
    Dio_WriteChannel(DIO_CH_INJECTOR3, DIO_HIGH);
    Dio_WriteChannel(DIO_CH_INJECTOR4, DIO_HIGH);

    S_Injector1Status = S_Injector2Status = ON;
    S_Injector3Status = S_Injector4Status = ON;

    /* Limpa fila - modo grupo nao usa fila de eventos individuais */
    S_InjTimeQueue_us[0] = S_InjTimeQueue_us[1] = 0u;
    S_InjTimeQueue_us[2] = S_InjTimeQueue_us[3] = 0u;
    S_InjOrderQueue[0]   = S_InjOrderQueue[1]   = 0u;
    S_InjOrderQueue[2]   = S_InjOrderQueue[3]   = 0u;

    /* Inicia o timer com o tempo de injecao em microsegundos */
    Gpt_StartTimer(GPT_CH_INJ_DURATION, (uint32)inj_time);
}

/* ------------------------------------------------------------------ */
/* Implementacao privada                                               */
/* ------------------------------------------------------------------ */

/**
 * \brief Avanca a fila de injecao e reinicia o timer se necessario
 *
 * Chamada internamente por CDD_INJ_StopFuelInjEvent() apos desligar
 * o injetor do slot [0]. Faz o shift de todos os slots uma posicao
 * para a frente e, se o novo slot [0] nao for zero, reinicia o timer
 * com o tempo residual correspondente.
 */
static void CDD_INJ_HandleInjectionQueue(void)
{
    /* Shift da fila: [1]->[0], [2]->[1], [3]->[2], [3]=0 */
    S_InjTimeQueue_us[0] = S_InjTimeQueue_us[1];
    S_InjTimeQueue_us[1] = S_InjTimeQueue_us[2];
    S_InjTimeQueue_us[2] = S_InjTimeQueue_us[3];
    S_InjTimeQueue_us[3] = 0u;

    S_InjOrderQueue[0] = S_InjOrderQueue[1];
    S_InjOrderQueue[1] = S_InjOrderQueue[2];
    S_InjOrderQueue[2] = S_InjOrderQueue[3];
    S_InjOrderQueue[3] = 0u;

    /* Para o timer antes de avaliar proximo evento */
    Gpt_StopTimer(GPT_CH_INJ_DURATION);

    if(S_InjTimeQueue_us[0] == 0u)
    {
        /* Fila vazia: nenhum evento pendente, timer permanece parado */
    }
    else
    {
        /* Proximo evento disponivel: reinicia timer com tempo residual */
        Gpt_StartTimer(GPT_CH_INJ_DURATION, S_InjTimeQueue_us[0]);
    }
}
