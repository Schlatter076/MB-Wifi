/*
 * usart_4G.c
 *
 *  Created on: 2020��6��11��
 *      Author: loyer
 */
#include "usart_4G.h"

void USART2_Init(uint32_t bound)
{
	USART_DeInit(USART2);

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2  TXD
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	//PA3  RXD
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//��ʼ��GPIOA3

	//Usart2 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	//USART2 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;	//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;	//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl =
	USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); //��ʼ������2

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); //�������ڽ��ܺ����߿����ж�
	USART_ITConfig(USART2, USART_IT_ORE, ENABLE); //������������ж�

	USART_Cmd(USART2, ENABLE);
}

void F4G_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	F4G_Fram.init = 1;
	F4G_Fram.allowProcessServerData = 0;

	USART2_Init(bound);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = PKEY_4G | RST_4G;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO���ٶ�Ϊ50MHz

	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//����
	do
	{
		PKEY_4G_Pin_SetH;
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
		//delay_ms(1000);
		PKEY_4G_Pin_SetL;
		//��λ4Gģ��
		RST_4G_Pin_SetH;
		delay_ms(1100);
		RST_4G_Pin_SetL;
		delay_ms(500);

		F4G_ExitUnvarnishSend();
		Send_AT_Cmd(In4G, "AT+CIPCLOSE", "OK", NULL, 500);
		Send_AT_Cmd(In4G, "AT+RSTSET", "OK", NULL, 500);
	} while (!AT_Test(In4G));

	//��ȡ4Gģ�������Ϣ
	getModuleMes();
	//���ӵ�������
	if (RegisterParams.ip != NULL && RegisterParams.port != NULL)
	{
		while (!ConnectToServerBy4G(RegisterParams.ip, RegisterParams.port))
			;
	}
	else
	{
		while (!ConnectToServerBy4G(TCPServer_IP, TCPServer_PORT))
			;
	}
	u8 request_cnt = 0;
	//�ϵ�ע��
	while (1)
	{
		if (request_cnt >= 3)
		{
			NVIC_SystemReset(); //����
			//RunApp();
		}
		request(REQ_REGISTER);
		request_cnt++;
		delay_ms(500);
		if (F4G_Fram.InfBit.FinishFlag)
		{
			F4G_Fram.InfBit.FinishFlag = 0;
			split((char *)F4G_Fram.DeData, ",");
			//ע��ɹ�
			if (memcmp(F4G_Fram.Server_Command[1],
			RES_REGISTER, 2) == 0)
			{
				char *tem = (char *)F4G_Fram.ServerData;
				tem = strtok((char *)F4G_Fram.ServerData, "_");
				RegisterParams.heartTime = atoi(tem);
				tem = strtok(NULL, "_");
				RegisterParams.statuHeartTime = atoi(tem);
				printf("ht=%d,sht=%d\r\n", RegisterParams.heartTime,
						RegisterParams.statuHeartTime);
				break;
			}
		}
	}
	F4G_Fram.registerSuccess = 1;
	RegisterParams.allowHeart = 1;
	F4G_Fram.allowProcessServerData = 1;
}

