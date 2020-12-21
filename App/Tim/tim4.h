/*
 * tim4.h
 *
 *  Created on: 2020Äê7ÔÂ20ÈÕ
 *      Author: loyer
 */

#ifndef TIM_TIM4_H_
#define TIM_TIM4_H_

#include "bat_usart.h"
#include "charge.h"
#include "usart_4G.h"
#include "L74HC595.h"
#include "L74hc165.h"
#include "motor.h"
#include "string.h"
#include "led.h"


volatile u8 allowTCSamePort;
//volatile u8 modifyPortSta;
volatile u8 curPort;
volatile u8 allowModuleUpdate;


void TIM4_Init(u16 per,u16 psc);
#endif /* TIM_TIM4_H_ */
