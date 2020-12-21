/*
 * bat_usart.c
 *
 *  Created on: 2020年6月17日
 *      Author: loyer
 */
#include "bat_usart.h"

Uart3MagType UART3_RX_BUF =
{ 0 };   //中断接收缓存

/**卡口状态*/
const unsigned char bat_statuBuf[16] =
{ 1, 0, 9, 8, 3, 2, 11, 10, 0xFF, 0xFF, 5, 4, 0xFF, 0xFF, 7, 6 };

/**
 * 初始化串口3
 * @bound 波特率-如9600
 */
void USART3_Init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	USART_DeInit(USART3);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/*  配置GPIO的模式和IO口 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //TX			   //串口输出PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	    //复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure); /* 初始化串口输入IO */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	    //RX			 //串口输入PB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;		  //模拟输入
	GPIO_Init(GPIOB, &GPIO_InitStructure); /* 初始化GPIO */

	//USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;		  //波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		 //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		  //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;		  //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl =
	USART_HardwareFlowControl_None;		  //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART3, &USART_InitStructure); //初始化串口1

	USART_Cmd(USART3, ENABLE);  //使能串口1

	USART_ClearFlag(USART3, USART_FLAG_TC);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);  //开启相关中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;  //串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
}

/**
 * 初始化充电宝与主板通信串口
 * @bound 串口波特率
 * 同时初始化不同口充电宝通信选通
 */
void Init_Bat_Usart(u32 bound)
{
	USART3_Init(bound);

	GPIO_InitTypeDef GPIO_InitStructure; //定义结构体变量

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;  //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(GPIOB, &GPIO_InitStructure); /* 初始化GPIO */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8; //选择你要设置的IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 //设置推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  //设置传输速率
	GPIO_Init(GPIOC, &GPIO_InitStructure); /* 初始化GPIO */

}
/**
 * 选择与哪个口的充电宝进行通信
 * @port 数字1-6代表六口设备的六个口
 */
void communicateWithPort(u8 port)
{
	switch (port)
	{
	case 1:
		_A1_L;
		_B1_L;
		break;
	case 2:
		_A1_H;
		_B1_L;
		break;
	case 3:
		_A1_L;
		_B1_H;
		break;
	case 4:
		_A1_H;
		_B1_H;
		_A2_L;
		_B2_L;
		break;
	case 5:
		_A1_H;
		_B1_H;
		_A2_H;
		_B2_L;
		break;
	case 6:
		_A1_H;
		_B1_H;
		_A2_L;
		_B2_H;
		break;
	default:
		break;
	}
//	printf("now communicate with powerBank_%d\r\n", port);
}
/**
 * 测试某个充电宝口的通信状况
 * @port 0-5 代表口1-6
 */
u8 communicationTest(u8 p)
{
	u8 cnt = 0;
	if (!powerbankStatu.Charging[p]) //当前未充电
	{
		controlPowerBankCharge(p + 1, 1); //给充电宝供电
	}
	communicateWithPort(p + 1); //建立串口连接
	do
	{
		if (cnt > 4)
		{
			return 0;
		}
		TX3_RX3_Exchange;
		delay_ms(2);
		controlPowerbank(0x0F);
		delay_ms(50);
		cnt++;
	} while (!powerbankStatu.TEST);
	powerbankStatu.TEST = 0;
	return 1;
}
/**
 * 检查口状态
 * @i 0-5
 * @buf 字段
 * @return 状态码
 */
u8 checkPowerbankStatus(u8 i, char buf[])
{
	u8 pbStatu = 0;
	u8 motorStatu = 0;
	u8 communicationStatu = 0;
	u8 portStatu = 0;
	u8 index = 0;
	u8 pbERROR = 0;
	char i2s[3] =
	{ 0 };

	key_statu = read_74HC165();
	communicationStatu = communicationTest(i);
	pbStatu = ((key_statu & 0xFF) & (1 << i)) == 0 ? 0 : 1;
	motorStatu = (((key_statu >> 8) & 0xFF) & (1 << i)) == 0 ? 0 : 1;
	index = (pbERROR << 3) | (pbStatu << 2) | (communicationStatu << 1)
			| (motorStatu << 0);

	//查状态表
	portStatu = bat_statuBuf[index];
	//增加判断============Begin
	if (portStatu == 11 || portStatu == 1) //有电池，锁弹出异常，通信正常
	{
		if (ReadIgnoreLock(i) == 1) //设置了忽略锁状态
		{
			portStatu = (portStatu == 11) ? 10 : 0;
		}
	}
	//增加判断============End
	//=============================================
	setBATInstruction(i, portStatu);
	//=============================================

	if (communicationStatu)
	{
		if (pbERROR)
		{
			sprintf(buf, "%d_%d_%02X_%s", i, portStatu, powerbankStatu.ERROR,
					(char *) powerbankStatu.ChargeBankID);
		}
		else
		{
//			buf[0] = i + 0x30;
			strcpy(buf, itoa(i, i2s, 10));
			strcat(buf, "_");
			strcat(buf, itoa(portStatu, i2s, 10));
			strcat(buf, "_");
			strcat(buf, itoa(powerbankStatu.VOL, i2s, 10));
			strcat(buf, "_");
			strncat(buf, (char *) powerbankStatu.ChargeBankID, 8);

//			sprintf(buf, "%d_%d_%d_%s", i, portStatu, powerbankStatu.VOL,
//					(char *) powerbankStatu.ChargeBankID);
		}
	}
	//无3、4字段
	else
	{
		snprintf(buf, 4, "%d_%d", i, portStatu);
	}
//	printf("%s\r\n", buf);
	return portStatu;
}

