/*
 * bat_usart.h
 *
 *  Created on: 2020年6月17日
 *      Author: loyer
 */

#ifndef POWERBANK_BAT_USART_H_
#define POWERBANK_BAT_USART_H_

#include "bit_band.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "charge.h"
#include "L74hc165.h"
#include "string.h"
#include "systick.h"
#include "L74HC595.h"
#include "usart_4G.h"

#define TX3_RX3_Exchange  PC_out(8)=!PC_out(8)
#define _A1_H    GPIO_SetBits(GPIOC, GPIO_Pin_6)
#define _A1_L    GPIO_ResetBits(GPIOC, GPIO_Pin_6)
#define _B1_H    GPIO_SetBits(GPIOC, GPIO_Pin_7)
#define _B1_L    GPIO_ResetBits(GPIOC, GPIO_Pin_7)
#define _A2_H    GPIO_SetBits(GPIOB, GPIO_Pin_14)
#define _A2_L    GPIO_ResetBits(GPIOB, GPIO_Pin_14)
#define _B2_H    GPIO_SetBits(GPIOB, GPIO_Pin_15)
#define _B2_L    GPIO_ResetBits(GPIOB, GPIO_Pin_15)

#define UART3_RX_BUF_SIZE 20 //UART3接收数据空间域大小
#define UART3_TX_BUF_SIZE 16  //下行数据装配缓冲区

typedef struct
{
    uint8_t flag;
    uint8_t counter;
    uint8_t len;
    uint8_t data[UART3_RX_BUF_SIZE];
} Uart3MagType;

#define	ComTest	       0xFF
#define SetID_SUCCESS  0xFE
//下行数据标识符定义
#define CMD_Header     0xA0
#define CMD_End        0xA1
#define	CMD_ReadID	   0x01
#define	CMD_SetID	   0x11
#define CMD_ReadVOL    0x02
#define CMD_ReadCUR    0x03
#define CMD_ReadERROR  0x0A
//上行数据标识符定义
#define UP_Header     0xB0
#define UP_End        0xB1
#define UP_ReadID     0x81
#define UP_ReadVOL    0x82
#define UP_ReadCUR    0x83
#define UP_ReadERROR  0x8A

#define CRC_BUF_LEN  16
typedef struct
{
    uint8_t len;
    uint8_t cmd;
    uint8_t CRC_BUF[CRC_BUF_LEN];
} FramType;

extern FramType framMag;

/*---------全局变量定义-------------------------------------------------------------------*/
extern Uart3MagType UART3_RX_BUF;     //中断接收完成标识
extern u8 UART3_TX_BUF[UART3_TX_BUF_SIZE];

#define ChargeBankIDSize        0x08

typedef struct
{
	u8 VOL;
	u8 TEST;
	u8 ERROR;
	u16 CUR;
	unsigned char Charging[6];
	unsigned char ChargingCnt;
	u8 ChargeBankID[ChargeBankIDSize];
	char powerBankBuf[6][18];
} PowerBankType;
extern PowerBankType powerbankStatu;

void USART3_Init(u32 bound);
void Init_Bat_Usart(u32 bound);
void communicateWithPort(u8 port);
uint8_t BSP_UART_SendOneByte(uint8_t data);
unsigned int get_crc(unsigned char pbuf[], unsigned char num);
unsigned int get_crc_2(int num, ...);
void BSP_UART_SendMultipleBytes(unsigned char buf[], unsigned char len);
void app_frame_anasys(void);
void app_cmd_anasys(void);
void controlPowerbank(u8 cmd);
u8 communicationTest(u8 p);
u8 checkPowerbankStatus(u8 i, char buf[]);
void setBATInstruction(u8 port, u8 sta);
void scanPowerBank(void);

#endif /* POWERBANK_BAT_USART_H_ */
