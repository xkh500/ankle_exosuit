#include "bsp_usart1.h"
#include "main.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"
extern UART_HandleTypeDef huart1;
//extern UART_HandleTypeDef huart3;
xUATR_TypeDef xUART1 = {0}; // 定义结构体，用于UART1。也可以不用结构体，用单独的变量
//xUATR_TypeDef xUART3 = {0}; // 定义结构体，用于UART1。也可以不用结构体，用单独的变量
void usart1_DMAprintf(const char *fmt,...)
{
		static uint8_t dmatx_buf[256] = {0};
    static va_list ap;
    static uint16_t len;
    va_start(ap, fmt);

    //return length of string 
    //返回字符串长度
    len = vsprintf((char *)dmatx_buf, fmt, ap);

    va_end(ap);

    HAL_UART_Transmit_DMA(&huart1, dmatx_buf, len);

}

void usart1_printf(const char *format,...)
{
	static uint8_t tx_buf[256] = {0};
	static uint16_t len;
	static va_list args;	
	va_start(args,format);
	len = vsnprintf((char*)tx_buf,sizeof(tx_buf)+1,(char*)format,args);
	va_end(args);
	HAL_UART_Transmit(&huart1, tx_buf, len,0xff);
}


/******************************************************************************
* 函 数： HAL_UARTEx_RxEventCallback
* 功 能： DMA+空闲中断回调函数
* 参 数： UART_HandleTypeDef *huart // 触发的串口
* uint16_t Size // 接收字节
* 返回值： 无
* 备 注： 1：这个是回调函数，不是中断服务函数。技巧：使用CubeMX生成的工程中，中断服务函数已被CubeMX安排妥当，我们只管重写回调函数
* 2：触发条件：当DMA接收到指定字节数时，或产生空闲中断时，硬件就会自动调用本回调函数，无需进行人工调用;
* 2：必须使用这个函数名称，因为它在CubeMX生成时，已被写好了各种函数调用、函数弱定义(在stm32xx_hal_uart.c的底部); 不要在原弱定义中增添代码，而是重写本函数
* 3：无需进行中断标志的清理，它在被调用前，已有清中断的操作;
* 4：生成的所有DMA+空闲中断服务函数，都会统一调用这个函数，以引脚编号作参数
* 5：判断参数传进来的引脚编号，即可知道是哪个串口接收收了多少字节
******************************************************************************/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart == &huart1) // 判断串口
	{
		__HAL_UNLOCK(huart); // 解锁串口状态

		xUART1.ReceiveNum = Size; // 把接收字节数，存入结构体xUSART1.ReceiveNum，以备使用
		memset(xUART1.ReceiveData, 0, sizeof(xUART1.ReceiveData)); // 清0前一帧的接收数据
		memcpy(xUART1.ReceiveData, xUART1.BuffTemp, Size); // 把新数据，从临时缓存中，复制到xUSART1.ReceiveData[], 以备使用
		//TickType_t Time = xTaskGetTickCount();
		//usart1_DMAprintf("%d\r\n",Time);
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, xUART1.BuffTemp, sizeof(xUART1.BuffTemp)); // 再次开启DMA空闲中断; 每当接收完指定长度，或者产生空闲中断时，就会来到这个
	}
	/*if (huart == &huart3) // 判断串口
	{
		__HAL_UNLOCK(huart); // 解锁串口状态

		xUART3.ReceiveNum = Size; // 把接收字节数，存入结构体xUSART1.ReceiveNum，以备使用
		memset(xUART3.ReceiveData, 0, sizeof(xUART3.ReceiveData)); // 清0前一帧的接收数据
		memcpy(xUART3.ReceiveData, xUART3.BuffTemp, Size); // 把新数据，从临时缓存中，复制到xUSART1.ReceiveData[], 以备使用
		HAL_UARTEx_ReceiveToIdle_DMA(&huart3, xUART3.BuffTemp, sizeof(xUART3.BuffTemp)); // 再次开启DMA空闲中断; 每当接收完指定长度，或者产生空闲中断时，就会来到这个
	}*/
}

