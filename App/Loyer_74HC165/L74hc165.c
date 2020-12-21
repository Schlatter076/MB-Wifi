/*
 * L74hc165.c
 *
 *  Created on: 2020年6月17日
 *      Author: loyer
 */
#include "L74hc165.h"

volatile u16 key_statu = 0xFF;

void Init_74HC165(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; //定义结构体变量
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(GPIOA, &GPIO_InitStructure); /* 初始化GPIO */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(GPIOA, &GPIO_InitStructure); /* 初始化GPIO */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	 //设置数据口上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure); /* 初始化GPIO */

//	_HC165_PL = 1;  //不读取状态
//	_HC165_CP = 1;  //时钟拉高，便于产生上升沿
	read_74HC165();

}
u16 read_74HC165(void)
{
	u8 i;
	u16 temp = 0;
	static u16 old_statu = 0xffff;
	_HC165_PL = 1;
	delay_us(1);
	_HC165_PL = 0; //PL拉低，读取按键状态
	delay_us(1);
	_HC165_PL = 1; //PL拉高，停止按键读取
	delay_us(1);
	_HC165_CP = 1;
	delay_us(1);
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) == 0) //低电平表示按键导通
		temp += 1;
	for (i = 0; i < 15; i++) //因为最高位不用移位即可读取，故循环(16-1)次，依次把次高位移到最高位，第一级74HC165优先读取
	{
		_HC165_CP = 0;  //为低读取数据
		delay_us(1);
		_HC165_CP = 1;
		delay_us(1);
		temp = temp << 1;
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) == 0)
			temp += 1;

	}
	_HC165_PL = 1;
	_HC165_CP = 1;
	temp &= 0x3f3f;
	if(old_statu != temp)
	{
		printf("key_statu=%04X\r\n", temp);
		old_statu = temp;
	}
	return temp;
}
