#include "globals.h"

// 全局变量定义
volatile uint8_t CMD_START = 0;
float maxvalue = 100.0f;
float HSThreshhold1 = 0.7f;
float HSThreshhold2 = 0.3f;
int rise_time = 50;
int fall_time = 80;
fp32 trajectoryValue = 0;
TickType_t gaitCycleDuration=1000;
TickType_t lastHeelStrikeTime = 0; 
fp32 voltage=0;
float ref=0;
// 其他共享的变量定义...
