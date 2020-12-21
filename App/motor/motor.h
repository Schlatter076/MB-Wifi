/*
 * motor.h
 *
 *  Created on: 2020Äê6ÔÂ17ÈÕ
 *      Author: loyer
 */

#ifndef MOTOR_MOTOR_H_
#define MOTOR_MOTOR_H_

#include "bit_band.h"
#include "L74hc165.h"
#include "usart.h"
#include "audio.h"

#define FORWARD  1
#define BACKWARD 0

extern volatile u8 motor_running;

void Init_Motor(void);
void motor_reset(void);
void motor_run(u8 mot, u8 dir);
void motor_stop(u8 mot);
u8 IsMotorArravalsOrigin(u8 mot);
void popUP_powerBank(u8 powerBank, u8 play);
void popUP_All(void);

#endif /* MOTOR_MOTOR_H_ */
