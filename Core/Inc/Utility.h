/*
 * Utility.h
 *
 *  Created on: Nov 6, 2021
 *      Author: LENOVO X250
 */

#ifndef INC_UTILITY_H_
#define INC_UTILITY_H_
#include "sys_core.h"

RTC_HandleTypeDef hrtc;
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;




extern u16 gsm_data_grabber(const char *rawstr, u16 len, const char *start_id, char end_id, char *grabs, u16 max_grab);
extern void putStringInArray(const char* s, const char* oldW, const char* newW, char* result, int rLen);
extern int makeIntArr(char *buf, int len, s16* arrInt);
extern void synchronize_time(void);
extern s16 getTime_string(char* timeStr);


#endif /* INC_UTILITY_H_ */
