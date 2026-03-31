# EcuAbs - Camada de Abstracao de Sensores e Atuadores

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Criar a camada EcuAbs no Common que encapsula a conversao de mV para grandeza fisica (kPa, degC, %, mV) para todos os sensores da ECU, removendo essa logica do RTE.

**Architecture:** EcuAbs fica entre MCAL e RTE. Le MCAL (Adc_ReadChannel_mV), aplica curvas de calibracao, saturacao e diagnostico de faixa, e expoe funcoes Get_*() com unidades fisicas. O RTE passa a chamar EcuAbs em vez de MCAL+conversao manual. Nenhuma dependencia de plataforma — usa apenas Adc.h.

**Tech Stack:** C99, AUTOSAR naming, MCAL API (Adc.h, Dio.h)

---

## Arquitetura Final

```
ASW (Fuel, Spark, Mngmt)
        |
       RTE  ← chama EcuAbs_Get*(), nao MCAL
        |
     EcuAbs ← conversao mV->fisica, saturacao, diagnostico
        |
      MCAL   ← Adc_ReadChannel_mV(), Dio_ReadChannel()
```

## Sensores a Encapsular

| Funcao EcuAbs | Canal MCAL | Conversao | Unidade |
|---|---|---|---|
| EcuAbs_GetAirTemp_degC() | ADC_CH_AIR_TEMP | T = 2.827*V² - 43.44*V + 135.1 | degC |
| EcuAbs_GetCoolantTemp_degC() | ADC_CH_COOLANT_TEMP | T = -32.24*V + 133.43 | degC |
| EcuAbs_GetMAP_kPa() | ADC_CH_MAP | P = 25.16*V - 3.87 | kPa |
| EcuAbs_GetVbatt_mV() | ADC_CH_VBATT | Vbat = mV * 14.0/5.0 | mV |
| EcuAbs_GetPedalPos_pct() | ADC_CH_PEDAL | linearizacao 39..191 -> 0..100% | % |
| EcuAbs_GetTPS_raw() | ADC_CH_TBI_POS_RED | valor filtrado (background) | mV |
| EcuAbs_GetEngineSpeed_rpm() | CDD_Get_EngineSpeed_RAW() | passthrough | RPM |
| EcuAbs_GetEthanolPercent() | fixo 275 (TODO sensor) | passthrough | 0-1000 |
| EcuAbs_GetPhaseState() | DIO_CH_PHASE_STATE | Dio_ReadChannel | 0/1 |
| EcuAbs_GetIgnitionOn() | DIO_CH_IGNITION_ON | Dio_ReadChannel | 0/1 |

---

### Task 1: Criar EcuAbs_Sensors.h (API publica)

**Files:**
- Create: `Common/EcuAbs/EcuAbs_Sensors.h`

**Step 1: Criar o header com todos os prototipos**

```c
#ifndef ECUABS_SENSORS_H
#define ECUABS_SENSORS_H

#include "Platform_Types.h"

/* Temperaturas */
sint16  EcuAbs_GetAirTemp_degC(void);
sint16  EcuAbs_GetCoolantTemp_degC(void);

/* Pressao */
uint16  EcuAbs_GetMAP_kPa(void);

/* Tensao */
uint16  EcuAbs_GetVbatt_mV(void);

/* Pedal e borboleta */
uint16  EcuAbs_GetPedalPos_pct(void);
uint16  EcuAbs_GetTPS_raw(void);

/* Motor */
uint16  EcuAbs_GetEngineSpeed_rpm(void);
uint16  EcuAbs_GetEthanolPercent(void);

/* Entradas discretas */
uint8   EcuAbs_GetPhaseState(void);
uint8   EcuAbs_GetIgnitionOn(void);

/* Inicializacao (reservada para futuro) */
void    EcuAbs_Init(void);

/* Atualizacao ciclica - chamar a cada 10ms antes do RTE */
void    EcuAbs_Update(void);

#endif /* ECUABS_SENSORS_H */
```

**Step 2: Commit**
```bash
git add Common/EcuAbs/EcuAbs_Sensors.h
git commit -m "feat(ecuabs): add EcuAbs_Sensors.h API header"
```

---

### Task 2: Criar EcuAbs_Sensors.c (implementacao)

**Files:**
- Create: `Common/EcuAbs/EcuAbs_Sensors.c`

**Step 1: Implementar todas as funcoes de conversao**

Cada funcao segue o padrao:
1. Le mV via `Adc_ReadChannel_mV(ADC_CH_*)`
2. Converte mV para Volts (`/ 1000.0f`)
3. Aplica curva de calibracao
4. Satura nos limites fisicos do sensor
5. Retorna valor na unidade fisica

Coeficientes de calibracao extraidos do `rte_environment.c` atual:
- Air Temp: `T = 2.827*V^2 - 43.44*V + 135.1`, sat [-10, 129.75]
- Coolant: `T = -32.24*V + 133.43`, sat [-5.25, 143.25]
- MAP: `P = 25.16*V - 3.87`, sat [8.72, 121.96]
- Vbatt: `Vbat = mV * 14.0/5.0`
- Pedal: `adc8 = mV*255/5000`, `pct = (adc8-39)*100/(191-39)`, sat [0, 100]
- TPS: usa `tps_filtered_value` (extern do ecu_tasks.c)
- RPM: `CDD_Get_EngineSpeed_RAW()` passthrough
- Ethanol: fixo 275 (TODO)
- Phase/Ignition: `Dio_ReadChannel()` passthrough

`EcuAbs_Update()` atualiza variaveis internas static. Os getters retornam o ultimo valor atualizado (padrao AUTOSAR runnable).

