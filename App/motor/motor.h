/*
 * motor.h
 *
 *  Created on: 2020年6月17日
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

//切换电机方案用
#define MOTOR_CTR_1V2  1u

extern volatile u8 motor_running;

void Init_Motor(void);
void motor_run(u8 mot, u8 dir);
void motor_stop(u8 mot);
u8 IsMotorArravalsOrigin(u8 mot);
void popUP_powerBank(u8 powerBank, u8 play);
void popUP_All(void);
void remoteCtrMotot(u8 powerBank, u16 gtime, u16 ktime);

#endif /* MOTOR_MOTOR_H_ */
