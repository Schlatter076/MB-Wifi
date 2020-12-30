/*
 * usart.h
 *
 *  Created on: 2019Äê10ÔÂ19ÈÕ
 *      Author: Loyer
 */

#ifndef USART_H_
#define USART_H_

#include "bit_band.h"
#include "STMFlash.h"
#include "tcp_public.h"
#include "tcp_process.h"

void USART1_Init(u32 bound);
void usart1_callback(void);
u8 hexStr2Byte(char *hexStr);

#endif /* USART_H_ */
