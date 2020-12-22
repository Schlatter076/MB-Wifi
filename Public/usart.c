#include "usart.h"
#include "stdio.h"
#include "usart_4G.h"
#include "string.h"
#include "base64.h"
#include "motor.h"
#include "audio.h"
#include "bat_usart.h"
#include "L74HC595.h"
#include "tim4.h"

//*/
/*******************************************************************************
 * 函 数 名         : USART1_Init
 * 函数功能		   : USART1初始化函数
 * 输    入         : bound:波特率
 * 输    出         : 无
 *******************************************************************************/
void USART1_Init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	USART_DeInit(USART1);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/*  配置GPIO的模式和IO口 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //TX			   //串口输出PA9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	    //复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); /* 初始化串口输入IO */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	    //RX			 //串口输入PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;		  //模拟输入
	GPIO_Init(GPIOA, &GPIO_InitStructure); /* 初始化GPIO */

	//USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;		  //波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		 //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		  //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;		  //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl =
	USART_HardwareFlowControl_None;		  //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStructure); //初始化串口1

	USART_Cmd(USART1, ENABLE);  //使能串口1

	USART_ClearFlag(USART1, USART_FLAG_TC);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  //开启相关中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;  //串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

	//开始拆解注册参数
	ReadAPPServer();
	printf("RegisterParams=%s\r\n", APPServer);
	char *tem = NULL;
	char res[35] =
	{ 0 };
	tem = strtok(APPServer, "-");
	memcpy(RegisterParams.key, tem, 16); //取key
	if (strcmp("aaaaaaaaaaaaaaaa", RegisterParams.key) == 0)
	{
		printf("key is right!\r\n");
	}
	tem = strtok(NULL, "-");
	strcpy(res, tem); //指向ip和port
	//取剩下的参数
	tem = strtok(NULL, "-");
	RegisterParams.needConfirmParams = atoi(tem);
	tem = strtok(NULL, "-");
	RegisterParams.motor_TCtime = atoi(tem);
	tem = strtok(NULL, "-");
	RegisterParams.motor_HTtime = atoi(tem);
	//取ip和port
	// tem = res;
	tem = strtok(res, ":");
	strcpy(RegisterParams.ip, tem);
	tem = strtok(NULL, ":");
	strncpy(RegisterParams.port, tem, 5);
	RegisterParams.port[5] = '\0';
	if (strncmp("13401", RegisterParams.port, 5) == 0)
	{
		printf("port is right!\r\n");
	}
	printf("key=%s,ip=%s,port=%s,confirm=%d,TCtime=%d,HTtime=%d\r\n",
			RegisterParams.key, RegisterParams.ip, RegisterParams.port,
			RegisterParams.needConfirmParams, RegisterParams.motor_TCtime,
			RegisterParams.motor_HTtime);
}
/*******************************************************************************
 * 函 数 名         : USART1_IRQHandler
 * 函数功能		   : USART1中断函数
 * 输    入         : 无
 * 输    出         : 无
 *******************************************************************************/
