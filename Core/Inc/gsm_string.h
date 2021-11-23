/*
 * gsm_string.h
 *
 *  Created on: Nov 6, 2021
 *      Author: Laplace
 */

#ifndef INC_GSM_STRING_H_
#define INC_GSM_STRING_H_
#include "sys_core.h"
#include "Flash_Data.h"

#define BAUD_RATE                   115200
#define USE_GLOBAL_SIM              0
#define MAX_GSM_STAGE               7
#define MAX_ALIVE_TIME              (2*MAX_BEAT_TIME)
//Action maximum process number
#define DETECT_GSM_NO               11
#define INIT_GPRS_NO                6
#define NTP_SERVER_NO               6
#define INIT_TCP_NO                 2
#define NET_STAT_NO                 1
#define SEND_DATA_NO                3
#define CLOSE_NET_NO                2


//variable
typedef enum
{
	_DETECT_GSM_SIM,
	_INIT_GPRS,
	_NTP_SERVER,
	_INIT_TCP,
	_NET_STAT,
	_SEND_DATA,
	_CLOSE_NET
} _gsm_process;


extern _gsm_process 					   _GSM_PROCESS;
extern const u16                           GSM_MAX_STAGE_STEP[MAX_GSM_STAGE];
extern u8                                  gsm_Raw_Buffer[MAX_LEN];
extern char 							   RES_BUFF[MAX_LEN];
extern char 							   custom_apn[32];
extern s8                                  net;
extern s8                                  apn_net ;
extern u8                                  stage_step;
extern u8                                  stage_jump;
extern u16								   Res_Len;
extern u16 								   grab_len;
extern u32                                 gsm_response_tmr;
extern u32                                 gsm_operation_delay;
extern u32                                 tcp_alive_timer;
extern u32                                 routine_id;
extern bool								   Isroaming;
extern bool								   retry;

typedef struct
{
  const char *query_str;
  const char *resp_str;
  u16 wait_tmr;
} gsm_parameter;


extern gsm_parameter detect_gsm    [DETECT_GSM_NO];
extern gsm_parameter init_gprs     [INIT_GPRS_NO];
extern gsm_parameter ntp_server    [NTP_SERVER_NO];
extern gsm_parameter init_tcp      [INIT_TCP_NO];
extern gsm_parameter net_stat      [NET_STAT_NO];
extern gsm_parameter send_data     [SEND_DATA_NO];
extern gsm_parameter close_net     [CLOSE_NET_NO];









void init_apn_str(void);
void init_GSM_str(void);
s8 id_network_name(const char* mccmnc);
//void connectivity_watchdog(void);
void network_failed_restart(void);

#endif /* INC_GSM_STRING_H_ */
