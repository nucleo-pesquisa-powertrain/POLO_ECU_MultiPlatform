/**
 * \file Dio.c
 * \brief Implementacao MCAL DIO para Infineon AURIX TC297B
 *
 * Encapsula as chamadas iLLD IfxPort_setPinState / IfxPort_getPinState /
 * IfxPort_setPinMode para fornecer a API comum definida em Dio.h.
 *
 * A tabela Dio_PinMap[] mapeia cada canal logico (DIO_CH_*) ao modulo de
 * porta fisico (Ifx_P*) e ao indice do pino. O indice do array e' o
 * proprio canal logico, portanto nao ha busca em tempo de execucao.
 *
 * Plataforma: Infineon AURIX TC297B
 * Compilador: TASKING VX-toolset for TriCore
 */

/* ------------------------------------------------------------------ */
/* Includes                                                            */
/* ------------------------------------------------------------------ */

#include "Dio.h"
#include "Dio_Cfg.h"
#include "Mcal_Compiler.h"

/* iLLD */
#include "IfxPort.h"
#include "IfxPort_reg.h"

/* ------------------------------------------------------------------ */
/* Tabela de mapeamento: canal logico -> pino fisico TC297B            */
/* ------------------------------------------------------------------ */
/*
 * A ordem das entradas DEVE seguir a numeracao dos canais DIO_CH_*
 * definidos em Dio.h. O canal logico e' usado diretamente como indice.
 *
 * Formato: { porta, pino, isOutput }
 *   isOutput = 1 -> configurado como saida push-pull em Dio_Init()
 *   isOutput = 0 -> configurado como entrada em Dio_Init()
 */
const Dio_PinMapEntry Dio_PinMap[DIO_NUM_CHANNELS] =
{
    /* [ 0] DIO_CH_INJECTOR1       */ { &MODULE_P02, 1u,  1u },
    /* [ 1] DIO_CH_INJECTOR2       */ { &MODULE_P02, 3u,  1u },
    /* [ 2] DIO_CH_INJECTOR3       */ { &MODULE_P10, 6u,  1u },
    /* [ 3] DIO_CH_INJECTOR4       */ { &MODULE_P10, 4u,  1u },
    /* [ 4] DIO_CH_COIL1           */ { &MODULE_P33, 10u, 1u },
    /* [ 5] DIO_CH_COIL2           */ { &MODULE_P00, 4u,  1u },
    /* [ 6] DIO_CH_FUEL_PUMP       */ { &MODULE_P22, 2u,  1u },
    /* [ 7] DIO_CH_COLDSTART_RELAY */ { &MODULE_P00, 12u, 1u },
    /* [ 8] DIO_CH_FAN_LOW         */ { &MODULE_P00, 8u,  1u },
    /* [ 9] DIO_CH_FAN_HIGH        */ { &MODULE_P00, 10u, 1u },
    /* [10] DIO_CH_LAMBDA_HEATER1  */ { &MODULE_P00, 1u,  1u },
    /* [11] DIO_CH_LAMBDA_HEATER2  */ { &MODULE_P02, 8u,  1u },
    /* [12] DIO_CH_MC33810_CS      */ { &MODULE_P22, 3u,  1u },
    /* [13] DIO_CH_MC33810_ENOUT   */ { &MODULE_P10, 7u,  1u },
    /* [14] DIO_CH_MC33186_DI1     */ { &MODULE_P02, 6u,  1u },
    /* [15] DIO_CH_MC33186_DI2     */ { &MODULE_P00, 7u,  1u },
    /* [16] DIO_CH_MC33186_COD     */ { &MODULE_P02, 4u,  1u },
    /* [17] DIO_CH_MC33186_IN1     */ { &MODULE_P00, 11u, 1u },
    /* [18] DIO_CH_GIN0            */ { &MODULE_P00, 0u,  1u },
    /* [19] DIO_CH_GIN1            */ { &MODULE_P00, 2u,  1u },
    /* [20] DIO_CH_GIN2            */ { &MODULE_P00, 4u,  1u },
    /* [21] DIO_CH_GIN3            */ { &MODULE_P00, 6u,  1u },
    /* [22] DIO_CH_IGNITION_ON     */ { &MODULE_P23, 4u,  0u },
    /* [23] DIO_CH_PHASE_STATE     */ { &MODULE_P14, 8u,  0u },
    /* [24] DIO_CH_BRAKE_SW1       */ { &MODULE_P14, 4u,  0u },
    /* [25] DIO_CH_BRAKE_SW2       */ { &MODULE_P15, 4u,  0u },
    /* [26] DIO_CH_CLUTCH_SW       */ { &MODULE_P15, 6u,  0u },
    /* [27] DIO_CH_AC_SW           */ { &MODULE_P14, 7u,  0u },
    /* [28] DIO_CH_MC33186_SF      */ { &MODULE_P00, 7u,  0u },
    /* [29] DIO_CH_LED1            */ { &MODULE_P33, 4u,  1u },
    /* [30] DIO_CH_LED2            */ { &MODULE_P33, 2u,  1u },
    /* [31] DIO_CH_LED3            */ { &MODULE_P33, 12u, 1u },
    /* [32] DIO_CH_LED4            */ { &MODULE_P33, 6u,  1u },
    /* [33] DIO_CH_LED_HIGH        */ { &MODULE_P13, 0u,  1u },
    /* [34] DIO_CH_LED_MID         */ { &MODULE_P13, 1u,  1u },
    /* [35] DIO_CH_LED_LOW         */ { &MODULE_P13, 2u,  1u },
    /* [36] DIO_CH_LED_GND         */ { &MODULE_P13, 3u,  1u }
};

