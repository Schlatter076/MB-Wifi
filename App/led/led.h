/*
 * led.h
 *
 *  Created on: 2020Äê7ÔÂ3ÈÕ
 *      Author: loyer
 */

#ifndef LED_LED_H_
#define LED_LED_H_

#include "bit_band.h"

#define LED_Status  PA_out(15)
#define BRIGHT  1
#define DARK    0

void Init_led(void);

#endif /* LED_LED_H_ */
