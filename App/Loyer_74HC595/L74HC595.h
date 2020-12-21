/*
 * L74HC595.h
 *
 *  Created on: 2020Äê6ÔÂ28ÈÕ
 *      Author: loyer
 */

#ifndef LOYER_74HC595_L74HC595_H_
#define LOYER_74HC595_L74HC595_H_

#include "bit_band.h"
#include "stdio.h"
#include "systick.h"

#define _HC595_CLK    PA_out(1)
#define _HC595_LATCH  PC_out(3)
#define _HC595_DATA      PA_out(4)

extern struct HC595Fram
{
	u8 allowSigleContrl;
	u8 LAST_LED_STATU;
	u8 slowBLINK[6];
	u8 slowCnt[6];
	u8 fastBLINK[6];
	u8 fastCnt[6];
} HC595_STATUS;

void Init_74HC595(void);
void HC595_Send_Byte(u8 byte);
void ledON(u8 led);
void ledOFF(u8 led);
void ledBLINK(u8 led);

#endif /* LOYER_74HC595_L74HC595_H_ */