**Step 2: Commit**
```bash
git add Common/EcuAbs/EcuAbs_Sensors.c
git commit -m "feat(ecuabs): implement sensor conversions with calibration curves"
```

---

### Task 3: Refatorar rte_environment.c para usar EcuAbs

**Files:**
- Modify: `Common/RTE/rte_environment.c`

**Step 1: Substituir todas as chamadas Adc_ReadChannel_mV + conversao por EcuAbs_Get*()**

Mudancas:
- `Update_RTE_AirTemperature()`: remover conversao manual, usar `EcuAbs_GetAirTemp_degC()`
- `Update_RTE_CoolantTemperature()`: remover conversao manual, usar `EcuAbs_GetCoolantTemp_degC()`
- `Update_RTE_ManAirPress()`: remover conversao manual, usar `EcuAbs_GetMAP_kPa()`
- `Update_RTE_V_BatteryCharge()`: remover conversao manual, usar `EcuAbs_GetVbatt_mV()`
- `Update_RTE_rpm_EngineSpeed()`: usar `EcuAbs_GetEngineSpeed_rpm()`
- `Update_RTE_p_EthanolPercent()`: usar `EcuAbs_GetEthanolPercent()`
- Adicionar `#include "EcuAbs_Sensors.h"`, remover `#include "Adc.h"` se nao houver uso direto

Cada funcao Update_ fica com ~3 linhas (get + atribuicao).

**Step 2: Commit**
```bash
git add Common/RTE/rte_environment.c
git commit -m "refactor(rte): use EcuAbs for sensor readings"
```

---

### Task 4: Refatorar rte_operator.c para usar EcuAbs

**Files:**
- Modify: `Common/RTE/rte_operator.c`

**Step 1: Substituir leitura do pedal**

- `Update_RTE_p_ThrottlePedal()`: remover conversao ADC->%, usar `EcuAbs_GetPedalPos_pct()`
- Adicionar `#include "EcuAbs_Sensors.h"`

**Step 2: Commit**
```bash
git add Common/RTE/rte_operator.c
git commit -m "refactor(rte): use EcuAbs for pedal position"
```

---

### Task 5: Integrar EcuAbs_Update() no ciclo de tarefas

**Files:**
- Modify: `Common/Tasks/ecu_tasks.c`

**Step 1: Chamar EcuAbs_Update() na EcuTask_10ms, antes do Task0_Run()**

```c
#include "EcuAbs_Sensors.h"

void EcuTask_10ms(void)
{
    EcuAbs_Update();  /* Atualiza sensores antes da logica de aplicacao */
    Task0_Run();
    FUEL_MainTask10ms();
    XcpBackground();
}
```

E chamar `EcuAbs_Init()` no `EcuTask_Init()` apos `Adc_Init()`.

**Step 2: Commit**
```bash
git add Common/Tasks/ecu_tasks.c
git commit -m "feat(tasks): integrate EcuAbs_Update in 10ms cycle"
```

---

### Task 6: Refatorar ecu_tasks.c para usar EcuAbs nos LEDs

**Files:**
- Modify: `Common/Tasks/ecu_tasks.c`

**Step 1: Substituir Dio_ReadChannel direto por EcuAbs**

- `EcuTask_5ms()`: `Dio_ReadChannel(DIO_CH_PHASE_STATE)` -> `EcuAbs_GetPhaseState()`
- `EcuTask_100ms()`: `Dio_ReadChannel(DIO_CH_IGNITION_ON)` -> `EcuAbs_GetIgnitionOn()`

**Step 2: Commit**
```bash
git add Common/Tasks/ecu_tasks.c
git commit -m "refactor(tasks): use EcuAbs for discrete inputs"
```

---

### Task 7: Adicionar EcuAbs ao build de ambas plataformas

**Files:**
- Modify: Build system TC297B (Makefile/subdir.mk)
- Modify: Build system STM32H7 (Makefile/subdir.mk)

**Step 1: Adicionar Common/EcuAbs/ ao include path e source list**

- TC297B: adicionar nos subdir.mk do AURIX Studio
- STM32H7: adicionar nos subdir.mk do STM32CubeIDE

**Step 2: Build de ambas plataformas**

Verificar: clean + build TC297B sem erros
Verificar: clean + build STM32H7 sem erros

**Step 3: Commit**
```bash
git add -A
git commit -m "build: add EcuAbs to TC297B and STM32H7 build"
```

---

### Task 8: Testar no hardware TC297B (HIL + Tricore)

**Step 1: Gravar TC297B e verificar com debug**

Watch variables:
- `EcuAbs_GetMAP_kPa()` — deve retornar ~100 kPa (pressao atmosferica)
- `EcuAbs_GetCoolantTemp_degC()` — deve retornar temperatura ambiente
- `EcuAbs_GetEngineSpeed_rpm()` — deve variar com RPM do HIL
- `ecu_current_state` — deve transitar de PRE_START -> CRANKING com rotacao

Confirmar que injecao funciona como antes (regressao zero).

**Step 2: Commit tag**
```bash
git tag -a v0.2.0-ecuabs -m "EcuAbs sensor abstraction layer"
```

---

## Notas de Implementacao

1. **Nao criar EcuAbs por plataforma** — vai tudo em Common porque as curvas de calibracao sao do sensor, nao da plataforma
2. **tps_filtered_value** — extern declarado em ecu_tasks.c, EcuAbs pode ler diretamente
3. **EcuAbs_Update() vs getters diretos** — EcuAbs_Update() le todos os sensores uma vez por ciclo (10ms), getters retornam o cache. Isso garante consistencia temporal (todos os sensores lidos no mesmo instante)
4. **Diagnostico de faixa** — manter os checks de voltagem min/max dentro do EcuAbs. Futuramente setar flags de diagnostico (DEM/DTC)
