/*
 * gsm_process.h
 *
 *  Created on: Nov 6, 2021
 *      Author: Laplace
 */

#ifndef INC_GSM_PROCESS_H_
#define INC_GSM_PROCESS_H_

#include "gsm_string.h"
#include "Utility.h"

#define GSM_DELAY       		6000

extern const char 				*payload;
extern u32 						send_tmr;
extern const char*      		server_start_str;
extern const char*      		end_str;
extern bool						respond;


UART_HandleTypeDef huart2;

void setUp_GSM(void);
void GSM_Process(void);

#endif /* INC_GSM_PROCESS_H_ */
