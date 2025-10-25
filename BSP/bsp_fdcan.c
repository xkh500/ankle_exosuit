/* 通过STM32CubeMX初始化FDCAN控制后，还要进一步设置后，FDCAN才能正常运行。
 * 初始化滤波器 -> 初始化中断 -> 开启FDCAN控制
 */
#include "bsp_fdcan.h"
#include "main.h"
#include "bsp_usart1.h"
#include "detect_task.h"
#include "globals.h"

/* 外部变量 */
extern osThreadId motorHandle;
extern FDCAN_HandleTypeDef hfdcan1;       			   /* FDCAN1的对象句柄 */
FDCAN_FilterTypeDef hfdcan1_RX_Filter0;
FDCAN_TxHeaderTypeDef FDCAN1_TxHeader;
FDCAN_RxHeaderTypeDef FDCAN1_RxHeader;
motor_measure_t buffer1[BUFFER_SIZE];
motor_measure_t buffer2[BUFFER_SIZE];
motor_measure_t *volatile write_buffer=buffer1;
motor_measure_t *volatile process_buffer=buffer2;
volatile uint16_t write_index=0;
motor_measure_t latest_motor_data;  // 存储最新的电机数据
		
void buffer_append_int32(uint8_t* buffer, int32_t number, int32_t *index) {
	buffer[(*index)++] = number >> 24;
	buffer[(*index)++] = number >> 16;
	buffer[(*index)++] = number >> 8;
	buffer[(*index)++] = number;
}

void buffer_append_int16(uint8_t* buffer, int16_t number, int16_t *index) {
	buffer[(*index)++] = number >> 8;
	buffer[(*index)++] = number;
}

/**
  * 函数功能: 设置并初始化滤波器0,供FDCAN1使用
  * 输入参数: void
  *           
  * 返回值：  void
  *           
  *
  * 说明:
  *     1. 通过将成员FilterType设为FDCAN_FILTER_MASK,再加上成员FilterID1与FilterID2设为0x00,实现接收所有CAN ID.
  *     2. FDCAN1只设置了使用一个滤波器，所以成员FilterIndex设0，因为是从0开始的。
  *     3. FDCAN1设置了使用FIFO0存放CAN报文，所以滤波器需要关联到FIFO0.
  */
void FDCAN1_RX_Filter0_Init(void)
{
   hfdcan1_RX_Filter0.IdType = FDCAN_STANDARD_ID;              /* 只接收标准帧ID */
   hfdcan1_RX_Filter0.FilterIndex = 0;                         /* 滤波器索引0 */
   hfdcan1_RX_Filter0.FilterType = FDCAN_FILTER_MASK;          /* 滤波器类型 */
   //hfdcan1_RX_Filter0.FilterType = FDCAN_FILTER_RANGE;       /* 滤波器类型(允许接收报文的ID范围是FilterID1至FilterID2 */
   
   /* 注意：
    *  1.FDCAN_HandleTypeDef对象句柄的成员RxFifo0ElmtsNbr设置大于0时，表示启用RXFIFO0.
    *  2.
    *  3.如果启用RXFIFO0,那么滤波器必须关联到RXFIFO0,即FilterConfig必须赋值FDCAN_FILTER_TO_RXFIFO0.
    *  4.同理,如果启动RXFIFO1的话,滤波器必须关联到RXFIFO1,即FilterConfig必须赋值FDCAN_FILTER_TO_RXFIFO1.
    *
    */
   hfdcan1_RX_Filter0.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;  /* 滤波器关联到RXFIFO0 */
   hfdcan1_RX_Filter0.FilterID1 = 0x00;                        /* 滤波ID1: 0x00 */
   hfdcan1_RX_Filter0.FilterID2 = 0x00;                        /* 滤波ID2: 0x00 */
	
   /* 看看滤波器有没有创建成功 */
   if(HAL_FDCAN_ConfigFilter(&hfdcan1,&hfdcan1_RX_Filter0) != HAL_OK)
   {
        Error_Handler();
   }
}

/**
  * 函数功能: FDCAN1启动
  * 输入参数: void
  *           
  * 返回值：  void
  *           
  *
  * 说明:
  *    
  *      
  */
void FDCAN1_Start(void)
{
    HAL_FDCAN_Start(&hfdcan1); 
}

/**
  * 函数功能: 设置FDCAN1的中断
  * 输入参数: void
  *           
  * 返回值：  void
  *           
  *
  * 说明:
  *     1. 设置收到新的数据就产生中断。
  *     2. 配置水印中断防止FIFO溢出。
  *     3. 配置FIFO溢出中断，防止FIFO溢出导致丢失报文。
  */