void USART2_IRQHandler(void)
{
	u8 ucCh;
	if (USART_GetITStatus( USART2, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		ucCh = USART_ReceiveData( USART2);

		if (F4G_Fram.InfBit.Length < (TCP_MAX_LEN - 1))
		{
			F4G_Fram.Data[F4G_Fram.InfBit.Length++] = ucCh;
		}
		else
		{
			printf("4G Cmd size over.\r\n");
			memset(F4G_Fram.Data, 0, TCP_MAX_LEN);
			F4G_Fram.InfAll = 0;
		}
		//�յ��������˷��ص�����
		if (ucCh == ']' && (bool) strchr((const char *)F4G_Fram.Data, '['))
		{
			char *res = (char *)F4G_Fram.Data;
			F4G_Fram.serverStatuCnt = 0;
			while (*res != '[')
			{
				res++;
			}
			while (*res == '[')
				res++;

			F4G_Fram.base64Str = strtok(res, "]");
//			printf("before Decryption=%s\r\n",
//					F4G_Fram_Record_Struct.base64Str);
			base64_decode((const char *) F4G_Fram.base64Str,
					(unsigned char *) F4G_Fram.DeData);
			printf("after Decryption=%s\r\n", F4G_Fram.DeData);
			//split(F4G_Fram_Record_Struct.DeData, ",");
			memset(F4G_Fram.Data, '\0', TCP_MAX_LEN);
			F4G_Fram.InfBit.Length = 0;
			F4G_Fram.InfBit.FinishFlag = 1;
		}
		//=====================================================================
		if (F4G_Fram.allowProcessServerData)
		{
			if (F4G_Fram.InfBit.FinishFlag)
			{
				F4G_Fram.InfBit.FinishFlag = 0;
				split((char *)F4G_Fram.DeData, ",");
				TCP_Params.process4G = 1;
			}
		}
	}
	if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
	{
		USART_ClearFlag(USART2, USART_FLAG_ORE);
		USART_ReceiveData(USART2);
	}
}
/**
 * ��strͨ��delims���зָ�,���õ��ַ��������res��
 * @str ��ת���������ַ���
 * @delims �ָ���
 */
void split(char str[], char *delims)
{
	char *result = str;
	u8 inx = 0;
	while (inx < 2)
	{
		result++;
		if (*result == ',')
		{
			++inx;
		}
	}
	result++;
	memcpy(F4G_Fram.ServerData, result, BASE64_BUF_LEN);
//printf("comd2=%s\r\n", F4G_Fram_Record_Struct.ServerData);
	result = strtok(str, delims);
	F4G_Fram.Server_Command[0] = (unsigned char *)result;
	result = strtok( NULL, delims);
	F4G_Fram.Server_Command[1] = (unsigned char *)result;
}

/**
 * ͨ��4G�������ӵ�������
 * @addr IP��ַ������
 * @port �˿�
 * @return
 */
bool ConnectToServerBy4G(char* addr, char* port)
{
	char *p = mymalloc(100);
	sprintf(p, "AT+CIPSTART=\"TCP\",\"%s\",%s", addr, port);
	do
	{
		while (!Send_AT_Cmd(In4G, "AT+CIPSHUT", "SHUT OK", NULL, 500))
			;
		while (!Send_AT_Cmd(In4G, "AT+CREG?", "OK", NULL, 500))
			;
		while (!Send_AT_Cmd(In4G, "AT+CGATT?", "OK", NULL, 500))
			;
		while (!Send_AT_Cmd(In4G, "AT+CIPMUX=0", "OK", NULL, 500))
			; //������ģʽ
		while (!Send_AT_Cmd(In4G, "AT+CIPQSEND=1", "OK", NULL, 500))
			; //�촫ģʽ
		if (TCP_Params.cops == '3')
		{
			while (!Send_AT_Cmd(In4G, "AT+CSTT=cmiot", "OK",
			NULL, 1800))
				;
		}
		else if (TCP_Params.cops == '6')
		{
			while (!Send_AT_Cmd(In4G, "AT+CSTT=UNIM2M.NJM2MAPN", "OK", NULL,
					1800))
				;
		}
		else if (TCP_Params.cops == '9')
		{
			while (!Send_AT_Cmd(In4G, "AT+CSTT=CTNET", "OK", NULL, 1800))
				;
		}
		while (!Send_AT_Cmd(In4G, "AT+CIICR", "OK", NULL, 500))
			;
		Send_AT_Cmd(In4G, "AT+CIFSR", "OK", NULL, 500);
		//1.����ģʽΪTCP͸��ģʽ
		Send_AT_Cmd(In4G, "AT+CIPMODE=1", "OK", NULL, 500);

	} while (!Send_AT_Cmd(In4G, p, "CONNECT OK", "ALREADY CONNECT", 1800));
	myfree(p);
	return 1;
}
/**
 * ͸��ģʽ��4Gģ�鷢���ַ���
 */
void F4G_sendStr(char *str)
{
	u8 i = 0;
	char *cmd = mymalloc(200);
	snprintf(cmd, 200, "{(%s}", str);
	while (cmd[i] != '}')
	{
		USART_SendData(USART2, cmd[i]);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
			;
		i++;
	}
	USART_SendData(USART2, cmd[i]);
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
		;
	myfree(cmd);
}
/**
 * 4Gģ���˳�͸��ģʽ
 */
void F4G_ExitUnvarnishSend(void)
{
	delay_ms(1000);
	F4G_USART("+++");
	delay_ms(500);
}
/***********************���¿�ʼΪ�������ͨ��ҵ����벿��*************************************/
void getModuleMes(void)
{
	unsigned char *result = NULL;
	u8 inx = 0;
	strcpy(TCP_Params.locations[0], "0");
	strcpy(TCP_Params.locations[1], "0");
	//��ȡ����������
	while (!Send_AT_Cmd(In4G, "AT+ICCID", "+ICCID:", NULL, 500))
		;
	result = F4G_Fram.Data;
	inx = 0;
	while (!(*result <= '9' && *result >= '0'))
	{
		result++;
	}
	//��ֵΪ��ĸ������ʱ
	while ((*result <= '9' && *result >= '0')
			|| (*result <= 'Z' && *result >= 'A')
			|| (*result <= 'z' && *result >= 'a'))
	{
		TCP_Params.ccid[inx++] = *result;
		result++;
	}
	printf("CCID=%s\r\n", TCP_Params.ccid);

	//��ȡģ��������Ϣ
	while (!Send_AT_Cmd(In4G, "AT+COPS=0,1", "OK", NULL, 1000))
		;
	while (!Send_AT_Cmd(In4G, "AT+COPS?", "+COPS", NULL, 500))
		;
	if ((bool) strstr((const char *) F4G_Fram.Data, "CMCC"))
	{
		TCP_Params.cops = '3';
	}
	else if ((bool) strstr((const char *) F4G_Fram.Data, "UNICOM"))
	{
		TCP_Params.cops = '6';
	}
	else
	{
		TCP_Params.cops = '9';
	}
	printf("COPS is \"%c\"\r\n", TCP_Params.cops);
	//��ȡ�ź�
	while (!Send_AT_Cmd(In4G, "AT+CSQ", "OK", NULL, 500))
		;
	result = F4G_Fram.Data;
	while (*result++ != ':')
		;
	result++;
	TCP_Params.rssi = atoi(strtok((char *) result, ","));
	printf("CSQ is %d\r\n", TCP_Params.rssi);
}
/**
 * ��װ����λ��ͨѶʱ���ַ���
 * @buf �����ַ���
 * @cmd ������
 * @ccid ����������
 * @net ��ǰʹ������
 * @module ��ǰʹ�õ�ģ��
 * @ver ��ǰ�̼��汾��
 * @attr ��ǰ��γ��
 * @num  ���ڻ���
 * @return ��
 */
void communicateWithHost(char buf[], char *cmd, char *ccid, char net,
		char *module, char *ver, char *longitude, char *latitude, char *num)
{
	const char* template = "%s,%s,%s-%c-%s-%s-%s_%s-%s";
	ReadDeviceID();
	sprintf(buf, template, RDeviceID, cmd, ccid, net, module, ver, longitude,
			latitude, num);
}

void catRequestStr(char buf[], char *cmd)
{
	ReadVersion();
	communicateWithHost(buf, cmd, (char *)TCP_Params.ccid,
			TCP_Params.cops, "2", RVersion,
			TCP_Params.locations[0],
			TCP_Params.locations[1], "06");
}
/**
 * ���������������
 * @cmd ����Ŀ�����
 */
void request(char *cmd)
{
	char rBuf1[128] =
	{ 0 };
	catRequestStr(rBuf1, cmd);
	request2Server(rBuf1);
}

void response2reset(char *cmd)
{
	char rBuf2[128] =
	{ 0 };
	ReadDeviceID();
	sprintf(rBuf2, "%s,%s", RDeviceID, cmd);
	request2Server(rBuf2);
}
/**
 * ��ͨ����
 */
void common_heart(void)
{
	const char *heart = "{(}";
//	printf("%s\r\n", heart);
	F4G_Fram.serverStatuCnt += 1;
	while (*heart != '}')
	{
		USART_SendData(USART2, *heart);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
			;
		heart++;
	}
	USART_SendData(USART2, *heart);
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
		;
}
/**
 * ״̬����
 */
void statu_heart(char *cmds)
{
//ƴ���ַ���
	char reqBuf[256] =
	{ 0 };
	const char* template = "%s,%s,%s-%s-%s-%s-%s-%s-%s-%s";
//	F4G_Params.serverStatuCnt += 1;
	ReadDeviceID();
	sprintf(reqBuf, template, RDeviceID, cmds, powerbankStatu.powerBankBuf[0],
			powerbankStatu.powerBankBuf[1], powerbankStatu.powerBankBuf[2],
			powerbankStatu.powerBankBuf[3], powerbankStatu.powerBankBuf[4],
			powerbankStatu.powerBankBuf[5],
			Int2Strs(TCP_Params.rssi), "aaaaa");
	request2Server(reqBuf);
}
/**
 * ��������
 *
 */
void dd_tc(u8 port, char *cmds)
{
//ƴ���ַ���
	char reqBuf2[256] =
	{ 0 };
	const char* template = "%s,%s,%s-%s-%s";
	TCP_Params.statuCode[port] = checkPowerbankStatus(port,
			powerbankStatu.powerBankBuf[port]);
	ReadDeviceID();
	sprintf(reqBuf2, template, RDeviceID, cmds,
			powerbankStatu.powerBankBuf[port], TCP_Params.dd, "aaaaa");
	request2Server(reqBuf2);
}
/**
 * ϵͳ����
 */
void sys_tc(u8 port, char *cmds)
{
//ƴ���ַ���
	char reqBuf3[256] =
	{ 0 };
	const char* template = "%s,%s,%s-%s";
	TCP_Params.statuCode[port] = checkPowerbankStatus(port,
			powerbankStatu.powerBankBuf[port]);
	ReadDeviceID();
	sprintf(reqBuf3, template, RDeviceID, cmds,
			powerbankStatu.powerBankBuf[port], "aaaaa");
	request2Server(reqBuf3);
}

void currentPortStatuChanged(u8 port)
{
//ƴ���ַ���
	char reqBuf4[256] =
	{ 0 };
	const char* template = "%s,%s,%s-%s";
	ReadDeviceID();
	sprintf(reqBuf4, template, RDeviceID, "37",
			powerbankStatu.powerBankBuf[port], "aaaaa");
	request2Server(reqBuf4);
}

char *Int2Strs(u8 num)
{
	static char tem[3] =
	{ 0 };
	snprintf(tem, 3, "%d", num);
	return tem;
}

/**
 * ���������������
 */
void request2Server(char str[])
{
	printf("%s\r\n", str);
	base64_encode((const unsigned char *) str, (char *)F4G_Fram.EnData);
	F4G_sendStr((char *)F4G_Fram.EnData);
}

