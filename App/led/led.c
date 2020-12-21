/*
 * led.c
 *
 *  Created on: 2020年7月3日
 *      Author: loyer
 */
#include "led.h"

void Init_led(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; //定义结构体变量

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(GPIOA, &GPIO_InitStructure); /* 初始化GPIO */

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//关闭JTAG，保留SWD，释放PB3 PB4 PA15
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	GPIO_ResetBits(GPIOA, GPIO_Pin_15);
}
