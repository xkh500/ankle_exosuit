#ifndef BSP_USART_H
#define BSP_USART_H
#include "struct_typedef.h"
/* UART收发缓冲大小 */
#define ATK_MW8266D_UART_RX_BUF_SIZE            128
#define ATK_MW8266D_UART_TX_BUF_SIZE            64
typedef struct
{
uint16_t ReceiveNum; // 接收字节数; 在中断回调函数里被自动赋值，只要字节数>0，即为接收到新一帧的所有数据。
uint8_t ReceiveData[128]; // 接收到的数据
uint8_t BuffTemp[128]; // 接收缓存; 这是一个缓存，用于DMA逐个字节接收，接收完成中断调用后会把这个数组内容复制到ReceiveData数组里。
} xUATR_TypeDef;
extern xUATR_TypeDef xUART1; // 定义串口1的数据接收结构体，方便管理变量
extern xUATR_TypeDef xUART3; // 定义串口3的数据接收结构体，方便管理变量

void usart1_DMAprintf(const char *fmt,...);
void usart1_printf(const char *format,...);

/* 操作函数 */
void atk_mw8266d_uart_printf(char *fmt, ...);       /* ATK-MW8266D UART printf */
#endif
