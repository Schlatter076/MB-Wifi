/*
 * charge.c
 *
 *  Created on: 2020年6月17日
 *      Author: loyer
 */
#include "charge.h"

void Init_BAT_Charge(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; //定义结构体变量

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2
			| GPIO_Pin_12;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(GPIOB, &GPIO_InitStructure); /* 初始化GPIO */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(GPIOC, &GPIO_InitStructure); /* 初始化GPIO */

	BAT1 = 1;
	BAT2 = 1;
	BAT3 = 1;
	BAT4 = 1;
	BAT5 = 1;
	BAT6 = 1;
}
/**
 * 控制充电宝充电函数
 * @pb 1-6表示充电宝1-6
 * @allow 1-表示允许  0-表示不允许
 */
void controlPowerBankCharge(u8 pb, u8 allow)
{
	switch (pb)
	{
	case 1:
		BAT1 = allow;
		break;
	case 2:
		BAT2 = allow;
		break;
	case 3:
		BAT3 = allow;
		break;
	case 4:
		BAT4 = allow;
		break;
	case 5:
		BAT5 = allow;
		break;
	case 6:
		BAT6 = allow;
		break;
	default:
		break;
	}
}

