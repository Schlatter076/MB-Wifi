/*
 * tcp_process.c
 *
 *  Created on: 2020年11月11日
 *      Author: loyer
 */
#include "tcp_process.h"

ParamsOfWifiJoinAP_TypeDef ParamsOfWifiJoinAPInit =
{ 0 };

const char heart[] = "{(}";

/**
 * 透传模式下发送字符串
 * @USARTx 串口
 * @str 待发送的字符串
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
 * 获取wifi名和密码
 * @datas UTF-8编码的字符串，包含长度和内容
 * 		      如："19_9_黑火石w1c. 28！_hhs888888"
 * @Powj_Init 包含ssid和pwd的结构体
 * 无返回
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
 * 获取注册字符串
 * @strBuf 待获取的字符串
 * @len 字符串长度
 * @upCMD 上行控制字
 * @params 通信模块的一些参数
 * @modulType 所使用的的模块类型
 * @version 固件版本号
 * @num 几口机型
 * 无返回
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
 * 获取不带参数的请求字符串
 * @strBuf
 * @len
 * @upCMD 上行控制字
 * 无返回
 */
void getRequestStrWithoutParam(char *strBuf, int len, ENUM_tcpUP_TypeDef upCMD)
{
	const char* template = "%s,%d";
	ReadDeviceID();
	snprintf(strBuf, len, template, RDeviceID, upCMD);
}
/**
 * 获取充电宝状态字符串
 * @strBuf
 * @len
 * @upCMD
 * @rssi 信号(当使用4G模块时)
 * @portNum 卡口数
 * @portSTAStr 状态字符串(可变参)
 * 无返回
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
 * 获取充电宝状态字符串(不带信号)
 * @strBuf
 * @len
 * @upCMD
 * @portNum 卡口数
 * @portSTAStr 状态字符串(可变参)
 * 无返回
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
 * 处理服务端指令
 * @intenet 4G 或 Wifi
 * @fram 数据帧结构体
 * @params 参数结构体
 * 无返回
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
			play_audio(2); //租借成功
		}
		else if (atoi((const char *) fram->ServerData) == 2)
		{
			play_audio(3); //归还成功
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
 * 系统弹出充电宝
 * @intenet 4G 或 Wifi
 * @fram 数据帧结构体
 * @params 参数结构体
 * 无返回
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
 * 订单弹出充电宝
 * @intenet 4G 或 Wifi
 * @fram 数据帧结构体
 * @params 参数结构体
 * 无返回
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
 * 修改锁的状态
 * @fram 数据帧结构体
 * 无返回
 */
void modifyLockSTA(struct STRUCT_USART_Fram *fram)
{
	//修改忽略某卡口锁异常状态
	//接口参数：卡口_状态（1表示写死，0表示正常读取
	char *tem = strtok((char *) fram->ServerData, "-");
	u8 kk = atoi(tem);
	tem = strtok(NULL, "-");
	u8 ksta = atoi(tem);
	WriteIgnoreLock(kk, ksta);
}
/**
 * 普通心跳
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
 * 强制心跳
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
 * 响应系统重启
 */
void responseReset(void)
{
	char *buf = mymalloc(20);
	getRequestStrWithoutParam(buf, 20, UP_DeviceRest);
	myfree(buf);
	NVIC_SystemReset();
}
/**
 * 上报当前卡口状态变化
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
 * 请求注册
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
 * 设置wifi名和密码
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
 * 获取注册参数
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