/* ------------------------------------------------------------------ */
/* Implementacao da API                                                */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa todos os pinos digitais
 *
 * Saidas sao configuradas como push-pull (IfxPort_Mode_outputPushPullGeneral)
 * com nivel inicial LOW para evitar acionamento involuntario de atuadores.
 * Entradas sao configuradas como entrada sem pull (IfxPort_Mode_inputNoPullDevice).
 */
void Dio_Init(void)
{
    uint8 ch;

    for (ch = 0u; ch < DIO_NUM_CHANNELS; ch++)
    {
        const Dio_PinMapEntry* entry = &Dio_PinMap[ch];

        if (entry->isOutput != 0u)
        {
            /* Configura pino como saida push-pull de proposito geral */
            IfxPort_setPinModeOutput(entry->port,
                                     entry->pinIndex,
                                     IfxPort_OutputMode_pushPull,
                                     IfxPort_OutputIdx_general);

            /* Estado inicial: desligado (LOW) - seguro para todos os atuadores */
            IfxPort_setPinLow(entry->port, entry->pinIndex);
        }
        else
        {
            /* Configura pino como entrada sem resistor de pull */
            IfxPort_setPinModeInput(entry->port,
                                    entry->pinIndex,
                                    IfxPort_InputMode_noPullDevice);
        }
    }
}

/**
 * \brief Escreve nivel logico em um canal de saida
 *
 * \param ch    Canal logico (DIO_CH_*)
 * \param level DIO_HIGH (1) ou DIO_LOW (0)
 *
 * \note Nenhuma verificacao de direcao e' feita em runtime para minimizar
 *       overhead. A configuracao correta deve ser garantida em Dio_Init().
 */
void Dio_WriteChannel(Dio_ChannelType ch, uint8 level)
{
    if (ch >= DIO_NUM_CHANNELS)
    {
        /* Canal invalido - protecao contra acesso fora do array */
        return;
    }

    IfxPort_setPinState(Dio_PinMap[ch].port,
                        Dio_PinMap[ch].pinIndex,
                        (level != 0u) ? IfxPort_State_high : IfxPort_State_low);
}

/**
 * \brief Le o nivel logico de um canal
 *
 * \param ch Canal logico (DIO_CH_*)
 * \return DIO_HIGH (1) ou DIO_LOW (0)
 */
uint8 Dio_ReadChannel(Dio_ChannelType ch)
{
    if (ch >= DIO_NUM_CHANNELS)
    {
        return DIO_LOW;
    }

    return (uint8)IfxPort_getPinState(Dio_PinMap[ch].port,
                                      Dio_PinMap[ch].pinIndex);
}

/**
 * \brief Inverte o nivel logico de um canal de saida
 *
 * \param ch Canal logico (DIO_CH_*)
 */
void Dio_ToggleChannel(Dio_ChannelType ch)
{
    if (ch >= DIO_NUM_CHANNELS)
    {
        return;
    }

    IfxPort_togglePin(Dio_PinMap[ch].port, Dio_PinMap[ch].pinIndex);
}