void FDCAN1_Interrupt_Init(void)
{
   /*  在函数MX_FDCAN1_Init,将FDCAN1设置使用RXFIFO0(没有使用RXFIFO1),所以中断也需要判断RXFIFO0是不是有新数据 */
   if(HAL_FDCAN_ActivateNotification(&hfdcan1,FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0) != HAL_OK) /* 使能FDCAN中断(接收到新数据) */
   {
	   Error_Handler();
   }
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    uint8_t rxdata[8];
    if((RxFifo0ITs&FDCAN_IT_RX_FIFO0_NEW_MESSAGE)!=RESET)   //FIFO1新数据中断
    {
        //提取FIFO0中接收到的数据
        HAL_FDCAN_GetRxMessage(hfdcan,FDCAN_RX_FIFO0,&FDCAN1_RxHeader,rxdata);
				int16_t pos_int = (uint16_t)(rxdata[0] << 8 | rxdata[1]);
        int16_t spd_int = (uint16_t)(rxdata[2] << 8 | rxdata[3]);
        int16_t cur_int = (uint16_t)(rxdata[4] << 8 | rxdata[5]);
				float motor_pos = (float)(pos_int * 0.1f);
				// 检查 motor_pos 是否大于50或小于-50
        if (motor_pos > 200.0f || motor_pos < -5.0f)
        {
					if(motorHandle != NULL){
            // 如果超出范围，终止 motor_Task 线程
						CMD_START=0;
					}
        }
        write_buffer[write_index].motor_pos = motor_pos;
				write_buffer[write_index].motor_spd= (float)(spd_int * 10.0f)/31.5f;
        write_buffer[write_index].motor_cur= (float) (cur_int * 0.01f);
        write_buffer[write_index].motor_temp = rxdata[6];                                   \
				write_buffer[write_index].motor_error = rxdata[7];                     
				write_buffer[write_index].timestamp=detect_hook(0);
				// 使用互斥锁保护数据访问
				if(osMutexWait(motorDataHandle, osWaitForever) == osOK)
        {
           latest_motor_data=write_buffer[write_index];
            osMutexRelease(motorDataHandle);  // 释放互斥锁
        }
				// 更新写入索引
				write_index++;

				// 检查缓冲区是否已满
				if (write_index >= BUFFER_SIZE)
				{
						// 缓冲区已满，交换缓冲区指针
						motor_measure_t *temp = write_buffer;
						write_buffer = process_buffer;
						process_buffer = temp;
						write_index = 0;

						// 释放信号量，通知任务有新数据需要处理
						osSemaphoreRelease(bufferSemaphoreHandle);
				}
        HAL_FDCAN_ActivateNotification(hfdcan,FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0);
    }
}
uint8_t comm_can_transmit_eid(uint32_t id,uint8_t* msg,uint8_t _Len)
{	
    FDCAN1_TxHeader.Identifier=id;                             //32位ID
    FDCAN1_TxHeader.IdType=FDCAN_EXTENDED_ID;                  //标准ID
    FDCAN1_TxHeader.TxFrameType=FDCAN_DATA_FRAME;              //数据帧
    FDCAN1_TxHeader.DataLength=(uint32_t)_Len << 16;           //数据长度
    FDCAN1_TxHeader.ErrorStateIndicator=FDCAN_ESI_ACTIVE;      //错误状态指示器           
    FDCAN1_TxHeader.BitRateSwitch=FDCAN_BRS_OFF;               //关闭速率切换
    FDCAN1_TxHeader.FDFormat=FDCAN_CLASSIC_CAN;                //传统的CAN模式
    FDCAN1_TxHeader.TxEventFifoControl=FDCAN_NO_TX_EVENTS;     //无发送事件
    FDCAN1_TxHeader.MessageMarker=0;                           
    
    if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1,&FDCAN1_TxHeader,msg)!=HAL_OK) return 1;//发送
    return 0;	
}
void comm_can_set_rpm(uint8_t controller_id, float rpm) {
	int32_t send_index = 0;
	uint8_t buffer[4];
	rpm=rpm*31.5f;
	buffer_append_int32(buffer, (int32_t)rpm, &send_index);
	comm_can_transmit_eid(controller_id |
	((uint32_t)CAN_PACKET_SET_RPM << 8), buffer, send_index);
}
void comm_can_set_origin(uint8_t controller_id, uint8_t set_origin_mode) {
	uint8_t buffer;
	buffer=set_origin_mode;
	comm_can_transmit_eid(controller_id |
	((uint32_t)CAN_PACKET_SET_ORIGIN_HERE << 8), &buffer, 1);
}
void comm_can_set_pos_spd(uint8_t controller_id, float pos,int16_t spd, int16_t RPA ) {
	int32_t send_index = 0;
	int16_t send_index1 = 4;
	uint8_t buffer[8];
	buffer_append_int32(buffer, (int32_t)(pos * 10000.0f), &send_index);
	buffer_append_int16(buffer,spd/10.0f, &send_index1);
	buffer_append_int16(buffer,RPA/10.0f, &send_index1);
	comm_can_transmit_eid(controller_id |
	((uint32_t)CAN_PACKET_SET_POS_SPD << 8), buffer, send_index1);
}
void comm_can_set_pos(uint8_t controller_id, float pos) {
int32_t send_index = 0;
uint8_t buffer[4];
buffer_append_int32(buffer, (int32_t)(pos * 10000.0f), &send_index);
comm_can_transmit_eid(controller_id |
((uint32_t)CAN_PACKET_SET_POS << 8), buffer, send_index);
}
void comm_can_set_current(uint8_t controller_id, float current) {
 int32_t send_index = 0;
 uint8_t buffer[4];
 buffer_append_int32(buffer, (int32_t)(current * 1000.0f), &send_index);
 comm_can_transmit_eid(controller_id |
 ((uint32_t)CAN_PACKET_SET_CURRENT << 8), buffer, send_index);
 }
