# Ankle Exosuit Controller

Private STM32H743 project by **xkh500**

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
