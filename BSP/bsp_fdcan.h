#ifndef __BSP_FDCAN_H__
#define __BSP_FDCAN_H__

#include "fdcan.h"
#include "cmsis_os.h"
#define BUFFER_SIZE 128

typedef enum {
	CAN_PACKET_SET_DUTY = 0,         //占空比模式
	CAN_PACKET_SET_CURRENT,         //电流环模式
	CAN_PACKET_SET_CURRENT_BRAKE,   // 电流刹车模式
	CAN_PACKET_SET_RPM,             // 转速模式
	CAN_PACKET_SET_POS,              // 位置模式
	CAN_PACKET_SET_ORIGIN_HERE,      //设置原点模式
	CAN_PACKET_SET_POS_SPD,          //位置速度环模式
} CAN_PACKET_ID;

//motor data
typedef struct
{
		uint32_t timestamp;
	  int8_t motor_temp;
    uint8_t motor_error;
		float motor_pos;
		float motor_spd;
		float motor_cur;
} motor_measure_t;

extern motor_measure_t buffer1[BUFFER_SIZE];
extern motor_measure_t buffer2[BUFFER_SIZE];
extern motor_measure_t *volatile write_buffer;
extern motor_measure_t *volatile process_buffer;
extern volatile uint16_t write_index;
extern osSemaphoreId bufferSemaphoreHandle;
extern osMutexId motorDataHandle;
extern motor_measure_t latest_motor_data;
void FDCAN1_RX_Filter0_Init(void);
void FDCAN1_Interrupt_Init(void);
void FDCAN1_Start(void);
void comm_can_set_rpm(uint8_t controller_id, float rpm);
void comm_can_set_origin(uint8_t controller_id, uint8_t set_origin_mode);
void comm_can_set_pos_spd(uint8_t controller_id, float pos,int16_t spd, int16_t RPA );
void comm_can_set_pos(uint8_t controller_id, float pos);
void comm_can_set_current(uint8_t controller_id, float current);

#endif /*__BSP_FDCAN_H */

