/*
 * audio.h
 *
 *  Created on: 2020Äê6ÔÂ20ÈÕ
 *      Author: loyer
 */

#ifndef AUDIO_AUDIO_H_
#define AUDIO_AUDIO_H_

#include "stm32f10x.h"
#include "bit_band.h"
#include "systick.h"

#define TRIG1_H  (PA_out(5)=1)
#define TRIG1_L  (PA_out(5)=0)
#define TRIG2_H  (PA_out(6)=1)
#define TRIG2_L  (PA_out(6)=0)
#define TRIG3_H  (PA_out(7)=1)
#define TRIG3_L  (PA_out(7)=0)


void Init_Audio(void);
void play_audio(u8 aud);

#endif /* AUDIO_AUDIO_H_ */
