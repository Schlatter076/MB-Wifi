 /*
 * wifi.c
 *
 *  Created on: 2020年10月26日
 *      Author: loyer
 */
#include "wifi.h"

#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

volatile u16 UART4_Read_len = 0;
;

void UART4_Init(u32 bound)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	USART_DeInit(UART4);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PC10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	//PC11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_Init(GPIOC, &GPIO_InitStructure);	//初始化GPIOC 11

	//Uart4 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	//UART4 初始化设置
	USART_InitStructure.USART_BaudRate = bound;	//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;	//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl =
	USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(UART4, &USART_InitStructure); //初始化串口4

	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE); //开启串口接受和总线空闲中断
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);
	USART_ITConfig(UART4, USART_IT_ORE, ENABLE); //开启串口溢出中断

	USART_Cmd(UART4, ENABLE);                    //使能串口4
}

void UART4_IRQHandler(void)
{
	u8 ucCh;

	if (USART_GetITStatus( UART4, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(UART4, USART_IT_RXNE);
		ucCh = USART_ReceiveData( UART4);
		if (WIFI_Fram.InfBit.Length < ( TCP_MAX_LEN - 1))
		{
			//预留1个字节写结束符
			WIFI_Fram.Data[WIFI_Fram.InfBit.Length++] = ucCh;
		}
		else
		{
			printf("wifi cmd size over.\r\n");
			memset(WIFI_Fram.Data, 0, TCP_MAX_LEN);
			WIFI_Fram.InfAll = 0;
		}
	}
	if (USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)  //数据帧接收完毕
	{
		UART4->SR; //先读SR，再读DR
		UART4->DR;

		if (strstr((const char *) WIFI_Fram.Data, "CLOSED")) //连接断掉
		{
			printf("*******CLOSED*****\r\n");
			WIFI_Fram.linkedClosed = 1;
		}
		char *ptr = strstr((const char *) WIFI_Fram.Data, "+IPD"); //是否接收到服务器传来的数据
		int len = 0;
		if (ptr != NULL)
		{
			while (*ptr != ',')
			{
				ptr++;
			}
			ptr++;
			len = atoi(ptr);
			while (*ptr != ':')
			{
				ptr++;
			}
			ptr++;
			memcpy(WIFI_Fram.DeData, ptr, len);
			WIFI_Fram.InfBit.FinishFlag = 1;
			UART4_Read_len = 0;
		}
	}
	if (USART_GetITStatus(UART4, USART_FLAG_ORE) != RESET)
	{
		USART_ClearFlag(UART4, USART_FLAG_ORE);
		USART_ReceiveData(UART4);
	}
}
void WIFI_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(
	WIFI_CH_PD_Pin_Periph_Clock | WIFI_RST_Pin_Periph_Clock, ENABLE);

	GPIO_InitStructure.GPIO_Pin = WIFI_CH_PD_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO口速度为50MHz
	GPIO_Init(WIFI_CH_PD_Pin_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = WIFI_RST_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO口速度为50MHz
	GPIO_Init(WIFI_RST_Pin_Port, &GPIO_InitStructure);

	UART4_Init(bound);
	WIFI_CH_PD_Pin_SetH; //使能
	do
	{
		WIFI_RST_Pin_SetL;
		delay_ms(1000);
		delay_ms(1000);
		WIFI_RST_Pin_SetH;
		delay_ms(100);
	} while (!AT_Test(InWifi));
}
//选择wifi模块的工作模式
// enumMode：工作模式
//返回1：选择成功 0：选择失败
bool WIFI_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode)
{
	switch (enumMode)
	{
	case STA:
		return Send_AT_Cmd(InWifi, "AT+CWMODE_CUR=1", "OK", "no change", 1800);

	case AP:
		return Send_AT_Cmd(InWifi, "AT+CWMODE_CUR=2", "OK", "no change", 1800);

	case STA_AP:
		return Send_AT_Cmd(InWifi, "AT+CWMODE_CUR", "OK", "no change", 1800);

	default:
		return false;
	}
}

//wifi模块连接外部WiFi
//pSSID：WiFi名称字符串
//pPassWord：WiFi密码字符串
//返回1：连接成功 0：连接失败
bool WIFI_JoinAP(char * pSSID, char * pPassWord)
{
	char cCmd[120];

	sprintf(cCmd, "AT+CWJAP_CUR=\"%s\",\"%s\"", pSSID, pPassWord);

	return Send_AT_Cmd(InWifi, cCmd, "OK", NULL, 1800);

}

