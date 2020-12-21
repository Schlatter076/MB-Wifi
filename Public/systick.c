///*
// * systick.c
// *
// *  Created on: 2019��8��13��
// *      Author: hw076
// */
//#include "systick.h"
//
//static uint8_t fac_us = 0;							//us��ʱ������
//static uint16_t fac_ms = 0;							//ms��ʱ������
//
///**
// * ��ʼ���ӳٺ���
// * SYSTICK��ʱ�ӹ̶�ΪAHBʱ�ӵ�1/8
// * SYSCLK:ϵͳʱ��Ƶ��
// */
//void SysTick_Init(uint8_t SYSCLK)
//{
//	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
//	fac_us = SYSCLK / 8;
//	fac_ms = (uint16_t) fac_us * 1000;
//}
//
///*******************************************************************************
// * �� �� ��         : delay_us
// * ��������		   : us��ʱ��
// * ��    ��         : nus��Ҫ��ʱ��us��
// * ע��:nus��ֵ,��Ҫ����798915us(���ֵ��2^24/fac_us@fac_us=21)
// * ��    ��         : ��
// *******************************************************************************/
//void delay_us(uint32_t nus)
//{
//	u32 temp;
//	SysTick->LOAD = 9 * nus;
//	SysTick->VAL = 0X00;							//��ռ�����
//	SysTick->CTRL = 0X01;							//ʹ�ܣ����������޶����������ⲿʱ��Դ
//	do
//	{
//		temp = SysTick->CTRL;							//��ȡ��ǰ������ֵ
//	} while ((temp & 0x01) && (!(temp & (1 << 16))));					//�ȴ�ʱ�䵽��
//	SysTick->CTRL = 0x00; //�رռ�����
//	SysTick->VAL = 0X00; //��ռ�����
//
//}
//
///*******************************************************************************
// * �� �� ��         : delay_ms
// * ��������	   : ms��ʱ��
// * ��    ��         : nms��Ҫ��ʱ��ms��
// * ע��:nms��ֵ,SysTick->LOADΪ24λ�Ĵ�����
// * ��Ҫ����0xffffff*8*1000/SYSCLK
// * ��72M������,nms<=1864ms
// * ��    ��         : ��
// *******************************************************************************/
//void delay_ms(uint16_t nms)
//{
////	while (nms--)
////	{
////		delay_us(1000);
////	}
//	u32 temp;
//	SysTick->LOAD = 9000 * nms;
//	SysTick->VAL = 0X00; //��ռ�����
//	SysTick->CTRL = 0X01; //ʹ�ܣ����������޶����������ⲿʱ��Դ
//	do
//	{
//		temp = SysTick->CTRL; //��ȡ��ǰ������ֵ
//	} while ((temp & 0x01) && (!(temp & (1 << 16)))); //�ȴ�ʱ�䵽��
//	SysTick->CTRL = 0x00; //�رռ�����
//	SysTick->VAL = 0X00; //��ռ�����
//	//��ռ�����
//}

/*
 * systick.c
 *
 *  Created on: 2019��8��13��
 *      Author: hw076
 */
#include "systick.h"

static uint8_t fac_us = 0;							//us��ʱ������
static uint16_t fac_ms = 0;							//ms��ʱ������

/**
 * ��ʼ���ӳٺ���
 * SYSTICK��ʱ�ӹ̶�ΪAHBʱ�ӵ�1/8
 * SYSCLK:ϵͳʱ��Ƶ��
 */
void SysTick_Init(uint8_t SYSCLK)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	fac_us = SYSCLK / 8;
	fac_ms = (uint16_t) fac_us * 1000;
}

/*******************************************************************************
 * �� �� ��         : delay_us
 * ��������		   : us��ʱ��
 * ��    ��         : nus��Ҫ��ʱ��us��
 * ע��:nus��ֵ,��Ҫ����798915us(���ֵ��2^24/fac_us@fac_us=21)
 * ��    ��         : ��
 *******************************************************************************/
void delay_us(uint32_t nus)
{
	uint32_t temp;
	SysTick->LOAD = nus * fac_us; 					//ʱ�����
	SysTick->VAL = 0x00;        					//��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	//��ʼ����
	do
	{
		temp = SysTick->CTRL;
	} while ((temp & 0x01) && !(temp & (1 << 16)));		//�ȴ�ʱ�䵽��
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
	SysTick->VAL = 0X00;      					 //��ռ�����
}

/*******************************************************************************
 * �� �� ��         : delay_ms
 * ��������	   : ms��ʱ��
 * ��    ��         : nms��Ҫ��ʱ��ms��
 * ע��:nms��ֵ,SysTick->LOADΪ24λ�Ĵ�����
 * ��Ҫ����0xffffff*8*1000/SYSCLK
 * ��72M������,nms<=1864ms
 * ��    ��         : ��
 *******************************************************************************/
void delay_ms(uint16_t nms)
{
	uint32_t temp;
	SysTick->LOAD = (uint32_t) nms * fac_ms;		//ʱ�����(SysTick->LOADΪ24bit)
	SysTick->VAL = 0x00;							//��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	//��ʼ����
	do
	{
		temp = SysTick->CTRL;
	} while ((temp & 0x01) && !(temp & (1 << 16)));		//�ȴ�ʱ�䵽��
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
	SysTick->VAL = 0X00;       					//��ռ�����
}
