/*
 * tcp_public.h
 *
 *  Created on: 2020��10��24��
 *      Author: loyer
 */

#ifndef _TCP_PUBLIC_H_
#define _TCP_PUBLIC_H_

#include "stm32f10x.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "systick.h"
#include "malloc.h"
#include "stdarg.h"

#define TCP_MAX_LEN 1024		  //�����ջ����ֽ���
#define BASE64_BUF_LEN 512

struct STRUCT_USART_Fram  //����һ��ȫ�ִ�������֡�Ĵ����ṹ��
{
	unsigned char Data[TCP_MAX_LEN];
	unsigned char DeData[BASE64_BUF_LEN];
	unsigned char EnData[BASE64_BUF_LEN];
	char *base64Str;
	unsigned char ServerData[BASE64_BUF_LEN];
	unsigned char *Server_Command[2];
	volatile unsigned char IsNotInAT;
	volatile u8 linkedClosed;
	volatile u8 allowProcessServerData;
	volatile u8 init;
	volatile u8 registerSuccess;
	volatile u8 firstStatuHeartNotSucc;
	volatile u8 allowCheckChange;
	volatile u8 serverStatuCnt;
	union
	{
		__IO u16 InfAll;
		struct
		{
			__IO u16 Length :15;                               // 14:0
			__IO u16 FinishFlag :1;                                // 15
		} InfBit;
	};
};
extern struct STRUCT_USART_Fram F4G_Fram;
extern struct STRUCT_USART_Fram WIFI_Fram;
extern volatile u8 CurrentInternet;

extern struct STRUCT_USART_Params
{
	char locations[2][12];
	unsigned char ccid[24];
	unsigned char cops;
	u8 rssi; //�ź�ǿ��
	volatile u8 STHeart;
	volatile u8 reqSTheart;
	volatile u8 sysTC;
	volatile u8 ddTC;
	char cmd[2];
	volatile int port;
	char dd[20];
	volatile u8 play;
	u8 statuCode[6];
	u8 currentStatuCode[6];
	char htCMD[2];
	volatile u8 checkPBst;
	volatile u8 getLongitude;
	volatile u8 popAll;
	volatile u8 setID;
	volatile u8 process4G;
	volatile u8 processWIFI;
} TCP_Params;

typedef enum
{
	STA, AP, STA_AP
} ENUM_Net_ModeTypeDef;

typedef enum
{
	enumTCP, enumUDP,
} ENUM_NetPro_TypeDef;

typedef enum
{
	Multiple_ID_0 = 0,
	Multiple_ID_1 = 1,
	Multiple_ID_2 = 2,
	Multiple_ID_3 = 3,
	Multiple_ID_4 = 4,
	Single_ID_0 = 5,
} ENUM_ID_NO_TypeDef;

typedef enum
{
	In4G = 0, InWifi,
} ENUM_Internet_TypeDef;

void _USART_printf(USART_TypeDef * USARTx, char * Data, ...);
bool Send_AT_Cmd(ENUM_Internet_TypeDef internet, char *cmd, char *ack1,
		char *ack2, u32 time);
bool AT_Test(ENUM_Internet_TypeDef internet);

#endif /* _TCP_PUBLIC_H_ */