//wifi模块启动多连接
//enumEnUnvarnishTx：配置是否多连接
//返回1：配置成功 0：配置失败
bool WIFI_Enable_MultipleId(FunctionalState enumEnUnvarnishTx)
{
	char cStr[20];

	sprintf(cStr, "AT+CIPMUX=%d", (enumEnUnvarnishTx ? 1 : 0));

	return Send_AT_Cmd(InWifi, cStr, "OK", 0, 500);
}

//wifi模块连接外部服务器
//enumE：网络协议
//ip：服务器IP字符串
//ComNum：服务器端口字符串
//id：模块连接服务器的ID
//返回1：连接成功 0：连接失败
bool WIFI_Link_Server(ENUM_NetPro_TypeDef enumE, char * ip, int ComNum,
		ENUM_ID_NO_TypeDef id)
{
	char *cStr = mymalloc(100);
	char *cCmd = mymalloc(120);
	bool rc = false;

	switch (enumE)
	{
	case enumTCP:
		sprintf(cStr, "\"%s\",\"%s\",%d", "TCP", ip, ComNum);
		break;

	case enumUDP:
		sprintf(cStr, "\"%s\",\"%s\",%d", "UDP", ip, ComNum);
		break;

	default:
		break;
	}

	if (id < 5)
		sprintf(cCmd, "AT+CIPSTART=%d,%s", id, cStr);

	else
		sprintf(cCmd, "AT+CIPSTART=%s", cStr);

	rc = Send_AT_Cmd(InWifi, cCmd, "OK", "ALREAY CONNECT", 1800);
	myfree(cStr);
	myfree(cCmd);
	return rc;
}

//配置wifi模块进入透传发送
//返回1：配置成功 0：配置失败
bool WIFI_UnvarnishSend(void)
{
	if (!Send_AT_Cmd(InWifi, "AT+CIPMODE=1", "OK", 0, 500))
		return false;

	return Send_AT_Cmd(InWifi, "AT+CIPSEND", "OK", ">", 500);

}

//wifi模块发送字符串
//enumEnUnvarnishTx：声明是否已使能了透传模式
//pStr：要发送的字符串
//ulStrLength：要发送的字符串的字节数
//ucId：哪个ID发送的字符串
//返回1：发送成功 0：发送失败
bool WIFI_SendString(FunctionalState enumEnUnvarnishTx, char * pStr,
		u32 ulStrLength, ENUM_ID_NO_TypeDef ucId)
{
	char cStr[20];
	bool bRet = false;

	if (enumEnUnvarnishTx)
	{
		WIFI_USART("%s", pStr);

		bRet = true;

	}

	else
	{
		if (ucId < 5)
			sprintf(cStr, "AT+CIPSENDEX=%d,%d", ucId,
					(unsigned int) ulStrLength + 2);

		else
			sprintf(cStr, "AT+CIPSENDEX=%d", (unsigned int) ulStrLength + 2);

		Send_AT_Cmd(InWifi, cStr, "> ", 0, 1000);

		bRet = Send_AT_Cmd(InWifi, pStr, "SEND OK", 0, 1000);
	}

	return bRet;

}

//wifi模块退出透传模式
void WIFI_ExitUnvarnishSend(void)
{
	delay_ms(1000);
	WIFI_USART("+++");
	delay_ms(500);
}

//wifi 的连接状态，较适合单端口时使用
//返回0：获取状态失败
//返回2：获得ip
//返回3：建立连接
//返回4：失去连接
u8 WIFI_Get_LinkStatus(void)
{
	if (Send_AT_Cmd(InWifi, "AT+CIPSTATUS", "OK", 0, 500))
	{
		if (strstr((const char *) WIFI_Fram.Data, "STATUS:2\r\n"))
			return 2;

		else if (strstr((const char *) WIFI_Fram.Data, "STATUS:3\r\n"))
			return 3;

		else if (strstr((const char *) WIFI_Fram.Data, "STATUS:4\r\n"))
			return 4;

	}
	return 0;
}

bool ConnectToServerByWIFI(char* addr, int port)
{
	WIFI_Net_Mode_Choose(STA);
	Send_AT_Cmd(InWifi, "AT+GMR", "OK", NULL, 500);
	while (!WIFI_JoinAP("ChinaNet-mPRv", "7qfe3pqt"))
		;
	WIFI_Enable_MultipleId(DISABLE);
	while (!WIFI_Link_Server(enumTCP, addr, port, Single_ID_0))
		;
	return true;
}
