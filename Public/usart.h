/*
 * usart.h
 *
 *  Created on: 2019年10月19日
 *      Author: Loyer
 */

#ifndef USART_H_
#define USART_H_

#include "bit_band.h"
#include "STMFlash.h"

#define RX_BUF_MAX_LEN 1024		  //最大接收缓存字节数
extern struct USART_Fram	  //定义一个全局串口数据帧的处理结构体
{
	char RX_BUF[RX_BUF_MAX_LEN];
	union
	{
		__IO u16 InfAll;
		struct
		{
			__IO u16 FramLength :15;                               // 14:0
			__IO u16 FramFinishFlag :1;                                // 15
		} InfBit;
	};
} USART1_Record_Struct;

extern struct RegisterFram	  //定义一个全局串口数据帧的处理结构体
{
	char key[16];
	u8 motor_TCtime;
	u8 motor_HTtime;
	char ip[30];
	u8 needConfirmParams;
	u8 heartTime;
	u8 statuHeartTime;
	u8 allowHeart;
	char port[5];
} RegisterParams;

void USART1_Init(u32 bound);
u8 hexStr2Byte(char *hexStr);

#endif /* USART_H_ */
