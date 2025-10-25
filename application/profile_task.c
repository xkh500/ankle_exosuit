#include "main.h"
#include "globals.h"
/* USER CODE END Header_profile_Task */
void profile_Task(void const * argument)
{
  /* USER CODE BEGIN profile_Task */
	uint32_t notificationValue;
  /* Infinite loop */
  for(;;)
  {
        // Wait for notification from gait_Task
        xTaskNotifyWait(0x00,    // Clear no bits on entry
                        0xFFFFFFFF, // Clear all bits on exit
                        &notificationValue, // Store the notification value here
                        portMAX_DELAY); // Wait indefinitely

        if (notificationValue & NOTIFICATION_VALUE) {
            TickType_t riseTime = gaitCycleDuration * rise_time / 100; 
            TickType_t fallTime = gaitCycleDuration * fall_time / 100; 

            // 初始为低
            trajectoryValue = 0.0f;

            // 等待上升时间
            osDelay(riseTime);

            // 上升为高
            trajectoryValue = maxvalue;

            // 等待下降时间
            osDelay(fallTime - riseTime);

            // 下降为低
            trajectoryValue = 0.0f;

        }
  }
  /* USER CODE END profile_Task */
}
//void profile_Task(void const * argument)
//{
//    /* USER CODE BEGIN profile_Task */
//    uint32_t notificationValue;

//    /* Infinite loop */
//    for(;;)
//    {
//        // Wait for notification from gait_Task
//        xTaskNotifyWait(0x00,    // Clear no bits on entry
//                        0xFFFFFFFF, // Clear all bits on exit
//                        &notificationValue, // Store the notification value here
//                        portMAX_DELAY); // Wait indefinitely

//        if (notificationValue & NOTIFICATION_VALUE) {
//            
//            // 计算时间点
//					double rise_percentage=rise_time / 100;
//          double fall_percentage=fall_time / 100;
//					
//					for(;;)
//					{
//						int a1=0;//上升段完成情况
//						int a2=0;//下降段完成情况
//						
//						
//						TickType_t currentTime = xTaskGetTickCount();  //当前时间
//					  double Percentage=(currentTime-lastHeelStrikeTime)/gaitCycleDuration;
//						
//						// 初始为低
//            trajectoryValue = 0.0f;
//						
//						if(Percentage>rise_percentage && trajectoryValue == 0.0f)
//						{
//							// 上升为高
//              trajectoryValue = maxvalue;
//							a1=1;
//						}
//						
//						if(Percentage>fall_percentage && trajectoryValue == maxvalue)
//						{
//							trajectoryValue = 0.0f;
//							a2=1;
//						}
//						
//						if(a1==1 && a2==1)
//							break;
//						
//					}
//					
//        }
//    }

//    /* USER CODE END profile_Task */
//}
