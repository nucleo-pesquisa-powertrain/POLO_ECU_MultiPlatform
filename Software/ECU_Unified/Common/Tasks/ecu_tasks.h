/**
 * \file ecu_tasks.h
 * \brief Declaracao dos corpos de tarefa unificados da ECU
 *
 * Todas as funcoes declaradas aqui sao implementadas em ecu_tasks.c
 * usando exclusivamente a MCAL AUTOSAR (Dio, Adc, Icu, Gpt).
 * Nenhuma dependencia de iLLD (TC297B) ou STM32 HAL e' permitida.
 *
 * O escalonamento e' responsabilidade da camada de plataforma:
 *   - TC297B : bare-metal com flags setadas pela ISR do STM (Cpu0_Main.c)
 *   - STM32H7: FreeRTOS com tarefas dedicadas (main_ecu.c)
 *
 * Fluxo de inicializacao esperado:
 *   1. Plataforma inicializa perifericos especificos (clock, watchdog, etc.)
 *   2. EcuTask_Init() e' chamado uma unica vez
 *   3. Loop de tarefas periodicas comeca (EcuTask_5ms ... EcuTask_100ms)
 *   4. EcuTask_Background() e' chamado continuamente entre as tarefas
 */

#ifndef ECU_TASKS_H
#define ECU_TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Variaveis exportadas (usadas por rte_components.c, tbi_calibration) */
/* ------------------------------------------------------------------ */

/**
 * Valor filtrado do TPS (media de 10 ms).
 * Calculado em EcuTask_Background() e lido pelo RTE.
 * Unidade: milivolts (0..5000 mV).
 */
extern unsigned long tps_filtered_value;

/* ------------------------------------------------------------------ */
/* API de Tarefas                                                       */
/* ------------------------------------------------------------------ */

/**
 * \brief Inicializa todos os modulos da ECU.
 *
 * Deve ser chamado uma unica vez antes do inicio do loop ciclico.
 * Sequencia:
 *   1. MCAL: Dio_Init, Adc_Init, Icu_Init, Gpt_Init
 *   2. CDD : CrankshaftPosition, INJ, TBI (SPARK nao tem Init separado,
 *            usa apenas os pinos ja configurados pelo Dio_Init)
 *   3. XCP : XcpCanIf_Init, XcpInit
 *   4. Registra callbacks GPT (ignicao, injecao, dwell)
 *   5. Registra callback ICU (crankshaft) e habilita deteccao de borda
 */
void EcuTask_Init(void);

/**
 * \brief Tarefa ciclica de 5 ms.
 *
 * Uso atual: toggle de variavel de debug para XCP/INCA.
 * LD4 desabilitado temporariamente para debug CAN.
 */
void EcuTask_5ms(void);

/**
 * \brief Tarefa ciclica de 10 ms.
 *
 * Executa:
 *   - Task0_Run()        : logica ASCET (XCP, RTE, ECU_State, TBI, THROTTLE)
 *   - FUEL_MainTask10ms(): calculo de tempo de injecao
 *   - XcpBackground()   : background do protocolo XCP
 */
void EcuTask_10ms(void);

/**
 * \brief Tarefa ciclica de 20 ms.
 *
 * Executa:
 *   - Toggle de variavel de debug
 *   - MNGT_MainTask20ms(): gerenciamento do motor
 *   - SPARK_MainTask20ms(): calculo do avanco de ignicao
 */
void EcuTask_20ms(void);

/**
 * \brief Tarefa ciclica de 100 ms.
 *
 * Executa:
 *   - Toggle de variavel de debug
 *   - Exibe status do CAN nos LEDs de debug (DIO_CH_LED1..4)
 *
 * \note ECU_Pages_Update() (display TFT) e' especifico do TC297B.
 *       Chamado via funcao fraca (weak) definida na plataforma.
 */
void EcuTask_100ms(void);

/**
 * \brief Loop de background (sem periodicidade fixa).
 *
 * Executa continuamente entre as tarefas periodicas:
 *   - Amostragem do TPS a cada 200 us
 *   - Calculo da media filtrada do TPS a cada 10 ms
 *
 * O XcpCanIf_Handler() NAO e' chamado aqui; e' responsabilidade
 * do loop de plataforma chama-lo para garantir latencia sub-1ms.
 */
void EcuTask_Background(void);

/* ------------------------------------------------------------------ */
/* Hook de plataforma (implementacao fraca / opcional)                  */
/* ------------------------------------------------------------------ */

/**
 * \brief Hook para atualizacao do display TFT (especifico TC297B).
 *
 * Declarada como funcao fraca na implementacao padrao (sem operacao).
 * O arquivo Cpu0_Main.c do TC297B fornece a implementacao real que
 * chama ECU_Pages_Update().
 *
 * Em plataformas sem display, o linker usa a versao fraca automaticamente.
 */
void EcuTask_Hook_DisplayUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* ECU_TASKS_H */
