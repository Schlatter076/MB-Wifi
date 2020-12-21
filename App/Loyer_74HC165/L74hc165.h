/*
 * L74hc165.h
 *
 *  Created on: 2020Äê6ÔÂ17ÈÕ
 *      Author: loyer
 */

#ifndef LOYER_74HC165_L74HC165_H_
#define LOYER_74HC165_L74HC165_H_

#include "bit_band.h"
#include "stdio.h"
#include "systick.h"

#define _HC165_PL  PA_out(11)
#define _HC165_CP  PA_out(8)

extern volatile u16 key_statu;

void Init_74HC165(void);
u16 read_74HC165(void);


#endif /* LOYER_74HC165_L74HC165_H_ */
