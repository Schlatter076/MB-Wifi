/*
 * usart.h
 *
 *  Created on: 2019��10��19��
 *      Author: Loyer
 */

#ifndef USART_H_
#define USART_H_

#include "bit_band.h"
#include "STMFlash.h"
#include "tcp_public.h"

void USART1_Init(u32 bound);
u8 hexStr2Byte(char *hexStr);

#endif /* USART_H_ */
