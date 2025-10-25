#include "fatfs.h"
#include "sdmmc.h"
#include "bsp_fdcan.h"
#include <stdio.h>
#include "bsp_usart1.h"
#include <string.h>
#include "struct_typedef.h"

#define FF_MAX_SS		4096
char buffer[128];

//	函数：FatFs_FileTest
//	功能：进行文件写入和读取测试
//
uint8_t  FatFs_FileTest(void)	//文件创建和写入测试
{
	uint8_t i = 0;
	uint16_t BufferSize = 0;	
	FIL	MyFile;			// 文件对象
	UINT 	MyFile_Num;		//	数据长度
	BYTE 	MyFile_WriteBuffer[] = "STM32H7B0 SD卡 文件系统测试";	//要写入的数据
	BYTE 	MyFile_ReadBuffer[1024];	//要读出的数据
	uint8_t MyFile_Res;    /* Return value for SD */
	
	usart1_printf("-------------FatFs 文件创建和写入测试---------------\r\n");
	
	MyFile_Res = f_open(&MyFile,"0:FatFs Test.txt",FA_CREATE_ALWAYS | FA_WRITE);	//打开文件，若不存在则创建该文件
	if(MyFile_Res == FR_OK)
	{
		usart1_printf("文件打开/创建成功，准备写入数据...\r\n");
		
		MyFile_Res = f_write(&MyFile,MyFile_WriteBuffer,sizeof(MyFile_WriteBuffer),&MyFile_Num);	//向文件写入数据
		if (MyFile_Res == FR_OK)	
		{
			usart1_printf("写入成功，写入内容为：\r\n");
			usart1_printf("%s\r\n",MyFile_WriteBuffer);
		}
		else
		{
			usart1_printf("文件写入失败，请检查SD卡或重新格式化!\r\n");
			f_close(&MyFile);	  //关闭文件	
			return ERROR;			
		}
		f_close(&MyFile);	  //关闭文件			
	}
	else
	{
		usart1_printf("无法打开/创建文件，请检查SD卡或重新格式化!\r\n");
		f_close(&MyFile);	  //关闭文件	
		return ERROR;		
	}
	
	usart1_printf("-------------FatFs 文件读取测试---------------\r\n");	
	
	BufferSize = sizeof(MyFile_WriteBuffer)/sizeof(BYTE);									// 计算写入的数据长度
	MyFile_Res = f_open(&MyFile,"0:FatFs Test.txt",FA_OPEN_EXISTING | FA_READ);	//打开文件，若不存在则创建该文件
	MyFile_Res = f_read(&MyFile,MyFile_ReadBuffer,BufferSize,&MyFile_Num);			// 读取文件
	if(MyFile_Res == FR_OK)
	{
		usart1_printf("文件读取成功，正在校验数据...\r\n");
		
		for(i=0;i<BufferSize;i++)
		{
			if(MyFile_WriteBuffer[i] != MyFile_ReadBuffer[i])		// 校验数据
			{
				usart1_printf("校验失败，请检查SD卡或重新格式化!\r\n");
				f_close(&MyFile);	  //关闭文件	
				return ERROR;
			}
		}
		usart1_printf("校验成功，读出的数据为：\r\n");
		usart1_printf("%s\r\n",MyFile_ReadBuffer);
	}	
	else
	{
		usart1_printf("无法读取文件，请检查SD卡或重新格式化!\r\n");
		f_close(&MyFile);	  //关闭文件	
		return ERROR;		
	}	
	
	f_close(&MyFile);	  //关闭文件	
	return SUCCESS;
}
//	函数：FatFs_GetVolume
//	功能：计算设备的容量，包括总容量和剩余容量

void FatFs_GetVolume(void)	// 计算设备容量
{
	FATFS *fs;		//定义结构体指针
	uint32_t SD_CardCapacity = 0;		//SD卡的总容量
	uint32_t SD_FreeCapacity = 0;		//SD卡空闲容量
	DWORD fre_clust, fre_sect, tot_sect; 	//空闲簇，空闲扇区数，总扇区数

	f_getfree("0:",&fre_clust,&fs);			//获取SD卡剩余的簇

	tot_sect = (fs->n_fatent-2) * fs->csize;	//总扇区数量 = 总的簇 * 每个簇包含的扇区数
	fre_sect = fre_clust * fs->csize;			//计算剩余的可用扇区数	   

	SD_CardCapacity = tot_sect / 2048 ;	// SD卡总容量 = 总扇区数 * 512( 每扇区的字节数 ) / 1048576(换算成MB)
	SD_FreeCapacity = fre_sect / 2048 ;	//计算剩余的容量，单位为M
	usart1_printf("-------------------获取设备容量信息-----------------\r\n");		
	usart1_printf("SD容量：%dMB\r\n",SD_CardCapacity);	
	usart1_printf("SD剩余：%dMB\r\n",SD_FreeCapacity);
}