void scanPowerBank(void)
{
	u8 inx = 0;
	for (inx = 0; inx < 6; inx++)
	{
		memset(powerbankStatu.powerBankBuf[inx], '\0', 18);
		TCP_Params.statuCode[inx] = checkPowerbankStatus(inx,
				powerbankStatu.powerBankBuf[inx]);
	}
}

/**
 * 根据检测的状态设置
 */
void setBATInstruction(u8 port, u8 sta)
{
	if (sta >= 0 && sta <= 11)
	{
		if (sta == 10) //卡口正常状态
		{
//			powerbankStatu.ChargingCnt = 0; //从0开始计数
//			if (powerbankStatu.VOL < 95) //电量不满95%充电
//			{
//				for (int j = 0; j < 6; j++)
//				{
//					if (powerbankStatu.Charging[j] != 0) //充电中
//					{
//						powerbankStatu.ChargingCnt++;
//					}
//				}
//				if (powerbankStatu.ChargingCnt < 3)
//				{
//					powerbankStatu.Charging[port] = 1;
//					controlPowerBankCharge(port + 1, 1); //充电
//					HC595_STATUS.slowBLINK[port] = 0; //慢闪
//					ledON(port + 1);
//					HC595_STATUS.fastBLINK[port] = 0;
//				}
//				else
//				{
//					powerbankStatu.Charging[port] = 0;
//					controlPowerBankCharge(port + 1, 0); //不充电
//					ledON(port + 1);
//					HC595_STATUS.slowBLINK[port] = 0; //慢闪
//					HC595_STATUS.fastBLINK[port] = 0;
//				}
//			}
//			else
//			{
//				powerbankStatu.Charging[port] = 0;
//				controlPowerBankCharge(port + 1, 0); //不充电
//				ledON(port + 1);
//				HC595_STATUS.slowBLINK[port] = 0; //慢闪
//				HC595_STATUS.fastBLINK[port] = 0;
//			}
			//11.10更改充电逻辑Start=========================================
			ledON(port + 1);
			HC595_STATUS.slowBLINK[port] = 0; //慢闪
			HC595_STATUS.fastBLINK[port] = 0;
			if (powerbankStatu.VOL < 95) //电量不满95%充电
			{
				controlPowerBankCharge(port + 1, 1); //充电
			}
			else
			{
				controlPowerBankCharge(port + 1, 0); //不充电
			}
			//11.10更改充电逻辑End===========================================
		}
		else if (sta == 0) //正常无充电宝状态
		{
			powerbankStatu.Charging[port] = 0;
			controlPowerBankCharge(port + 1, 0); //不充电
			ledOFF(port + 1);
			HC595_STATUS.slowBLINK[port] = 0; //慢闪
			HC595_STATUS.fastBLINK[port] = 0;
		}
		else //有异常
		{
			powerbankStatu.Charging[port] = 0;
			controlPowerBankCharge(port + 1, 0); //不充电
			HC595_STATUS.slowBLINK[port] = 0; //慢闪
			HC595_STATUS.fastBLINK[port] = 1; //快闪
			//ledON(port + 1);
		}
	}
}
//============================================================================

uint8_t BSP_UART_SendOneByte(uint8_t data)
{
	USART_SendData(USART3, data);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TC) != SET)
		;
	return (data);
}

/**
 * CRC校验函数
 * @pbuf 待校验的数组
 * @num 待校验的个数
 * @return 校验后的crc值
 */
