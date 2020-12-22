/*
 * task.c
 *
 *  Created on: 2020年12月22日
 *      Author: HHS007
 */
#include "tim4.h"

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

		TIM_Cmd(TIM2, DISABLE);
		module4GPowerOn();
	}
}

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

	}
}

void TIM5_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);

		TIM_Cmd(TIM5, DISABLE);
		//状态心跳
		if (F4G_Fram.forceHeart_32 == 1)
		{
			F4G_Fram.forceHeart_32 = 0;
			forceHeart(In4G, &TCP_Params, UP_AllPortsSTA);
		}
		if (F4G_Fram.forceHeart_90 == 1)
		{
			F4G_Fram.forceHeart_90 = 0;
			forceHeart(In4G, &TCP_Params, UP_StatuHeart);
		}
		if (TCP_Params.process4G == 1)
		{
			TCP_Params.process4G = 0;
			ProcessServerCmd(In4G, &F4G_Fram, &TCP_Params);
		}
		if (TCP_Params.processWIFI == 1)
		{
			TCP_Params.processWIFI = 0;
			ProcessServerCmd(InWifi, &F4G_Fram, &TCP_Params);
		}
		TIM_Cmd(TIM5, ENABLE);
	}
}
