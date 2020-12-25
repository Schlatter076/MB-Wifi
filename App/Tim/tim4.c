/*
 * tim4.c
 *
 *  Created on: 2020��7��20��
 *      Author: loyer
 */
#include "tim4.h"

volatile u8 initCnt = 0;
u8 hc595Statu = 0xFF;
volatile u32 heartCnt = 0;
volatile u8 statuHeartCnt = 0;
volatile u16 firstStatuHeartCnt = 0;
//volatile u8 checkCnt = 0;
volatile u8 allowTCSamePort = 0;
volatile u16 allowTCSamePortCnt = 0;
//volatile u8 modifyPortSta = 0;
volatile u8 curPort = 0xFF;
volatile u8 allowModuleUpdate = 0;
u16 moduleUpCnt = 0;

volatile u8 checkCnt = 0;
//wifi����
volatile u8 wifi_initCnt = 0;
volatile u32 wifi_heartCnt = 0;
volatile u8 wifi_statuHeartCnt = 0;
volatile u16 wifi_firstStatuHeartCnt = 0;
/*******************************************************************************
 * �� �� ��         : TIM4_Init
 * ��������		   : TIM4��ʼ������
 * ��    ��         : per:��װ��ֵ
 psc:��Ƶϵ��
 * ��    ��         : ��
 *******************************************************************************/
void TIM4_Init(u16 per, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	TIM_DeInit(TIM4);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //ʹ��TIM4ʱ��

	TIM_TimeBaseInitStructure.TIM_Period = per;   //�Զ�װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc; //��Ƶϵ��
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //�������ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE); //������ʱ���ж�
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn; //��ʱ���ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM4, ENABLE); //ʹ�ܶ�ʱ��
}
/**
 * ����ʱ����ʼ��
 */
void Task_Init(u16 per, u16 psc)
{

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	TIM_DeInit(TIM5);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	//TIM5����5ms��ʱ
	TIM_TimeBaseInitStructure.TIM_Period = per;   //�Զ�װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc; //��Ƶϵ��
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //�������ϼ���ģʽ
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStructure);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn; //��ʱ���ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);
	//�������������ݴ�������
	TIM_Cmd(TIM5, ENABLE);
}
/*******************************************************************************
 * �� �� ��         : TIM4_IRQHandler
 * ��������		   : TIM4�жϺ���
 * ��    ��         : ��
 * ��    ��         : ��
 *******************************************************************************/
