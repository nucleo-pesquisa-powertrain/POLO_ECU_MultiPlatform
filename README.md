# POLO ECU MultiPlatform

## Sobre

Software embarcado de gerenciamento eletrônico do motor (ECU) para o veículo Polo, com arquitetura unificada AUTOSAR-like que suporta duas plataformas de hardware:

- **Infineon AURIX TC297B** (TriCore, 300 MHz) — plataforma principal
- **STM32H745** (Cortex-M7 + M4, 480 MHz) — plataforma alternativa

O mesmo código de aplicação (ASW, RTE, CDD, XCP) roda em ambas as plataformas. Somente a camada MCAL (drivers de hardware) é específica por microcontrolador.

## Estrutura do Repositório

```
POLO_ECU_MultiPlatform/
├── Apresentacoes_e_Eventos/     Apresentações e eventos do projeto
├── Artigos_e_TCCs/              Artigos e TCCs produzidos
├── Materiais/                   Diagramas, datasheets, documentação
├── Referencias/                 Material de referência
├── Software/                    Código fonte
│   └── ECU_Unified/
│       ├── Common/              Código compartilhado (ambas plataformas)
│       │   ├── MCAL/Api/        API MCAL padronizada (Dio, Adc, Gpt, Icu, Can, Spi, Pwm)
│       │   ├── CDD/             Custom Device Drivers (crankshaft, sync, spark, injectors)
│       │   ├── RTE/             Runtime Environment (sensores, componentes)
│       │   ├── XCP/             Protocolo XCP para calibração (Vector Basic)
│       │   ├── ASW/             Application Software (fuel, spark, idle, throttle, TBI)
│       │   └── Tasks/           Corpo das tasks periódicas (5/10/20/100ms)
│       │
│       ├── Platform_TC297B/     Implementação Infineon AURIX
│       │   ├── MCAL/            Wrappers iLLD (Dio→IfxPort, Gpt→IfxGpt12, etc.)
│       │   ├── Libraries/       iLLD + Infra + Service
│       │   ├── HardwareAdp/     Init de periféricos (ADC, ERU, GPT12)
│       │   ├── Main/            Cpu0/1/2_Main.c (bare-metal scheduler)
│       │   └── TFT/             Display (exclusivo TC297B)
│       │
│       └── Platform_STM32H7/    Implementação STM32
│           ├── MCAL/            Wrappers STM32 HAL (Dio→HAL_GPIO, Gpt→HAL_TIM, etc.)
│           ├── Drivers/         STM32 HAL + CMSIS
│           ├── CubeMX/          Código gerado pelo STM32CubeMX
│           ├── Middlewares/     FreeRTOS
│           ├── Main/            main_ecu.c (FreeRTOS scheduler)
│           └── CM4/             Core secundário Cortex-M4
└── README.md
```

## Arquitetura de Software

```
┌───────────────────────────────────────────┐
│         ASW (Application Software)         │  COMPARTILHADO
│   Fuel, Spark, Idle, Throttle, TBI, State  │
├───────────────────────────────────────────┤
│         RTE (Runtime Environment)           │  COMPARTILHADO
│   Sensores, componentes, sinais             │
├───────────────────────────────────────────┤
│         CDD (Custom Device Drivers)         │  COMPARTILHADO
│   Crankshaft, Sync, Spark, Injectors        │
├───────────────────────────────────────────┤
│         XCP (Calibração/Medição)            │  COMPARTILHADO
│   Vector XCP Basic over CAN                 │
├───────────────────────────────────────────┤
│         MCAL API (Interface Padronizada)    │  COMPARTILHADO
│   Dio, Adc, Gpt, Icu, Can, Spi, Pwm        │
├──────────────────┬────────────────────────┤
│  MCAL TC297B     │   MCAL STM32H7         │  SEPARADO
│  iLLD wrappers   │   STM32 HAL wrappers   │
└──────────────────┴────────────────────────┘
```

## Como Compilar

### Infineon AURIX TC297B
1. Abrir **AURIX Development Studio**
2. Importar projeto de `Software/ECU_Unified/Platform_TC297B/`
3. Build define: `PLATFORM_TC297B`

### STM32H745
1. Abrir **STM32CubeIDE**
2. Importar projeto de `Software/ECU_Unified/Platform_STM32H7/`
3. Build define: `PLATFORM_STM32H7`

## Comunicação

- **XCP over CAN**: 500 kbps, RX ID 0x600, TX ID 0x601
- **Compatível com CANape** para calibração e medição em tempo real

## Núcleo de Pesquisa Powertrain
