/*
 * sys_core.c
 *
 *  Created on: Nov 10, 2021
 *      Author: LENOVO X250
 */


#include "sys_core.h"


int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}

int _write(int file, char *ptr, int len)
{
	int i = 0;

	for(i = 0; i < len; i++)
	{
		__io_putchar(*ptr++);
	}

	return len;
}