void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

		//��һ���ϱ���״̬ʧ��
		if (F4G_Fram.firstStatuHeartNotSucc)
		{
			firstStatuHeartCnt++;
			if (firstStatuHeartCnt == 2000)
			{
				firstStatuHeartCnt = 0;
				F4G_Fram.forceHeart_32 = 1;
			}
		}
		//�ϵ�ע��ɹ�
		if (F4G_Fram.registerSuccess)
		{
			F4G_Fram.registerSuccess = 0;

			F4G_Fram.allowHeart = 1;

			LED_Status = 1;
			//�յ���һ���ϱ���״̬
			F4G_Fram.init = 0; //��ʼ��ȫ�����
			//��һ���ϱ���籦������Ϣ
			F4G_Fram.forceHeart_32 = 1;
			F4G_Fram.firstStatuHeartNotSucc = 1;
		}
		//4Gģ���ϵ�ע����
		if (F4G_Fram.init && WIFI_Fram.allowHeart == 0)
		{
			initCnt++;
			if (initCnt == 50)
			{
				initCnt = 0;
				LED_Status ^= 1;
//				hc595Statu ^= 0xFF;
//				HC595_Send_Byte(hc595Statu);
			}
		}
		//�ϵ�ɹ������ɹ���ȡ��������Ϣ
		if (F4G_Fram.allowHeart)
		{
			heartCnt++;
			if (heartCnt == RegisterParams.heartTime * 100)
			{
				heartCnt = 0;
				commonHeart(USART2);
				statuHeartCnt++;
				if (statuHeartCnt == RegisterParams.statuHeartTime)
				{
					statuHeartCnt = 0;
					F4G_Fram.forceHeart_90 = 1;
				}
			}
		}
		//wifi
		//��һ���ϱ���״̬ʧ��
		if (WIFI_Fram.firstStatuHeartNotSucc)
		{
			wifi_firstStatuHeartCnt++;
			if (wifi_firstStatuHeartCnt == 2000)
			{
				wifi_firstStatuHeartCnt = 0;
				WIFI_Fram.forceHeart_32 = 1;
			}
		}
		//�ϵ�ע��ɹ�
		if (WIFI_Fram.registerSuccess)
		{
			WIFI_Fram.registerSuccess = 0;

			WIFI_Fram.allowHeart = 1;

			LED_Status = 1;
			//�յ���һ���ϱ���״̬
			WIFI_Fram.init = 0; //��ʼ��ȫ�����
			//��һ���ϱ���籦������Ϣ
			WIFI_Fram.forceHeart_32 = 1;
			WIFI_Fram.firstStatuHeartNotSucc = 1;
		}
		if (WIFI_Fram.init && F4G_Fram.allowHeart == 0)
		{
			wifi_initCnt++;
			if (wifi_initCnt == 50)
			{
				wifi_initCnt = 0;
				LED_Status ^= 1;
				//				hc595Statu ^= 0xFF;
				//				HC595_Send_Byte(hc595Statu);
			}
		}
		if (WIFI_Fram.allowHeart)
		{
			wifi_heartCnt++;
			if (wifi_heartCnt == RegisterParams.heartTime * 100)
			{
				wifi_heartCnt = 0;
				commonHeart(UART4);
				wifi_statuHeartCnt++;
				if (wifi_statuHeartCnt == RegisterParams.statuHeartTime)
				{
					wifi_statuHeartCnt = 0;
					WIFI_Fram.forceHeart_90 = 1;
				}
			}
		}
		if (allowTCSamePort)
		{
			allowTCSamePortCnt++;
			if (allowTCSamePortCnt == 300)
			{
				printf("allow TC.\r\n");
				allowTCSamePortCnt = 0;
				allowTCSamePort = 0;
				curPort = 0xFF;
			}
		}
		//����
		if (HC595_STATUS.fastBLINK[0])
		{
			HC595_STATUS.fastCnt[0]++;
			if (HC595_STATUS.fastCnt[0] == 20)
			{
				HC595_STATUS.fastCnt[0] = 0;
				ledBLINK(1);
			}
		}
		if (HC595_STATUS.fastBLINK[1])
		{
			HC595_STATUS.fastCnt[1]++;
			if (HC595_STATUS.fastCnt[1] == 20)
			{
				HC595_STATUS.fastCnt[1] = 0;
				ledBLINK(2);
			}
		}
		if (HC595_STATUS.fastBLINK[2])
		{
			HC595_STATUS.fastCnt[2]++;
			if (HC595_STATUS.fastCnt[2] == 20)
			{
				HC595_STATUS.fastCnt[2] = 0;
				ledBLINK(3);
			}
		}
		if (HC595_STATUS.fastBLINK[3])
		{
			HC595_STATUS.fastCnt[3]++;
			if (HC595_STATUS.fastCnt[3] == 20)
			{
				HC595_STATUS.fastCnt[3] = 0;
				ledBLINK(4);
			}
		}
		if (HC595_STATUS.fastBLINK[4])
		{
			HC595_STATUS.fastCnt[4]++;
			if (HC595_STATUS.fastCnt[4] == 20)
			{
				HC595_STATUS.fastCnt[4] = 0;
				ledBLINK(5);
			}
		}
		if (HC595_STATUS.fastBLINK[5])
		{
			HC595_STATUS.fastCnt[5]++;
			if (HC595_STATUS.fastCnt[5] == 20)
			{
				HC595_STATUS.fastCnt[5] = 0;
				ledBLINK(6);
			}
		}
		//����
		if (HC595_STATUS.slowBLINK[0])
		{
			HC595_STATUS.slowCnt[0]++;
			if (HC595_STATUS.slowCnt[0] == 80)
			{
				HC595_STATUS.slowCnt[0] = 0;
				ledBLINK(1);
			}
		}
		if (HC595_STATUS.slowBLINK[1])
		{
			HC595_STATUS.slowCnt[1]++;
			if (HC595_STATUS.slowCnt[1] == 80)
			{
				HC595_STATUS.slowCnt[1] = 0;
				ledBLINK(2);
			}
		}
		if (HC595_STATUS.slowBLINK[2])
		{
			HC595_STATUS.slowCnt[2]++;
			if (HC595_STATUS.slowCnt[2] == 80)
			{
				HC595_STATUS.slowCnt[2] = 0;
				ledBLINK(3);
			}
		}
		if (HC595_STATUS.slowBLINK[3])
		{
			HC595_STATUS.slowCnt[3]++;
			if (HC595_STATUS.slowCnt[3] == 80)
			{
				HC595_STATUS.slowCnt[3] = 0;
				ledBLINK(4);
			}
		}
		if (HC595_STATUS.slowBLINK[4])
		{
			HC595_STATUS.slowCnt[4]++;
			if (HC595_STATUS.slowCnt[4] == 80)
			{
				HC595_STATUS.slowCnt[4] = 0;
				ledBLINK(5);
			}
		}
		if (HC595_STATUS.slowBLINK[5])
		{
			HC595_STATUS.slowCnt[5]++;
			if (HC595_STATUS.slowCnt[5] == 80)
			{
				HC595_STATUS.slowCnt[5] = 0;
				ledBLINK(6);
			}
		}
		checkCnt++;
		if (checkCnt > 2)
		{
			TCP_Params.checkPBst = 1;
		}
	}
}
