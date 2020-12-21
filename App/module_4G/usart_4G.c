/*
 * usart_4G.c
 *
 *  Created on: 2020年6月11日
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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	//PA3  RXD
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//初始化GPIOA3

	//Usart2 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	//USART2 初始化设置
	USART_InitStructure.USART_BaudRate = bound;	//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;	//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl =
	USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART2, &USART_InitStructure); //初始化串口2

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); //开启串口接受和总线空闲中断
	USART_ITConfig(USART2, USART_IT_ORE, ENABLE); //开启串口溢出中断

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
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO口速度为50MHz

	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//开机
	do
	{
		PKEY_4G_Pin_SetH;
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
		//delay_ms(1000);
		PKEY_4G_Pin_SetL;
		//复位4G模块
		RST_4G_Pin_SetH;
		delay_ms(1100);
		RST_4G_Pin_SetL;
		delay_ms(500);

		F4G_ExitUnvarnishSend();
		Send_AT_Cmd(In4G, "AT+CIPCLOSE", "OK", NULL, 500);
		Send_AT_Cmd(In4G, "AT+RSTSET", "OK", NULL, 500);
	} while (!AT_Test(In4G));

	//获取4G模块参数信息
	getModuleMes();
	//连接到服务器
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
	//上电注册
	while (1)
	{
		if (request_cnt >= 3)
		{
			NVIC_SystemReset(); //重启
			//RunApp();
		}
		request(REQ_REGISTER);
		request_cnt++;
		delay_ms(500);
		if (F4G_Fram.InfBit.FinishFlag)
		{
			F4G_Fram.InfBit.FinishFlag = 0;
			split((char *)F4G_Fram.DeData, ",");
			//注册成功
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
		//收到服务器端发回的数据
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
 * 将str通过delims进行分割,所得的字符串填充在res中
 * @str 待转换的数据字符串
 * @delims 分隔符
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
 * 通过4G网络连接到服务器
 * @addr IP地址或域名
 * @port 端口
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
			; //单链接模式
		while (!Send_AT_Cmd(In4G, "AT+CIPQSEND=1", "OK", NULL, 500))
			; //快传模式
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
		//1.设置模式为TCP透传模式
		Send_AT_Cmd(In4G, "AT+CIPMODE=1", "OK", NULL, 500);

	} while (!Send_AT_Cmd(In4G, p, "CONNECT OK", "ALREADY CONNECT", 1800));
	myfree(p);
	return 1;
}
/**
 * 透传模式下4G模块发送字符串
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
 * 4G模块退出透传模式
 */
void F4G_ExitUnvarnishSend(void)
{
	delay_ms(1000);
	F4G_USART("+++");
	delay_ms(500);
}
/***********************以下开始为与服务器通信业务代码部分*************************************/
void getModuleMes(void)
{
	unsigned char *result = NULL;
	u8 inx = 0;
	strcpy(TCP_Params.locations[0], "0");
	strcpy(TCP_Params.locations[1], "0");
	//获取物联网卡号
	while (!Send_AT_Cmd(In4G, "AT+ICCID", "+ICCID:", NULL, 500))
		;
	result = F4G_Fram.Data;
	inx = 0;
	while (!(*result <= '9' && *result >= '0'))
	{
		result++;
	}
	//当值为字母和数字时
	while ((*result <= '9' && *result >= '0')
			|| (*result <= 'Z' && *result >= 'A')
			|| (*result <= 'z' && *result >= 'a'))
	{
		TCP_Params.ccid[inx++] = *result;
		result++;
	}
	printf("CCID=%s\r\n", TCP_Params.ccid);

	//获取模块网络信息
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
	//获取信号
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
 * 组装与上位机通讯时的字符串
 * @buf 填充的字符串
 * @cmd 控制字
 * @ccid 物联网卡号
 * @net 当前使用网络
 * @module 当前使用的模块
 * @ver 当前固件版本号
 * @attr 当前经纬度
 * @num  几口机型
 * @return 无
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
 * 向服务器发起请求
 * @cmd 请求的控制字
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
 * 普通心跳
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
 * 状态心跳
 */
void statu_heart(char *cmds)
{
//拼接字符串
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
 * 订单弹出
 *
 */
void dd_tc(u8 port, char *cmds)
{
//拼接字符串
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
 * 系统弹出
 */
void sys_tc(u8 port, char *cmds)
{
//拼接字符串
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
//拼接字符串
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
 * 向服务器发起请求
 */
void request2Server(char str[])
{
	printf("%s\r\n", str);
	base64_encode((const unsigned char *) str, (char *)F4G_Fram.EnData);
	F4G_sendStr((char *)F4G_Fram.EnData);
}

