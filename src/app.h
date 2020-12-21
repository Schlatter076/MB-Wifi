/*
 * app.h
 *
 *  Created on: 2020Äê10ÔÂ26ÈÕ
 *      Author: loyer
 */

#ifndef APP_H_
#define APP_H_

#include "stm32f10x.h"
#include "STMFlash.h"
#include "stdio.h"

typedef  void (*pFunction)(void);
extern pFunction Jump_To_Application;
extern uint32_t JumpAddress;

#define ApplicationAddress    0x8010000


void RunApp(void);

#endif /* APP_H_ */