void FatFs_Check(void)	//判断FatFs是否挂载成功，若没有创建FatFs则格式化SD卡
{
	BYTE work[FF_MAX_SS]; 
	
	FATFS_LinkDriver(&SD_Driver, SDPath);		// 初始化驱动
	retSD = f_mount(&SDFatFS,"0:",1);	//	挂载SD卡
	
	if (retSD == FR_OK)	//判断是否挂载成功
	{
		usart1_printf("\r\nSD文件系统挂载成功\r\n");
		//FatFs_GetVolume();
	}
	else		
	{
		if(retSD == 13)
		{
			usart1_printf("SD卡还未创建文件系统，即将格式化\r\n");
			
			retSD = f_mkfs("0:",FM_FAT32,0,work,sizeof work);		//格式化SD卡，FAT32，簇默认大小16K
			
			if (retSD == FR_OK)		//判断是否格式化成功
				usart1_printf("SD卡格式化成功！\r\n");
			else
				usart1_printf("格式化失败，请检查或更换SD卡！\r\n");
		}
		else
			usart1_printf("挂载失败：%d\r\n",retSD);
	}
}
void printf_sdcard_info(void)
{
	HAL_SD_CardInfoTypeDef  SDCardInfo;  
	uint64_t CardCap;      	//SD卡容量
	HAL_SD_CardCIDTypeDef SDCard_CID; 

	HAL_SD_GetCardCID(&hsd1,&SDCard_CID);	//获取CID
	HAL_SD_GetCardInfo(&hsd1,&SDCardInfo);                    //获取SD卡信息
	CardCap=(uint64_t)(SDCardInfo.LogBlockNbr)*(uint64_t)(SDCardInfo.LogBlockSize);	//计算SD卡容量
	switch(SDCardInfo.CardType)
	{
		case CARD_SDSC:
		{
			if(SDCardInfo.CardVersion == CARD_V1_X)
				usart1_printf("Card Type:SDSC V1\r\n");
			else if(SDCardInfo.CardVersion == CARD_V2_X)
				usart1_printf("Card Type:SDSC V2\r\n");
		}
		break;
		case CARD_SDHC_SDXC:usart1_printf("Card Type:SDHC\r\n");break;
		default:break;
	}	
		
  usart1_printf("Card ManufacturerID: %d \r\n",SDCard_CID.ManufacturerID);				//制造商ID	
 	usart1_printf("CardVersion:         %d \r\n",(uint32_t)(SDCardInfo.CardVersion));		//卡版本号
	usart1_printf("Class:               %d \r\n",(uint32_t)(SDCardInfo.Class));		    //
 	usart1_printf("Card RCA(RelCardAdd):%d \r\n",SDCardInfo.RelCardAdd);					//卡相对地址
	usart1_printf("Card BlockNbr:       %d \r\n",SDCardInfo.BlockNbr);						//块数量
 	usart1_printf("Card BlockSize:      %d \r\n",SDCardInfo.BlockSize);					//块大小
	usart1_printf("LogBlockNbr:         %d \r\n",(uint32_t)(SDCardInfo.LogBlockNbr));		//逻辑块数量
	usart1_printf("LogBlockSize:        %d \r\n",(uint32_t)(SDCardInfo.LogBlockSize));		//逻辑块大小
	usart1_printf("Card Capacity:       %d MB\r\n",(uint32_t)(CardCap>>20));				//卡容量
}

void sdcard_Task(void const * argument)
{
  /* USER CODE BEGIN sdcard_Task */
	char buffer[128];
	FIL file;
	static bool_t file_opened = 0; // 文件默认是关闭状态
	FatFs_Check();
	//printf_sdcard_info();
	//FatFs_FileTest();
  /* Infinite loop */
  for(;;)
  {
		// 检查串口命令来控制文件操作
	if (xUART1.ReceiveNum)
	{
		if (strcmp((char*)xUART1.ReceiveData, "start") == 0)
		{
			if (!file_opened)
			{
				// 尝试打开或创建CSV文件，使用 FA_CREATE_ALWAYS 模式来覆盖文件内容
				if(f_open(&file, "can_data.csv", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
				{
					usart1_printf("无法打开/创建文件，请检查SD卡或重新格式化!\r\n");
					f_close(&file);	  //关闭文件	
					vTaskDelete(NULL);
				}
				else
				{
					usart1_printf("文件打开成功，启用文件写入。\r\n");
					file_opened = 1; // 设置文件为打开状态
					// 写入CSV文件头
					if (f_puts("Time,Pos,Spd,Cur,Temp,Error\n", &file) == EOF)
					{
						usart1_printf("文件头写入失败！\r\n");
						f_close(&file); // 确保关闭文件
						vTaskDelete(NULL);
					}
				}
			}
			else
			{
			usart1_printf("文件已经打开，无需重复打开。\r\n");
			}
		}
		else if (strcmp((char*)xUART1.ReceiveData, "stop") == 0)
		{
			if (file_opened)
			{
				usart1_printf("停止写入数据，关闭文件...\r\n");
				f_close(&file); // 关闭文件
				file_opened = 0; // 设置文件为关闭状态
			}
			else
			{
			usart1_printf("文件尚未打开，无法关闭。\r\n");
			}
		}
		// 清除接收计数，准备接收下一条命令
		xUART1.ReceiveNum = 0;
	}

			// 只有在文件打开且写入启用时才进行写入操作
			//UBaseType_t stackWatermark=uxTaskGetStackHighWaterMark(NULL);
			//usart1_printf("任务剩余堆栈大小:%d words\r\n",stackWatermark);
		if (osSemaphoreWait(bufferSemaphoreHandle, osWaitForever) == osOK&&file_opened)
		{
				// 处理接收到的数据
				for (int i = 0; i < BUFFER_SIZE; i++)
				{
						//memset(buffer,0,sizeof(buffer));
						// 格式化数据行
						snprintf(buffer, sizeof(buffer), "%d,%f,%f,%f,%d,%d\n",
						process_buffer[i].timestamp,
						process_buffer[i].motor_pos,
						process_buffer[i].motor_spd,
						process_buffer[i].motor_cur,
						process_buffer[i].motor_temp,
						process_buffer[i].motor_error);
						// 写入数据到CSV文件
						UINT bytesWritten;
						if (f_write(&file, buffer, strlen(buffer), &bytesWritten) != FR_OK)
						{
								// 处理写入错误
								usart1_printf("文件写入失败，请检查SD卡或重新格式化!\r\n");
								break;
						}
				}

				// 刷新缓冲区到存储设备
				f_sync(&file);
			}
  }
  /* USER CODE END sdcard_Task */
}