unsigned int get_crc(unsigned char pbuf[], unsigned char num)
{
	unsigned char i, j;
	unsigned int wcrc = 0x0000;
	for (i = 0; i < num; i++)
	{
		wcrc ^= (pbuf[i] << 8);
		for (j = 0; j < 8; j++)
		{
			if (wcrc & 0x8000)
			{
				wcrc <<= 1;
				wcrc ^= 0x1021;
			}
			else
			{
				wcrc <<= 1;
			}
		}
	}
	return wcrc;
}
//unsigned int get_crc(unsigned char pbuf[], unsigned char num)
//{
//	unsigned char i, j;
//	unsigned int wcrc = 0xffff;
//	for (i = 0; i < num; i++)
//	{
//		wcrc ^= (unsigned int) (pbuf[i]);
//		for (j = 0; j < 8; j++)
//		{
//			if (wcrc & 1)
//			{
//				wcrc >>= 1;
//				wcrc ^= 0xa001;
//			}
//			else
//			{
//				wcrc >>= 1;
//			}
//		}
//	}
//	return wcrc;
//}
/**
 * CRC16校验函数
 * 可变参，@num为进行校验的数量
 */
unsigned int get_crc_2(int num, ...)
{
	va_list ap;  //声明参数指针
	unsigned int wcrc = 0x0000;
	unsigned char j = 0;
	va_start(ap, num); //设置参数指针
	while (num)
	{
		wcrc ^= (va_arg(ap, int) << 8);
		for (j = 0; j < 8; j++)
		{
			if (wcrc & 0x8000)
			{
				wcrc <<= 1;
				wcrc ^= 0x1021;
			}
			else
			{
				wcrc <<= 1;
			}
		}
		num--;
	}
	va_end(ap); //释放指针资源
	return wcrc;
}
//unsigned int get_crc_2(int num, ...)
//{
//	va_list ap;  //声明参数指针
//	unsigned int wcrc = 0xffff;
//	unsigned char j = 0;
//	va_start(ap, num); //设置参数指针
//
//	while (num)
//	{
//		wcrc ^= (unsigned int) (va_arg(ap, int));
//		for (j = 0; j < 8; j++)
//		{
//			if (wcrc & 1)
//			{
//				wcrc >>= 1;
//				wcrc ^= 0xa001;
//			}
//			else
//			{
//				wcrc >>= 1;
//			}
//		}
//		num--;
//	}
//	va_end(ap); //释放指针资源
//	return wcrc;
//}
/*
 * 函数名称：BSP_UART_SendMultipleBytes
 * 函数说明：串口发送多字节数据
 * IN：      data 待发送数据
 * OUT：     无
 * Return：  无
 */
void BSP_UART_SendMultipleBytes(unsigned char buf[], unsigned char len)
{
	uint8_t i;
	UART3_RX_BUF.counter = 0;
	for (i = 0; i < len; i++)
	{
		BSP_UART_SendOneByte(buf[i]);
	}
}

FramType framMag =
{ 0 };
PowerBankType powerbankStatu =
{ 0 };
/** function: app_frame_anasys
 *   breaf: 帧的协议解析
 *   input:  none
 *   output: none
 *   return: none
 */
void app_frame_anasys(void)
{
	uint8_t i = 0;
	uint16_t crc;
//取出指令长度
	framMag.len = UART3_RX_BUF.data[1];
//取出控制字
	framMag.cmd = UART3_RX_BUF.data[2];
//填充待CRC校验的字节
	for (i = 0; i < framMag.len - 4; i++)
	{
		framMag.CRC_BUF[i] = UART3_RX_BUF.data[i + 1];
	}
	crc = get_crc(framMag.CRC_BUF, framMag.len - 4);
//判断CRC校验值是否正确
	if (crc
			== (UART3_RX_BUF.data[framMag.len - 3] << 8
					| UART3_RX_BUF.data[framMag.len - 2]))
	{
		//开始解析命令
		app_cmd_anasys();
	}
}
/** function: app_cmd_anasys
 *   breaf: 命令的协议解析
 *   input:  none
 *   output: none
 *   return: none
 */