char usart1TestBuf[18] =
{ 0 };
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 r;
	char *res = NULL;
	u8 mot = 0;
	char buf[128];
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断
	{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		r = USART_ReceiveData(USART1);  //(USART1->DR);	//读取接收到的数据
		if (USART1_Fram.InfBit.Length < (TCP_MAX_LEN - 1))
		{
			USART1_Fram.Data[USART1_Fram.InfBit.Length++] =
					r;
		}
		if (r == ']')
		{
			WDeviceID = strtok((char *)USART1_Fram.Data, "]");
			printf("write DeviceID=%s\r\n", WDeviceID);
			STMFLASH_Write(EEPROM_ADDR, (u16 *) WDeviceID, 8);
//			STMFLASH_Read(EEPROM_ADDR, (u16 *) RDeviceID, 8);
//			printf("成功写入设备编号:%s\r\n", RDeviceID);
			USART1_Fram.InfBit.Length = 0; //重新开始接收新数据
		}
		else if (r == '$')
		{
			res = strtok((char *)USART1_Fram.Data, "$");
			Send_AT_Cmd(In4G, res, "OK", NULL, 500);
			USART1_Fram.InfBit.Length = 0; //重新开始接收新数据
		}
		else if (r == '#')
		{
			res = strtok((char *)USART1_Fram.Data, "#");
			controlPowerbank(hexStr2Byte(res));
			USART1_Fram.InfBit.Length = 0; //重新开始接收新数据
		}
		else if (r == '%')
		{
			res = strtok((char *)USART1_Fram.Data, "%");
			USART1_Fram.InfBit.Length = 0; //重新开始接收新数据
			if (*res == 'l') //亮某个灯
			{
				mot = *(res + 1) - 0x30;
				ledON(mot);
			}
			else if (*res == 'd') //灭某个灯
			{
				mot = *(res + 1) - 0x30;
				ledOFF(mot);
			}
			else if (*res == 'a') //直接操作全部
			{
				res++;
				HC595_Send_Byte(hexStr2Byte(res));
			}
			else if (*res == 'u') //允许更新模块固件
			{
				allowModuleUpdate = 1;
			}
			else if (*res == 'c') //读取4G模块的信息
			{
				allowModuleUpdate = 0;
			}
			else if (*res == 'r') //读取4G模块的信息
			{
				PC_USART("%s", F4G_Fram.Data);
				F4G_Fram.InfBit.Length = 0;
				memset(F4G_Fram.Data, '\0', TCP_MAX_LEN);
			}
		}
		else if (r == '@')
		{
			res = strtok((char *)USART1_Fram.Data, "@");
			USART1_Fram.InfBit.Length = 0; //重新开始接收新数据
			if (*res == 't')
			{
				mot = *(res + 1) - 0x30;
				printf("Trig the Audio_%d.\r\n", mot);
				play_audio(mot);
			}
			else if (*res == '&')
			{
				res++;
				checkPowerbankStatus((u8) atoi(res) - 1, usart1TestBuf);
				//communicationTest(atoi(res) - 1);
				USART1_Fram.InfBit.Length = 0; //重新开始接收新数据
			}
			else if (*res == 'e')
			{
				F4G_ExitUnvarnishSend();
			}
			else if (*res == 's')
			{
				res++;
				base64_encode((const unsigned char *) res, buf);
				TCP_sendStr(USART2, buf);
			}
			else if (*res == 'f')
			{
				mot = *(res + 1) - 0x30;
				printf("now motor forward=%d\r\n", mot);
				motor_run(mot, FORWARD);
			}
			else if (*res == 'b')
			{
				mot = *(res + 1) - 0x30;
				printf("now motor backward=%d\r\n", mot);
				motor_run(mot, BACKWARD);
			}
			else if (*res == 'p')
			{
				mot = *(res + 1) - 0x30;
				printf("now motor stop=%d\r\n", mot);
				motor_stop(mot);
			}
			else if (*res == 'r')
			{
				STMFLASH_Read(EEPROM_ADDR, (u16 *) RDeviceID, 8);
				printf("read DeviceID=%s\r\n", RDeviceID);
				USART1_Fram.InfBit.Length = 0; //重新开始接收新数据
			}
			else if (*res == 'x')
			{
				printf("now Exchange TX3_RX3\r\n");
				TX3_RX3_Exchange;
			}
			else if (*res == 'y')
			{
				mot = *(res + 1) - 0x30;
				communicateWithPort(mot);
			}
		}
	}
	if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
	{
		USART_ClearFlag(USART1, USART_FLAG_ORE);
		USART_ReceiveData(USART1);
	}
}

u8 hexStr2Byte(char *hexStr)
{
	unsigned char highByte, lowByte;
	highByte = *hexStr;
	lowByte = *(hexStr + 1);

	if (highByte > 0x39)
		highByte -= 0x37;
	else
		highByte -= 0x30;

	if (lowByte > 0x39)
		lowByte -= 0x37;
	else
		lowByte -= 0x30;

	return (u8) ((highByte << 4) | lowByte);
}

