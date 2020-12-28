/*
 * tcp_process.c
 *
 *  Created on: 2020��11��11��
 *      Author: loyer
 */
#include "tcp_process.h"

ParamsOfWifiJoinAP_TypeDef ParamsOfWifiJoinAPInit =
{ 0 };

const char heart[] = "{(}";

/**
 * ͸��ģʽ�·����ַ���
 * @USARTx ����
 * @str �����͵��ַ���
 */
void TCP_sendStr(USART_TypeDef* USARTx, char *str)
{
	u8 i = 0;
	char *cmd = mymalloc(200);
	char *base64Str = mymalloc(200);
	if (USARTx == USART2)
	{
		printf("<<%s\r\n", str);
	}
	else
	{
		printf("Wifi<<%s\r\n", str);
	}
	base64_encode((const unsigned char *) str, base64Str);
	snprintf(cmd, 200, "{(%s}", base64Str);
	while (cmd[i] != '}')
	{
		USART_SendData(USARTx, cmd[i]);
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
			;
		i++;
	}
	USART_SendData(USARTx, cmd[i]);
	while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
		;
	myfree(base64Str);
	myfree(cmd);
}

/**
 * ��ȡwifi��������
 * @datas UTF-8������ַ������������Ⱥ�����
 * 		      �磺"19_9_�ڻ�ʯw1c. 28��_hhs888888"
 * @Powj_Init ����ssid��pwd�Ľṹ��
 * �޷���
 */
void getWifiSsidAndPwd(char *datas, ParamsOfWifiJoinAP_TypeDef *Powj_Init)
{
	char *res = datas;
	u8 Inx = 0;
	char buf[4] =
	{ '\0', '\0', '\0', '\0' };
	u16 ssidLEN = 0;
	u16 pwdLEN = 0;
	while (*res != '_' && Inx < 3)
	{
		buf[Inx++] = *res++;
	}
	res++;
	ssidLEN = atoi(buf);
	printf("ssid_len=%d,", ssidLEN);
	Inx = 0;
	memset(buf, '\0', 4);
	while (*res != '_' && Inx < 3)
	{
		buf[Inx++] = *res++;
	}
	res++;
	pwdLEN = atoi(buf);
	printf("pwd_len=%d,", pwdLEN);
	memset(Powj_Init->ssid, '\0', 100);
	memset(Powj_Init->pwd, '\0', 100);
	Inx = 0;
	while (ssidLEN--)
	{
		Powj_Init->ssid[Inx++] = *res++;
	}
	res++;
	Inx = 0;
	while (pwdLEN--)
	{
		Powj_Init->pwd[Inx++] = *res++;
	}
	printf("SSID=%s,PWD=%s\r\n", Powj_Init->ssid, Powj_Init->pwd);
}

/**
 * ��ȡע���ַ���
 * @strBuf ����ȡ���ַ���
 * @len �ַ�������
 * @upCMD ���п�����
 * @params ͨ��ģ���һЩ����
 * @modulType ��ʹ�õĵ�ģ������
 * @version �̼��汾��
 * @num ���ڻ���
 * �޷���
 */
void getRegisterStr(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD,
		struct STRUCT_USART_Params *params, char moduleType, char *version,
		char *num)
{
	const char* template = "%s,%d,%s-%c-%c-%s-%s_%s-%s";
	ReadDeviceID();
	snprintf(strBuf, len, template, RDeviceID, upCMD, params->ccid,
			params->cops, moduleType, version, params->locations[0],
			params->locations[1], num);
}
/**
 * ��ȡ���������������ַ���
 * @strBuf
 * @len
 * @upCMD ���п�����
 * �޷���
 */
void getRequestStrWithoutParam(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD)
{
	const char* template = "%s,%d";
	ReadDeviceID();
	snprintf(strBuf, len, template, RDeviceID, upCMD);
}
/**
 * ��ȡ��籦״̬�ַ���
 * @strBuf
 * @len
 * @upCMD
 * @rssi �ź�(��ʹ��4Gģ��ʱ)
 * @portNum ������
 * @portSTAStr ״̬�ַ���(�ɱ��)
 * �޷���
 */
void getPowerbankSTAStr(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD,
		u8 rssi, int portNum, char *portSTAStr, ...)
{
	va_list ap;
	va_start(ap, portSTAStr);
	ReadDeviceID();
	snprintf(strBuf, len, "%s,%d,%s", RDeviceID, upCMD, portSTAStr);
	for (int i = 1; i < portNum; i++)
	{
		char *tem = va_arg(ap, char *);
		strcat(strBuf, "-");
		strcat(strBuf, tem);
	}
	char *temBuf = mymalloc(3);
	snprintf(temBuf, 3, "%d", rssi);
	strcat(strBuf, "-");
	strcat(strBuf, temBuf);
	myfree(temBuf);
	strcat(strBuf, "-aaaaa");
	va_end(ap);
}
/**
 * ��ȡ��籦״̬�ַ���(�����ź�)
 * @strBuf
 * @len
 * @upCMD
 * @portNum ������
 * @portSTAStr ״̬�ַ���(�ɱ��)
 * �޷���
 */