void app_cmd_anasys(void)
{
	uint8_t inx = 0;
	switch (framMag.cmd)
	{
	case 0x8F:
		powerbankStatu.TEST = 1;
		powerbankStatu.VOL = UART3_RX_BUF.data[11];
		for (inx = 0; inx < ChargeBankIDSize; inx++)
		{
			powerbankStatu.ChargeBankID[inx] = UART3_RX_BUF.data[inx + 3];
		}
		//		printf("%s.vol=%02X\r\n", powerbankStatu.ChargeBankID,
		//				powerbankStatu.VOL);
		break;
	case ComTest:
		powerbankStatu.TEST = 1;
//		printf("test powerbank UART success.\r\n");
		break;
	case UP_ReadID:
		powerbankStatu.TEST = 1;
		for (inx = 0; inx < ChargeBankIDSize; inx++)
		{
			powerbankStatu.ChargeBankID[inx] = UART3_RX_BUF.data[inx + 3];
		}
//		printf("current powerbankID=%s\r\n", powerbankStatu.ChargeBankID);
		break;
	case UP_ReadVOL:
		powerbankStatu.TEST = 1;
		powerbankStatu.VOL = UART3_RX_BUF.data[11];
		for (inx = 0; inx < ChargeBankIDSize; inx++)
		{
			powerbankStatu.ChargeBankID[inx] = UART3_RX_BUF.data[inx + 3];
		}
//		printf("%s.vol=%02X\r\n", powerbankStatu.ChargeBankID,
//				powerbankStatu.VOL);
		break;
	case UP_ReadCUR:
		powerbankStatu.TEST = 1;
		powerbankStatu.CUR = (UART3_RX_BUF.data[11] << 8)
				| UART3_RX_BUF.data[12];
		for (inx = 0; inx < ChargeBankIDSize; inx++)
		{
			powerbankStatu.ChargeBankID[inx] = UART3_RX_BUF.data[inx + 3];
		}
//		printf("%s.cur=%04X\r\n", powerbankStatu.ChargeBankID,
//				powerbankStatu.CUR);
		break;
	case UP_ReadERROR:
		powerbankStatu.TEST = 1;
		powerbankStatu.ERROR = UART3_RX_BUF.data[11];
		for (inx = 0; inx < ChargeBankIDSize; inx++)
		{
			powerbankStatu.ChargeBankID[inx] = UART3_RX_BUF.data[inx + 3];
		}
//		printf("%s.ERROR=%02X\r\n", powerbankStatu.ChargeBankID,
//				powerbankStatu.ERROR);
		break;
	default:
		break;
	}
}
/**
 * 控制充电宝
 * @cmd 控制字
 */
void controlPowerbank(u8 cmd)
{
	unsigned char DOWN_Data_BUF[6];
	u16 crc;
	DOWN_Data_BUF[0] = CMD_Header;
	DOWN_Data_BUF[1] = 0x06;
	DOWN_Data_BUF[2] = cmd;
	crc = get_crc_2(2, DOWN_Data_BUF[1], DOWN_Data_BUF[2]);
	DOWN_Data_BUF[3] = crc >> 8 & 0xFF;
	DOWN_Data_BUF[4] = crc & 0xFF;
	DOWN_Data_BUF[5] = CMD_End;

	BSP_UART_SendMultipleBytes(DOWN_Data_BUF, DOWN_Data_BUF[1]);
}

/*******************************************************************************
 * 函 数 名         : USART3_IRQHandler
 * 函数功能		   : USART3中断函数
 * 输    入         : 无
 * 输    出         : 无
 *******************************************************************************/
void USART3_IRQHandler(void)
{
	char data;
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断
	{
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		data = USART_ReceiveData(USART3);  //(USART1->DR);	//读取接收到的数据
		if (UART3_RX_BUF.flag == 0)  //是否允许接收新的数据包
		{
			UART3_RX_BUF.data[UART3_RX_BUF.counter] = data; //填充数据
			UART3_RX_BUF.counter++;
			switch (UART3_RX_BUF.counter)
			{
			case 1:  //校验起始字符
				if (data != UP_Header)
					UART3_RX_BUF.counter = 0;
				break;
			case 2:  //获取数据包长度
				UART3_RX_BUF.len = data;
				break;
			case UART3_RX_BUF_SIZE: //缓冲区已满但是仍未接收到正确的结束字符
				UART3_RX_BUF.counter = 0;
				//BSP_UART_SendOneByte(0xF3); //返回0xF3代表数据发送有误
				printf("UART3 data ERROR.\r\n");
				break;
			default:
				//获取完全部数据，且最后一字节数据校验正确
				if (UART3_RX_BUF.counter == UART3_RX_BUF.len && data == UP_End)
				{
					UART3_RX_BUF.flag = 1;
					UART3_RX_BUF.counter = 0;
					app_frame_anasys();
					UART3_RX_BUF.flag = 0;
					//BSP_UART_SendOneByte(0xFF);
				}
				break;
			}
		}
	}
	if (USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)
	{
		USART_ClearFlag(USART3, USART_FLAG_ORE);
		USART_ReceiveData(USART3);
	}
//	USART_ClearFlag(USART3, USART_FLAG_TC);
}
