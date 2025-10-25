#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>
#include "cmsis_os.h"
#include "struct_typedef.h"
// 全局变量声明
#define NOTIFICATION_VALUE 0x01
extern osThreadId profileHandle;
extern uint16_t ADC_ConvertedValue;
extern volatile uint8_t CMD_START;
extern float maxvalue;
extern float HSThreshhold1;
extern float HSThreshhold2;
extern int rise_time;
extern int fall_time;
extern fp32 trajectoryValue;
extern fp32 voltage;
// 全局变量，用于存储检测到的步态周期、当前轨迹值和上次脚跟着地时间
extern TickType_t lastHeelStrikeTime;
extern TickType_t gaitCycleDuration;
extern float ref;

#endif // GLOBALS_H
