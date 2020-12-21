/*
 * charge.h
 *
 *  Created on: 2020Äê6ÔÂ17ÈÕ
 *      Author: loyer
 */

#ifndef POWERBANK_CHARGE_H_
#define POWERBANK_CHARGE_H_

#include "stm32f10x.h"
#include "bit_band.h"


#define BAT1    PC_out(4)
#define BAT2    PC_out(5)
#define BAT3    PB_out(0)
#define BAT4    PB_out(1)
#define BAT5    PB_out(2)
#define BAT6    PB_out(12)

void Init_BAT_Charge(void);
void controlPowerBankCharge(u8 pb, u8 allow);
#endif /* POWERBANK_CHARGE_H_ */