void getPowerbankSTAStrWithoutRSSI(char *strBuf, int len,
		ENUM_tcpUP_TypeDef upCMD, int portNum, char *portSTAStr, ...)
{
	va_list ap;
	va_start(ap, portSTAStr);
	ReadDeviceID();
	snprintf(strBuf, len, "%s,%d,%s", RDeviceID, upCMD, portSTAStr);
	for (int i = 1; i < portNum; i++)
	{
		char *tem = va_arg(ap, char *);
		strcat(strBuf, "-");
		strcat(strBuf, tem);
	}
	strcat(strBuf, "-aaaaa");
	va_end(ap);
}
/**
 * ��������ָ��
 * @intenet 4G �� Wifi
 * @fram ����֡�ṹ��
 * @params �����ṹ��
 * �޷���
 */
void ProcessServerCmd(ENUM_Internet_TypeDef internet,
		struct STRUCT_USART_Fram *fram, struct STRUCT_USART_Params *params)
{
	switch (atoi((const char *) fram->Server_Command[1]))
	{
	case DOWN_RegiseterSucc:
		F4G_Fram.allowHeart = 1;
		getRegisterParams(fram);
		break;
	case DOWN_RecivedAllPortsSTA:
		fram->serverStatuCnt = 0;
		fram->firstStatuHeartNotSucc = 0;
		break;
	case DOWN_RecivedStatuHeart:
		fram->serverStatuCnt = 0;
		break;
	case DOWN_SystemPopupPowerbank:
		systemPopup(internet, fram, params);
		break;
	case DOWN_RecivedForceHeart:
		F4G_Fram.serverStatuCnt = 0;
		break;
	case DOWN_RecivedSystemPopupSTA:

		break;
	case DOWN_RecivedPowerbankSTAChanged:

		break;
	case DOWN_VoiceBroadcast:
		if (atoi((const char *) fram->ServerData) == 1)
		{
			play_audio(2); //���ɹ�
		}
		else if (atoi((const char *) fram->ServerData) == 2)
		{
			play_audio(3); //�黹�ɹ�
		}
		break;
	case DOWN_ForceHeart:
		forceHeart(internet, params, UP_ForceHeart);
		break;
	case DOWN_DeviceReset:
		responseReset();
		break;
	case DOWN_OrderPopupPowerbank:
		orderPopup(internet, fram, params);
		break;
	case DOWN_RecivedOrderPopupPowerbank:

		break;
	case DOWN_PopupAllPowerbanks:
		popUP_All();
		break;
	case DOWN_IgnoreLock:
		modifyLockSTA(fram);
		break;
	case DOWN_SetWifiSsidAndPwd:
		setWifiSsidAndPwd(internet, fram);
		break;
	case DOWN_SetID:
		WDeviceID = (char *) fram->ServerData;
		WriteDeviceID();
		NVIC_SystemReset();
		break;
	default:
		printf("cmd\"%s\" is not support\r\n", fram->Server_Command[1]);
		break;
	}
}
/**
 * ϵͳ������籦
 * @intenet 4G �� Wifi
 * @fram ����֡�ṹ��
 * @params �����ṹ��
 * �޷���
 */
void systemPopup(ENUM_Internet_TypeDef internet, struct STRUCT_USART_Fram *fram,
		struct STRUCT_USART_Params *params)
{
	char *buf = mymalloc(100);
	char *tem = strtok((char *) fram->ServerData, "-");
	params->port = atoi(tem);
	tem = strtok(NULL, "-");
	params->play = (u8) atoi(tem);
	if (curPort != params->port)
	{
		curPort = params->port;
		popUP_powerBank(params->port + 1, params->play);
		params->statuCode[params->port] = checkPowerbankStatus(params->port,
				powerbankStatu.powerBankBuf[params->port]);
		getPowerbankSTAStrWithoutRSSI(buf, 100, UP_SystemPopupSTA, 1,
				powerbankStatu.powerBankBuf[params->port]);

		if (internet == In4G)
		{
			TCP_sendStr(USART2, buf);
		}
		else
		{
			TCP_sendStr(UART4, buf);
		}
		allowTCSamePort = 1;
	}

	myfree(buf);
}

/**
 * ����������籦
 * @intenet 4G �� Wifi
 * @fram ����֡�ṹ��
 * @params �����ṹ��
 * �޷���
 */
