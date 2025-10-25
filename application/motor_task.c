#include "bsp_fdcan.h"
#include "bsp_usart1.h"
#include "globals.h"
#include "detect_task.h"

#define id 1
#define PI 3.14159265
// 状态机的状态定义
typedef enum {
    WAIT_FOR_HS_START,  // 等待足跟着地开始
    WAIT_FOR_HS_END,    // 等待足跟离地
} GaitState;

void motor_Task(void const * argument)
{
  /* USER CODE BEGIN motor_Task */
    FDCAN1_RX_Filter0_Init();
    FDCAN1_Start();
    FDCAN1_Interrupt_Init();
    vTaskDelay(500);
    comm_can_set_origin(id, 0);
    vTaskDelay(50);
    
    GaitState currentState = WAIT_FOR_HS_START;
	
//    TickType_t lastHeelStrikeTime = 0; //上一次脚跟着地时间
//    TickType_t gaitCycleDuration = 1000; // 步态周期持续时间，初始值可以设置一个合理的默认值
	
    //float time=0;
    /* Infinite loop */
    for(;;)
    {
        voltage=ADC_ConvertedValue*3.3f/4096;
        voltage=3.3f-voltage;
      
        
        //当前的步态检测算法的逻辑要求H1>=H2，因为如果H2>H1的话，在电压值＜H2后会立即判定为>H1，这样就不能拿到步态周期了。
        switch (currentState) 
        {
           case WAIT_FOR_HS_START:
                if (voltage > HSThreshhold1)   
                { 
                    // 检测到足跟着地开始
                    TickType_t currentTime = xTaskGetTickCount();
                    gaitCycleDuration = currentTime - lastHeelStrikeTime;
                    lastHeelStrikeTime = currentTime;
									
                    if(gaitCycleDuration<5000&&gaitCycleDuration>500) 
                    {
                        // 启动方波生成
                        xTaskNotify(profileHandle, NOTIFICATION_VALUE, eSetBits);
                    }
                    // 转到等待足跟离地状态
                    currentState = WAIT_FOR_HS_END;
                }
                break;

            case WAIT_FOR_HS_END:
                if (voltage < HSThreshhold2)   
                {
                    // 检测到足跟离地
                    currentState = WAIT_FOR_HS_START;
                }
                break;
            default:
                break;
        }
        
        if(toe_is_error(0)==1)
        {
            comm_can_set_current(id,0);
            vTaskDelete(NULL);
        }
        else if(CMD_START)
        {
            comm_can_set_pos(id, trajectoryValue);
        }
        else
            comm_can_set_current(id,0);
        osDelay(5);
    }
  /* USER CODE END motor_Task */
}
