/*
 * STMFlash.h
 *
 *  Created on: 2020年6月19日
 *      Author: loyer
 */

#ifndef FLASH_STMFLASH_H_
#define FLASH_STMFLASH_H_

#include "stm32f10x.h"
#include <string.h>
#include "tcp_process.h"

#define STM32_FLASH_SIZE 256 	 		//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 1              //使能FLASH写入(0，不使能;1，使能)

//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 	//STM32 BootFLASH的起始地址

//模拟EEPROM的起始地址
#define EEPROM_ADDR   0x0803F804

#define VERSION_ADDR  0x0803F820

#define APP_SERVER_ADDR  0x0803F840
//8字节设备编号  1字节是否写入编号  1字节是否在线更新程序

#define IGNORE_LOCK_ADDR  0x0803FF00
extern unsigned char IgnoreLock[6];

#define WIFI_FLAG_ADDR  0x0803FF10
#define WIFI_SSID_ADDR  0x0803FF12
#define WIFI_PWD_ADDR   0x0803FF80

__IO extern char *WDeviceID;
__IO extern char RDeviceID[9];

extern char *WVersion;
extern char RVersion[21];

extern char APPServer[101];

u16 STMFLASH_ReadHalfWord(u32 faddr);		  //读出半字
void STMFLASH_WriteLenByte(u32 WriteAddr, u32 DataToWrite, u16 Len);//指定地址开始写入指定长度的数据
u32 STMFLASH_ReadLenByte(u32 ReadAddr, u16 Len);				//指定地址开始读取指定长度数据
void STMFLASH_Write(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite);//从指定地址开始写入指定长度的数据
void STMFLASH_Read(u32 ReadAddr, u16 *pBuffer, u16 NumToRead); //从指定地址开始读出指定长度的数据

//测试写入
void Test_Write(u32 WriteAddr, u16 WriteData);
void WriteDeviceID(void);
void ReadDeviceID(void);
void writeVersion(void);
void ReadVersion(void);
void WriteAPPServer(char *server);
void ReadAPPServer(void);

void WriteIgnoreLock(u8 inx, u8 sta);
u8 ReadIgnoreLock(u8 inx);

void WriteWifiSsid(void);
void ReadWifiSsid(void);
void WriteWifiPwd(void);
void ReadWifiPwd(void);
void WriteWifiFlag(void);
u16 ReadWifiFlag(void);

#endif /* FLASH_STMFLASH_H_ */
