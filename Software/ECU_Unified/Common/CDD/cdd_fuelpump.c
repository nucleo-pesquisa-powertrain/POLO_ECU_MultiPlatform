/**
 * \file cdd_fuelpump.c
 * \brief CDD - Controle da Bomba de Combustivel
 *
 * Controla a saida digital da bomba de combustivel com base no estado
 * operacional da ECU. Logica de operacao:
 *
 *   - CRANKING / IDLE / ACCELERATION / OVERRUN:
 *       Bomba ativa continuamente; reseta a fase inicial para garantir
 *       re-pressurizacao caso o motor morra e religue.
 *
 *   - PRE_START / ENGINE_STOPPED (fase inicial nao concluida):
 *       Bomba ativa por 2 segundos (200 ciclos de 10 ms) para pre-
 *       pressurizacao do sistema de combustivel antes da partida.
 *
 *   - Qualquer outro estado:
 *       Bomba inativa.
 *
 * Dependencias iLLD removidas:
 *   - hal_discrete_outputs.h  (substituido por Dio.h)
 *   - IfxPort.h               (substituido por Dio.h / constantes DIO_HIGH/DIO_LOW)
 *
 * Historico:
 *   2026-03-28  Refatorado: substituidas chamadas HAL_DISCRETE_Set_FuelPump()
 *               e IfxPort_State_high/low por Dio_WriteChannel(DIO_CH_FUEL_PUMP,
 *               DIO_HIGH/DIO_LOW). Logica de controle preservada identica ao original.
 */

#include <stdbool.h>
#include "cdd_fuelpump.h"
#include "ECU_State_interface.h"
#include "Dio.h"

/** Contador de ciclos de 10 ms para a fase inicial de pre-pressurizacao */
static unsigned long fuel_pump_timer = 0u;

/**
 * Flag: fase inicial de pre-pressurizacao ja concluida.
 * Resetada quando o motor entra em operacao, para garantir nova
 * pre-pressurizacao caso o motor morra e tente religar.
 */
static int fuel_pump_initial_phase_done = false;

/**
 * \brief Tarefa periodica de 10 ms: controla a bomba de combustivel.
 *
 * Deve ser chamada a cada 10 ms pelo escalonador de tarefas.
 */
void CDD_FuelPump_Event(void)
{
    ECU_State_t current_state = Get_ECU_State();

    if ((current_state == ECU_STATE_CRANKING)     ||
        (current_state == ECU_STATE_ACCELERATION) ||
        (current_state == ECU_STATE_IDLE)         ||
        (current_state == ECU_STATE_OVERRUN))
    {
        /* Motor em operacao: bomba ativa continuamente */
        Dio_WriteChannel(DIO_CH_FUEL_PUMP, DIO_HIGH);

        /* Reseta a fase inicial para garantir re-pressurizacao apos parada */
        fuel_pump_initial_phase_done = false;
        fuel_pump_timer = 0u;
    }
    else if (((current_state == ECU_STATE_PRE_START)      ||
              (current_state == ECU_STATE_ENGINE_STOPPED)) &&
             (fuel_pump_initial_phase_done == false))
    {
        /* Fase de pre-pressurizacao: ativa bomba por 2 segundos (200 x 10 ms) */
        Dio_WriteChannel(DIO_CH_FUEL_PUMP, DIO_HIGH);
        fuel_pump_timer++;

        if (fuel_pump_timer >= 200u)
        {
            fuel_pump_initial_phase_done = true;
            fuel_pump_timer = 0u;
            Dio_WriteChannel(DIO_CH_FUEL_PUMP, DIO_LOW);
        }
    }
    else
    {
        /* Todos os outros estados: bomba inativa */
        Dio_WriteChannel(DIO_CH_FUEL_PUMP, DIO_LOW);
    }
}
