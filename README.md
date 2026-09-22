# Ankle Exosuit Controller

STM32H743 ankle exosuit controller by **xkh500**.

## Overview
Flexible ankle exosuit controller using FreeRTOS.
- IDE: Keil MDK v5 / CubeMX 6.4
- MCU: STM32H743
- Features:
  - Motor control via FDCAN
  - Sensor fault detection (detect_task)
  - Gait profile execution (profile_task)
  - [Removed] SD & WiFi modules for lightweight build

## Structure
- `Core/` — CubeMX generated init code (FDCAN, ADC, DMA, USART, FreeRTOS)
- `BSP/` — board support: FDCAN driver (`bsp_fdcan`), USART1 driver (`bsp_usart1`), `struct_typedef`
- `application/` — FreeRTOS tasks: `motor_task` (FDCAN motor control + gait state machine), `profile_task` (gait profile execution), `detect_task` (sensor fault detection), `globals`
- `MDK-ARM/` — Keil MDK project (`ankle_exosuit.uvprojx`)
- `Drivers/`, `Middlewares/` — STM32 HAL, CMSIS, FreeRTOS

## Build
Open `MDK-ARM/ankle_exosuit.uvprojx` in Keil MDK v5 and build, or regenerate
the MDK project from `ankle_exosuit.ioc` with STM32CubeMX 6.4.

## License
MIT — see [LICENSE](LICENSE).

`application/detect_task.c` and `application/detect_task.h` are derived from DJI's
RoboMaster development board example code and remain under DJI's copyright
(`(C) COPYRIGHT 2019 DJI`); they are not covered by this project's MIT license.
