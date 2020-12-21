/*
 * tcp_public.c
 *
 *  Created on: 2020��10��24��
 *      Author: loyer
 */
#include "tcp_public.h"

struct STRUCT_USART_Params TCP_Params =
{ 0 };
struct STRUCT_USART_Fram WIFI_Fram =
{ 0 };
struct STRUCT_USART_Fram F4G_Fram =
{ 0 };
volatile u8 CurrentInternet = In4G;

static char *itoa(int value, char *string, int radix)
{
	int i, d;
	int flag = 0;
	char *ptr = string;

	/* This implementation only works for decimal numbers. */
	if (radix != 10)
	{
		*ptr = 0;
		return string;
	}

	if (!value)
	{
		*ptr++ = 0x30;
		*ptr = 0;
		return string;
	}

	/* if this is a negative value insert the minus sign. */
	if (value < 0)
	{
		*ptr++ = '-';

		/* Make the value positive. */
		value *= -1;

	}

	for (i = 10000; i > 0; i /= 10)
	{
		d = value / i;

		if (d || flag)
		{
			*ptr++ = (char) (d + 0x30);
			value -= (d * i);
			flag = 1;
		}
	}

	/* Null terminate the string. */
	*ptr = 0;

	return string;

} /* NCL_Itoa */

void _USART_printf(USART_TypeDef * USARTx, char * Data, ...)
{
	const char *s;
	int d;
	char buf[16];

	va_list ap;
	va_start(ap, Data);

	while (*Data != 0)     // �ж��Ƿ񵽴��ַ���������
	{
		if (*Data == 0x5c)  //'\'
		{
			switch (*++Data)
			{
			case 'r':							          //�س���
				USART_SendData(USARTx, 0x0d);
				Data++;
				break;

			case 'n':							          //���з�
				USART_SendData(USARTx, 0x0a);
				Data++;
				break;

			default:
				Data++;
				break;
			}
		}

		else if (*Data == '%')
		{									  //
			switch (*++Data)
			{
			case 's':										  //�ַ���
				s = va_arg(ap, const char *);

				for (; *s; s++)
				{
					USART_SendData(USARTx, *s);
					while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
						;
				}

				Data++;

				break;

			case 'd':
				//ʮ����
				d = va_arg(ap, int);

				itoa(d, buf, 10);

				for (s = buf; *s; s++)
				{
					USART_SendData(USARTx, *s);
					while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
						;
				}

				Data++;

				break;

			default:
				Data++;

				break;

			}
		}

		else
			USART_SendData(USARTx, *Data++);

		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
			;

	}
}

/**
 * ��ģ�鷢��ATָ��
 * @cmd�������͵�ָ��
 * @ack1��@ack2���ڴ�����Ӧ��ΪNULL��������Ӧ������Ϊ���߼���ϵ
 * @time���ȴ���Ӧ��ʱ��
 * @return 1�����ͳɹ� 0��ʧ��
 */
bool Send_AT_Cmd(ENUM_Internet_TypeDef internet, char *cmd, char *ack1,
		char *ack2, u32 time)
{
	struct STRUCT_USART_Fram *USART_Fram;
	USART_TypeDef *USARTx;

	if (internet == In4G)
	{
		USART_Fram = &F4G_Fram;
		USARTx = USART2;
	}
	else
	{
		USART_Fram = &WIFI_Fram;
		USARTx = UART4;
	}

	USART_Fram->IsNotInAT = 0; //ATָ�������

	USART_Fram->InfBit.Length = 0;	//���¿�ʼ�����µ����ݰ�
	_USART_printf(USARTx, "%s\r\n", cmd);
	if (ack1 == 0 && ack2 == 0)	 //����Ҫ��������
	{
		return true;
	}
	delay_ms(time);	  //��ʱtimeʱ��

	USART_Fram->Data[USART_Fram->InfBit.Length] = '\0';

	printf("%s", USART_Fram->Data);

	if (ack1 != 0 && ack2 != 0)
	{
		//USART_Fram->InfAll = 0;
		return (( bool ) strstr((const char *) USART_Fram->Data, ack1)
				|| ( bool ) strstr((const char *) USART_Fram->Data, ack2));
	}
	else if (ack1 != 0)
	{
		//USART_Fram->InfAll = 0;
		return (( bool ) strstr((const char *) USART_Fram->Data, ack1));
	}
	else
	{
		//USART_Fram->InfAll = 0;
		return (( bool ) strstr((const char *) USART_Fram->Data, ack2));
	}
}

bool AT_Test(ENUM_Internet_TypeDef internet)
{
	u8 count = 0;
	char *module = mymalloc(10);
	if(internet == In4G)
	{
		sprintf(module, "%s", "4G module");
	}
	else
	{
		sprintf(module, "%s", "WIFI module");
	}
	while (count++ < 8)
	{
		Send_AT_Cmd(internet, "AT", "OK", NULL, 500);
	}
	if (Send_AT_Cmd(internet, "AT", "OK", NULL, 500))
	{
		printf("test %s success!\r\n", module);
		myfree(module);
		return 1;
	}
	printf("test %s fail!\r\n", module);
	myfree(module);
	return 0;
}