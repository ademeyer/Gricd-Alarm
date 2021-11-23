/*
 * Utility.c
 *
 *  Created on: Nov 6, 2021
 *      Author: Laplace
 */


#include "Utility.h"




bool time_synchronized = false;




u16 gsm_data_grabber(const char *rawstr, u16 len, const char *start_id, char end_id, char *grabs, u16 max_grab)
{
  u16 p = 0, j = 0, u = 0;
  char s_id[32] = "";
  u8 grab = 0;
  for (j=0; j < len; j++)
  {
    if ((strcmp(s_id, start_id) == 0) && !grab)
      grab = 1;

    char c = rawstr[j];
    if (grab == 0)
    {
      if (c == start_id[u])
        s_id[u++] = c;

      else
        u = 0;
    }

    if (c == end_id && end_id != '\0' && grab == 1)
      break;

    if (grab == 1 && p < max_grab)
    {
      grabs[p++] = c;
    }
  }

  grabs[p] = '\0';

  return p;
}

void putStringInArray(const char* s, const char* oldW, const char* newW, char* result, int rLen)
{
  //char* result;
  int i, cnt = 0;
  int newWlen = strlen(newW);
  int oldWlen = strlen(oldW);

  // Counting the number of times old word
  // occur in the string
  for (i = 0; s[i] != '\0'; i++)
  {
    if (strstr(&s[i], oldW) == &s[i])
    {
      cnt++;
      // Jumping to index after the old word.
      i += oldWlen - 1;
    }
  }

  i = 0;
  while (*s)
  {
    // compare the substring with the result
    if (strstr(s, oldW) == s)
    {
      strcpy(&result[i], newW);
      i += newWlen;
      s += oldWlen;
    }
    else
    {
      if (i < rLen)
        result[i++] = *s++;
    }
  }

  result[i] = '\0';
}


int makeIntArr(char *buf, int len, s16* arrInt)
{
  char _temp[10];
  int j = 0, Len = 0;

  for (int i = 0; i <= len; i++)
  {
    char c = buf[i];
    if ((c == ',') || (c == ' ') || (c == '.') || (c == '/') || (c == ':') || i == len)
    {
      if (j > 0 && strlen(_temp) > 0)
      {
        arrInt[Len++] = (s16)atoi(_temp);
        memset(_temp, 0, sizeof(_temp));
        j = 0;
      }
    }
    else if (c == '-')
    {
      arrInt[Len++] = -1;
      memset(_temp, 0, sizeof(_temp));
      i++; //move i one step
    }
    else if (c >= 48 && c <= 57)
    {
      _temp[j++] = c;
    }

  }
  return Len;
}

void setDateTime(char *buf, int len)
{
  //yr, mn, dy, hr, mm,ss;
  s16 timeTemp[6] = {0};
  s16 P = makeIntArr(buf, len, timeTemp);
  if (P < 6 ) return;

  if(timeTemp[0] >= 2000) timeTemp[0] = timeTemp[0] - 2000;

  sDate.Year = timeTemp[0];
  sDate.Month = timeTemp[1];
  sDate.Date = timeTemp[2];
  HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);


  sTime.Hours = timeTemp[3];
  sTime.Minutes = timeTemp[4];
  sTime.Seconds = timeTemp[5];
  HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);


}

s16 getTime_string(char* timeStr)
{
  s16 len = 0;
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  len = sprintf(timeStr, "%u/%u/%u %02u:%02u:%02u", sDate.Year, sDate.Month, sDate.Date, sTime.Hours, sTime.Minutes, sTime.Seconds);
  return len;
}

void synchronize_time(void)
{
  //format: yr/mn/dd, hr:mm:ss+tz

  if (gprs_time_found && !time_synchronized)
  {
    setDateTime(ttimeBuf, tLen);
#ifdef debug
    printf(("Time Synchronized using %s:%d\r\n"),ttimeBuf, tLen);
#endif
    memset(ttimeBuf, 0, sizeof(ttimeBuf));
    tLen = 0;
    time_synchronized = true;
  }
}
/*
#ifdef debug
u16 rec_str(char *buf, u16 MAX)
{
	u16 len = 0;
	while(1)
	{
		u8 ch = 0;
		HAL_UART_Receive(&huart1, &ch, 1, 2);

		if(ch)
		{
			if(len < MAX)
				buf[len++] = ch;
		}
		else
			break;
	}
	return len;
}
#endif
*/
