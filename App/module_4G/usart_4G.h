/*
 * usart_4G.h
 *
 *  Created on: 2020年6月11日
 *      Author: loyer
 */

#ifndef USART_4G_H_
#define USART_4G_H_

#include "stm32f10x.h"
#include "tcp_public.h"
#include "stdio.h"
#include <string.h>
#include <stdbool.h>
#include "systick.h"
#include "base64.h"
#include "stdlib.h"
#include "STMFlash.h"
#include "usart.h"
#include "bat_usart.h"
#include "audio.h"
#include "motor.h"
#include "app.h"

#define TCPServer_IP    "server.dayitc.com"
#define TCPServer_PORT  "13401"

#define F4G_USART(fmt, ...)	 _USART_printf (USART2, fmt, ##__VA_ARGS__)
#define PC_USART(fmt, ...)	 printf (fmt, ##__VA_ARGS__)

#define UART2_4G_TX GPIO_Pin_2 //GPIOA 4G模块通信
#define UART2_4G_RX GPIO_Pin_3
#define RST_4G      GPIO_Pin_0 //GPIOC
#define PKEY_4G     GPIO_Pin_1

#define RST_4G_Pin_SetH     GPIO_SetBits(GPIOC,RST_4G)
#define RST_4G_Pin_SetL     GPIO_ResetBits(GPIOC,RST_4G)
#define PKEY_4G_Pin_SetH     GPIO_SetBits(GPIOC,PKEY_4G)
#define PKEY_4G_Pin_SetL     GPIO_ResetBits(GPIOC,PKEY_4G)

#define REQ_REGISTER  "30"
#define RES_REGISTER  "31"
#define REQ_PORT_STATU  "32"
#define RES_PORT_STATU  "33"
#define RES_TC_POWERBANK  "34"
#define REQ_TC_POWERBANK   "35"
#define REQ_STATU_HEART   "90"
#define RES_STATU_HEART   "91"
#define RES_ENFORCE_HEART  "92"
#define REQ_ENFORCE_HEART  "93"

bool ConnectToServerBy4G(char* addr, char* port);
void USART2_Init(uint32_t rate);
void F4G_Init(u32 bound);
void reset_4G_module(void);
void split(char str[], char *delims);
void F4G_sendStr(char *str);
void F4G_ExitUnvarnishSend(void);
void getModuleMes(void);
void request2Server(char str[]);
void communicateWithHost(char buf[], char *cmd, char *ccid, char net,
		char *module, char *ver, char *longitude, char *latitude, char *num);
void catRequestStr(char buf[], char *cmd);
void request(char *cmd);
void common_heart(void);

void powerOn4G(void);
void response2reset(char *cmd);
void statu_heart(char *cmds);
void dd_tc(u8 port, char *cmds);
void sys_tc(u8 port, char *cmds);
void currentPortStatuChanged(u8 port);

char *Int2Strs(u8 num);

void checkLinkedStatus(void);

#endif /* USART_4G_H_ */