void orderPopup(ENUM_Internet_TypeDef internet, struct STRUCT_USART_Fram *fram,
		struct STRUCT_USART_Params *params)
{
	char *buf = mymalloc(100);
	char *tem = strtok((char *) fram->ServerData, "-");
	params->port = atoi(tem);
	tem = strtok(NULL, "-");
	strcpy(params->dd, tem);
	tem = strtok(NULL, "-");
	params->play = (u8) atoi(tem);
	if (curPort != params->port)
	{
		curPort = params->port;
		popUP_powerBank(params->port + 1, params->play);
		params->statuCode[params->port] = checkPowerbankStatus(params->port,
				powerbankStatu.powerBankBuf[params->port]);
		getPowerbankSTAStrWithoutRSSI(buf, 100, UP_OrderPopupPowerbank, 2,
				powerbankStatu.powerBankBuf[params->port], params->dd);

		if (internet == In4G)
		{
			TCP_sendStr(USART2, buf);
		}
		else
		{
			TCP_sendStr(UART4, buf);
		}
		allowTCSamePort = 1;
	}
	myfree(buf);
}
/**
 * �޸�����״̬
 * @fram ����֡�ṹ��
 * �޷���
 */
void modifyLockSTA(struct STRUCT_USART_Fram *fram)
{
	//�޸ĺ���ĳ�������쳣״̬
	//�ӿڲ���������_״̬��1��ʾд����0��ʾ������ȡ
	char *tem = strtok((char *) fram->ServerData, "-");
	u8 kk = atoi(tem);
	tem = strtok(NULL, "-");
	u8 ksta = atoi(tem);
	WriteIgnoreLock(kk, ksta);
}
/**
 * ��ͨ����
 */
void commonHeart(USART_TypeDef* USARTx)
{
	if (USARTx == USART2)
	{
		F4G_Fram.serverStatuCnt++;
	}
	else if (USARTx == UART4)
	{
		WIFI_Fram.serverStatuCnt++;
	}
	for (int i = 0; i < 3; i++)
	{
		USART_SendData(USARTx, heart[i]);
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
			;
	}
}
/**
 * ǿ������
 */
void forceHeart(ENUM_Internet_TypeDef internet,
		struct STRUCT_USART_Params *params, ENUM_tcpUP_TypeDef upCmd)
{
	char *buf = mymalloc(200);
	getPowerbankSTAStr(buf, 200, upCmd, params->rssi, 6,
			powerbankStatu.powerBankBuf[0], powerbankStatu.powerBankBuf[1],
			powerbankStatu.powerBankBuf[2], powerbankStatu.powerBankBuf[3],
			powerbankStatu.powerBankBuf[4], powerbankStatu.powerBankBuf[5]);

	if (internet == In4G)
	{
		TCP_sendStr(USART2, buf);
	}
	else
	{
		TCP_sendStr(UART4, buf);
	}
	myfree(buf);
}
/**
 * ��Ӧϵͳ����
 */
void responseReset(void)
{
	char *buf = mymalloc(20);
	getRequestStrWithoutParam(buf, 20, UP_DeviceRest);
	myfree(buf);
	NVIC_SystemReset();
}
/**
 * �ϱ���ǰ����״̬�仯
 */
void reportPortStatuChanged(u8 port, USART_TypeDef* USARTx)
{
	char *buf = mymalloc(50);
	getPowerbankSTAStrWithoutRSSI(buf, 50, UP_PowerbankSTAChanged, 1,
			powerbankStatu.powerBankBuf[port]);
	TCP_sendStr(USARTx, buf);
	myfree(buf);
}
/**
 * ����ע��
 */
void request4Register(USART_TypeDef* USARTx)
{
	char *buf = mymalloc(100);
	ReadVersion();
	getRegisterStr(buf, 100, UP_Regiser, &TCP_Params, '2', RVersion, "06");
	TCP_sendStr(USARTx, buf);
	myfree(buf);
}
/**
 * ����wifi��������
 */
void setWifiSsidAndPwd(ENUM_Internet_TypeDef internet,
		struct STRUCT_USART_Fram *fram)
{
	char *buf = mymalloc(100);
	getWifiSsidAndPwd((char *) fram->ServerData, &ParamsOfWifiJoinAPInit);
	getPowerbankSTAStrWithoutRSSI(buf, 100, UP_SetWifiSsidAndPwd, 1,
			(char *) fram->ServerData);

	if (internet == In4G)
	{
		TCP_sendStr(USART2, buf);
	}
	else
	{
		TCP_sendStr(UART4, buf);
	}
	myfree(buf);
	WriteWifiFlag();
	WriteWifiSsid();
	WriteWifiPwd();
	TCP_Params.wifiParamModified = 1;
}
/**
 * ��ȡע�����
 */
void getRegisterParams(struct STRUCT_USART_Fram *fram)
{
	char *tem = (char *) fram->ServerData;
	tem = strtok((char *) fram->ServerData, "_");
	RegisterParams.heartTime = atoi(tem);
	tem = strtok(NULL, "_");
	RegisterParams.statuHeartTime = atoi(tem);
	printf("ht=%d,sht=%d\r\n", RegisterParams.heartTime,
			RegisterParams.statuHeartTime);
	fram->registerSuccess = 1;
}
