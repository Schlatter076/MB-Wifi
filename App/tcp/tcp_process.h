/*
 * process.h
 *
 *  Created on: 2020��11��11��
 *      Author: loyer
 */

#ifndef _TCP_PROCESS_H_
#define _TCP_PROCESS_H_

#include "stm32f10x.h"
#include "malloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tcp_public.h"
#include <stdarg.h>
#include "bat_usart.h"
#include "tim4.h"
#include "STMFlash.h"

typedef struct
{
	char ssid[100];
	char pwd[100];
} ParamsOfWifiJoinAP_TypeDef;

extern ParamsOfWifiJoinAP_TypeDef ParamsOfWifiJoinAPInit;

typedef enum
{
	DOWN_RegiseterSucc = 31,
	DOWN_RecivedAllPortsSTA = 33,
	DOWN_SystemPopupPowerbank = 34,
	DOWN_RecivedSystemPopupSTA = 36,
	DOWN_RecivedPowerbankSTAChanged = 38,
	DOWN_RecivedStatuHeart = 91,
	DOWN_ForceHeart = 92,
	DOWN_RecivedForceHeart = 94,
	DOWN_DeviceReset = 60,
	DOWN_SetWifiHeart = 62,
	DOWN_SetWifiSsidAndPwd = 66,
	DOWN_OrderPopupPowerbank = 40,
	DOWN_RecivedOrderPopupPowerbank = 42,
	DOWN_VoiceBroadcast = 71,
	DOWN_PopupAllPowerbanks = 99,
	DOWN_IgnoreLock = 77,
	DOWN_SetID = 12,
	DOWN_RemoteCtrMotor = 73
} ENUM_tcpDOWN_TypeDef;

typedef enum
{
	UP_Regiser = 30,
	UP_AllPortsSTA = 32,
	UP_SystemPopupSTA = 35,
	UP_PowerbankSTAChanged = 37,
	UP_StatuHeart = 90,
	UP_ForceHeart = 93,
	UP_DeviceRest = 61,
	UP_SetWifiHeart = 63,
	UP_SetWifiSsidAndPwd = 67,
	UP_OrderPopupPowerbank = 41,
} ENUM_tcpUP_TypeDef;

void TCP_sendStr(USART_TypeDef* USARTx, char *str);
void getWifiSsidAndPwd(char *datas, ParamsOfWifiJoinAP_TypeDef *Powj_Init);
void getRegisterStr(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD,
		struct STRUCT_USART_Params *params, char moduleType, char *version,
		char *num);
void getRequestStrWithoutParam(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD);
void getPowerbankSTAStr(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD, u8 rssi,
		int portNum, char *portSTAStr, ...);
void getPowerbankSTAStrWithoutRSSI(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD,
		int portNum, char *portSTAStr, ...);
void ProcessServerCmd(ENUM_Internet_TypeDef internet,
		struct STRUCT_USART_Fram *fram, struct STRUCT_USART_Params *params);
void systemPopup(ENUM_Internet_TypeDef internet, struct STRUCT_USART_Fram *fram,
		struct STRUCT_USART_Params *params);
void orderPopup(ENUM_Internet_TypeDef internet, struct STRUCT_USART_Fram *fram,
		struct STRUCT_USART_Params *params);
void modifyLockSTA(struct STRUCT_USART_Fram *fram);
void commonHeart(USART_TypeDef* USARTx);
void forceHeart(ENUM_Internet_TypeDef internet,
		struct STRUCT_USART_Params *params, ENUM_tcpUP_TypeDef upCmd);
void responseReset(ENUM_Internet_TypeDef internet);
void reportPortStatuChanged(u8 port, USART_TypeDef* USARTx);
void request4Register(USART_TypeDef* USARTx);
void setWifiSsidAndPwd(ENUM_Internet_TypeDef internet, struct STRUCT_USART_Fram *fram);
void getRegisterParams(struct STRUCT_USART_Fram *fram);
void setMotorRun(struct STRUCT_USART_Fram *fram);

#endif /* _TCP_PROCESS_H_ */
