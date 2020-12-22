 /*
 * wifi.c
 *
 *  Created on: 2020��10��26��
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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	//PC11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//��������
	GPIO_Init(GPIOC, &GPIO_InitStructure);	//��ʼ��GPIOC 11

	//Uart4 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	//UART4 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;	//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;	//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl =
	USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(UART4, &USART_InitStructure); //��ʼ������4

	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE); //�������ڽ��ܺ����߿����ж�
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);
	USART_ITConfig(UART4, USART_IT_ORE, ENABLE); //������������ж�

	USART_Cmd(UART4, ENABLE);                    //ʹ�ܴ���4
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
			//Ԥ��1���ֽ�д������
			WIFI_Fram.Data[WIFI_Fram.InfBit.Length++] = ucCh;
		}
		else
		{
			printf("wifi cmd size over.\r\n");
			memset(WIFI_Fram.Data, 0, TCP_MAX_LEN);
			WIFI_Fram.InfAll = 0;
		}
	}
	if (USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)  //����֡�������
	{
		UART4->SR; //�ȶ�SR���ٶ�DR
		UART4->DR;

		if (strstr((const char *) WIFI_Fram.Data, "CLOSED")) //���Ӷϵ�
		{
			printf("*******CLOSED*****\r\n");
			WIFI_Fram.linkedClosed = 1;
		}
		char *ptr = strstr((const char *) WIFI_Fram.Data, "+IPD"); //�Ƿ���յ�����������������
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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO���ٶ�Ϊ50MHz
	GPIO_Init(WIFI_CH_PD_Pin_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = WIFI_RST_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO���ٶ�Ϊ50MHz
	GPIO_Init(WIFI_RST_Pin_Port, &GPIO_InitStructure);

	UART4_Init(bound);
	WIFI_CH_PD_Pin_SetH; //ʹ��
	do
	{
		WIFI_RST_Pin_SetL;
		delay_ms(1000);
		delay_ms(1000);
		WIFI_RST_Pin_SetH;
		delay_ms(100);
	} while (!AT_Test(InWifi));
}
//ѡ��wifiģ��Ĺ���ģʽ
// enumMode������ģʽ
//����1��ѡ��ɹ� 0��ѡ��ʧ��
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

//wifiģ�������ⲿWiFi
//pSSID��WiFi�����ַ���
//pPassWord��WiFi�����ַ���
//����1�����ӳɹ� 0������ʧ��
bool WIFI_JoinAP(char * pSSID, char * pPassWord)
{
	char cCmd[120];

	sprintf(cCmd, "AT+CWJAP_CUR=\"%s\",\"%s\"", pSSID, pPassWord);

	return Send_AT_Cmd(InWifi, cCmd, "OK", NULL, 1800);

}

//wifiģ������������
//enumEnUnvarnishTx�������Ƿ������
//����1�����óɹ� 0������ʧ��
bool WIFI_Enable_MultipleId(FunctionalState enumEnUnvarnishTx)
{
	char cStr[20];

	sprintf(cStr, "AT+CIPMUX=%d", (enumEnUnvarnishTx ? 1 : 0));

	return Send_AT_Cmd(InWifi, cStr, "OK", 0, 500);
}

//wifiģ�������ⲿ������
//enumE������Э��
//ip��������IP�ַ���
//ComNum���������˿��ַ���
//id��ģ�����ӷ�������ID
//����1�����ӳɹ� 0������ʧ��
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

//����wifiģ�����͸������
//����1�����óɹ� 0������ʧ��
bool WIFI_UnvarnishSend(void)
{
	if (!Send_AT_Cmd(InWifi, "AT+CIPMODE=1", "OK", 0, 500))
		return false;

	return Send_AT_Cmd(InWifi, "AT+CIPSEND", "OK", ">", 500);

}

//wifiģ�鷢���ַ���
//enumEnUnvarnishTx�������Ƿ���ʹ����͸��ģʽ
//pStr��Ҫ���͵��ַ���
//ulStrLength��Ҫ���͵��ַ������ֽ���
//ucId���ĸ�ID���͵��ַ���
//����1�����ͳɹ� 0������ʧ��
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

//wifiģ���˳�͸��ģʽ
void WIFI_ExitUnvarnishSend(void)
{
	delay_ms(1000);
	WIFI_USART("+++");
	delay_ms(500);
}

//wifi ������״̬�����ʺϵ��˿�ʱʹ��
//����0����ȡ״̬ʧ��
//����2�����ip
//����3����������
//����4��ʧȥ����
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
