/*
 * L74HC595.c
 *
 *  Created on: 2020年6月28日
 *      Author: loyer
 */
#include "L74HC595.h"

struct HC595Fram HC595_STATUS =
{ 0 };

void Init_74HC595(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; //定义结构体变量
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(GPIOA, &GPIO_InitStructure); /* 初始化GPIO */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(GPIOC, &GPIO_InitStructure); /* 初始化GPIO */

	_HC595_CLK = 0;
	_HC595_LATCH = 0;
	_HC595_DATA = 0;
	//灯板开始单独控制某一个灯
	HC595_STATUS.allowSigleContrl = 1;
}
/***
 *74HC595 发送一个字节
 *即往74HC595的SI引脚发送一个字节
 */
void HC595_Send_Byte(u8 byte)
{
	u8 i;
	for (i = 0; i < 8; i++)  //一个字节8位，传输8次，一次一位，循环8次，刚好移完8位
	{
		/****  步骤1：将数据传到DS引脚    ****/
		if (byte & 0x80)        //先传输高位，通过与运算判断第八是否为1
			_HC595_DATA = 1;    //如果第八位是1，则与 595 DS连接的引脚输出高电平
		else
			//否则输出低电平
			_HC595_DATA = 0;

		/*** 步骤2：SHCP每产生一个上升沿，当前的bit就被送入移位寄存器 ***/
		_HC595_CLK = 0;   // SHCP拉低
		delay_us(1);           // 适当延时
		_HC595_CLK = 1;  // SHCP拉高， SHCP产生上升沿
		delay_us(1);
		byte <<= 1;		// 左移一位，将低位往高位移，通过	if (byte & 0x80)判断低位是否为1
	}
	//输出锁存使能
	_HC595_LATCH = 0;
	delay_us(1);
	_HC595_LATCH = 1;
	delay_us(1);
}
/**
 * 点亮某一个led灯
 * @led 1-6表示灯1-6
 */
void ledON(u8 led)
{
	if (HC595_STATUS.allowSigleContrl)
	{
		HC595_STATUS.LAST_LED_STATU &= ~(1 << (7 - led)); //对某位清零
		HC595_Send_Byte(HC595_STATUS.LAST_LED_STATU);
	}
}

void ledOFF(u8 led)
{
	if (HC595_STATUS.allowSigleContrl)
	{
		HC595_STATUS.LAST_LED_STATU |= (1 << (7 - led)); //对某位置位
		HC595_Send_Byte(HC595_STATUS.LAST_LED_STATU);
	}
}

void ledBLINK(u8 led)
{
	if (HC595_STATUS.allowSigleContrl)
	{
		HC595_STATUS.LAST_LED_STATU ^= (1 << (7 - led)); //对某位置位
		HC595_Send_Byte(HC595_STATUS.LAST_LED_STATU);
	}
}

