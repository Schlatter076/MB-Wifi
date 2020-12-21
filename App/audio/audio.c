/*
 * audio.c
 *
 *  Created on: 2020年6月20日
 *      Author: loyer
 */
#include "audio.h"

void Init_Audio(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; //定义结构体变量
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(GPIOA, &GPIO_InitStructure); /* 初始化GPIO */

	//初始化置低不触发
	GPIO_ResetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);

}
/**
 * 播放语音模块里的第几段
 * @aud 第几段语音
 */
void play_audio(u8 aud)
{
	u8 i;
	TRIG2_H;
	delay_us(100);
	TRIG2_L;
	delay_us(100);
	for(i = 0; i < aud; i++)
	{
		TRIG1_H;
		delay_us(100);
		TRIG1_L;
	}
}

