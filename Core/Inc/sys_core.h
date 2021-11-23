/*
 * sys_core.h
 *
 *  Created on: Nov 6, 2021
 *      Author: Laplace
 */

#ifndef INC_SYS_CORE_H_
#define INC_SYS_CORE_H_
#include "stm32g0xx_hal.h"
#include <stdint.h>
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdbool.h"

//#define debug
#define LOGLEVEL 					1
#define tmr_not_expired(x, y)       x < y ? 1 : 0
#define MAX_LEN                     256
#define endMsg                      '\n'
#define progVersion                 "MOTE_ALARM_Ver 2.1"
#define MAX_BEAT_TIME               90000 // 1min

//datatypes
#define u8 	uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define s8  int8_t
#define s16 int16_t
#define s32 int32_t


extern char* msg;
extern char server[32];
extern char host[8];
extern u8 	gsm_stage;
extern u8 	tLen;
extern u8	alarm;
extern u16 	Res_Len;
extern u16 	inputLen;
extern bool tcp_alive;
extern bool	gprs_time_found;
extern bool gprs_Is_available;
extern bool dataAck;
extern bool time_synchronized;
extern bool GSM_Is_Powered;

extern char MCC[8];
extern char MNC[8];
extern char LAC[8];
extern char CID[8];
extern char IMEI[32];
extern char ICCID[32];
extern char CSQ[8];
extern char IP[32];
extern char GRD[32];
extern char ttimeBuf[32];
extern char	userInput[64];
extern u8 	gsm_pwr_cnt;
extern u32 	gsm_pwr_tmr;



UART_HandleTypeDef huart1;

extern int __io_putchar(int ch);
extern int _write(int file, char *ptr, int len);
/*
#ifdef debug
extern u16 rec_str(char *buf, u16 MAX);
#endif
*/

#endif /* INC_SYS_CORE_H_ */
