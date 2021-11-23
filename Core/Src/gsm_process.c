/*
 * gsm_process.c
 *
 *  Created on: Nov 6, 2021
 *      Author: Laplace
 */

#include "gsm_process.h"

const char 							*payload 						= 			"##{0},{1},ALARM,{2}";
u32 								send_tmr 						= 			0;
const char*                         server_start_str              	=         	"$$";
const char*                         end_str                       	=           "#";
bool								respond							=			false;
u8									alarm							=			0;
u8 									eof[] 							= 			{0x1A};
bool								GSM_Is_Powered					=           false;
u8 									gsm_pwr_cnt						=			0;
u32 							    gsm_pwr_tmr						=			1;



void sendPacket_Handler(void)
{
	if(tcp_alive && (respond || HAL_GetTick() >= (send_tmr + MAX_BEAT_TIME))) //90s
	{
		memset(RES_BUFF, 0, sizeof(RES_BUFF));
		if(respond)
		{
			strcat(RES_BUFF, "##");
			strcat(RES_BUFF, userInput);
			//putStringInArray(userInput, server_start_str, "##", RES_BUFF, sizeof(RES_BUFF));
		}
		else
		{
			memset(ttimeBuf, 0, sizeof(ttimeBuf));
			tLen = getTime_string(ttimeBuf);
			char temp_str[128] = "", temp_str1[128] = "", foo[4] = "";
			sprintf(foo, "%u", alarm);
			putStringInArray(payload, "{0}", ttimeBuf, temp_str, sizeof(temp_str));
			putStringInArray(temp_str, "{1}", IMEI, temp_str1, sizeof(temp_str1));
			putStringInArray(temp_str1, "{2}", foo, RES_BUFF, sizeof(RES_BUFF));
		}

        strcat(RES_BUFF, end_str);
        Res_Len = strlen(RES_BUFF);


#ifdef debug
        printf(("Sending Formed: %s<->%d\n"), RES_BUFF, Res_Len);
#endif
		gsm_stage = _NET_STAT;
		send_tmr = HAL_GetTick();
		respond = false;
	}
}

void receivePacket_Handler(void)
{//$$867157042962209,169389728377,1#
	//$$imei,serial_number,alarm_state#

	if(inputLen > 0)
	{
		char foo[4] = "";
#ifdef debug
		printf(("received: %s <-> %d\r\n"), userInput, inputLen);
#endif
        char tem_bb[32] = "", bbload[64] = "";
        gsm_data_grabber(userInput, inputLen, IMEI, ',', tem_bb, sizeof(tem_bb));
        strcat(bbload, tem_bb);
        strcat(bbload, ",");
        gsm_data_grabber(userInput, inputLen, bbload, ',', tem_bb, sizeof(tem_bb));
        strcat(bbload, tem_bb);
        strcat(bbload, ",");
        gsm_data_grabber(userInput, inputLen, bbload, ',', foo, sizeof(foo));
		alarm = atoi(foo);
#ifdef debug
		printf(("alarm: %s:%d\r\n"), foo, alarm);
#endif
		inputLen = 0;
		respond = true;
	}
}


void send_GSM_str(const char *str, u16 str_len, u16 wait, bool endTrans)
{
	if(str_len > 0)
	{
		HAL_UART_Transmit(&huart2, (u8*)str, str_len, 1000);
		if(endTrans)
			HAL_UART_Transmit(&huart2, eof, 1, 50);
		else
			HAL_UART_Transmit(&huart2, (u8*)"\r\n", 2, 50);
	}

	gsm_response_tmr = HAL_GetTick() + wait;
}


void connectivity_watchdog(void)
{
  if ((HAL_GetTick() > tcp_alive_timer) || retry)
  {
#ifdef debug
	  int ttl = MAX_ALIVE_TIME / 60000;
	  printf(("well, its over %dmin and no successful heart beat\r\n"), ttl);
#endif
	  //send_GSM_str("AT+CIPCLOSE", 11, 1, true);
	  //HAL_Delay(2000);
    if (tcp_alive) tcp_alive = false;
    if (gsm_stage >= _INIT_TCP)
    {
      network_failed_restart();
    }

    tcp_alive_timer = HAL_GetTick() + MAX_ALIVE_TIME;

    retry = false;
    HAL_NVIC_SystemReset();
  }
}

u16 rec_GSM_str(void)
{
	u16 len = 0;
	while(1)
	{
		u8 ch = 0;
		HAL_UART_Receive(&huart2, &ch, 1, 2);

		if(ch)
		{
			if(len < MAX_LEN)
				gsm_Raw_Buffer[len++] = ch;
		}
		else
			break;
	}
	return len;
}

bool GSM_str_is_valid(const char *comp_str)
{
  if (strstr((char*)gsm_Raw_Buffer, comp_str) != NULL)
    return true;

#ifdef debug
  printf(("\nGSM str byte %d is not valid for %d:%d\r\n"), grab_len, gsm_stage, stage_step);
#endif
  return false;
}

void GSM_Send_Handler(void)
{
  if (tmr_not_expired(HAL_GetTick(), gsm_response_tmr) || gsm_stage >= (MAX_GSM_STAGE - 1))return;

  switch (gsm_stage)
  {
    case _DETECT_GSM_SIM:
      if (!Isroaming && stage_step == 10)
      {
        gsm_stage++; // if time is already, move to the next gms stage
        stage_step = 0;
      }
      else
      {
#ifdef debug
        printf(("query: %s,  resp:%s,  time: %ld\r\n"), detect_gsm[stage_step].query_str, detect_gsm[stage_step].resp_str, detect_gsm[stage_step].wait_tmr);
#endif
        send_GSM_str(detect_gsm[stage_step].query_str, strlen(detect_gsm[stage_step].query_str), detect_gsm[stage_step].wait_tmr, false);
      }
      break;

    case _INIT_GPRS:
      if (apn_net == -1) return;        //if sim apn is not detected, no reason to run this at all
#ifdef debug
      printf(("query: %s, wait resp: %ld\r\n"), init_gprs[stage_step].query_str, init_gprs[stage_step].wait_tmr);
#endif
      if (stage_step == 3)
      {
        char temp_str[MAX_LEN] = "";
        if (strlen(custom_apn) <= 0)
          putStringInArray(init_gprs[stage_step].query_str, "{0}", APN[apn_net], temp_str, sizeof(temp_str));
        else
          putStringInArray(init_gprs[stage_step].query_str, "{0}", custom_apn, temp_str, sizeof(temp_str));
        send_GSM_str(temp_str, strlen(temp_str), init_gprs[stage_step].wait_tmr, false);
      }
      else
        send_GSM_str(init_gprs[stage_step].query_str, strlen(init_gprs[stage_step].query_str), init_gprs[stage_step].wait_tmr, false);
      break;

    case _NTP_SERVER:
#ifdef debug
      printf(("query: %s, wait resp: %ld\r\n"), ntp_server[stage_step].query_str, ntp_server[stage_step].wait_tmr);
#endif
      if (gprs_time_found)
      {
        gsm_stage++; // if time is already, move to the next gms stage
        stage_step = 0;
      }
      else
        send_GSM_str(ntp_server[stage_step].query_str, strlen(ntp_server[stage_step].query_str), ntp_server[stage_step].wait_tmr, false);
      break;

    case _INIT_TCP:
      if (!gprs_Is_available) gsm_stage = gsm_stage - 2; //if internet is not active, no reason to run this at all, rather move a step backward and try init gprs
      else
      {
#ifdef debug
        printf(("query: %s, wait resp: %ld\r\n"), init_tcp[stage_step].query_str, init_tcp[stage_step].wait_tmr);
#endif
        if (stage_step == 0) //insert detected APN
        {
          char temp_str1[MAX_LEN] = "", temp_str2[MAX_LEN] = "";
          putStringInArray(init_tcp[stage_step].query_str, "{0}", server, temp_str1, sizeof(temp_str1));
          putStringInArray(temp_str1, "{1}", host, temp_str2, sizeof(temp_str2));
          send_GSM_str(temp_str2, strlen(temp_str2), init_tcp[stage_step].wait_tmr, false);
        }
        else
          send_GSM_str(init_tcp[stage_step].query_str, strlen(init_tcp[stage_step].query_str), init_tcp[stage_step].wait_tmr, false);
      }
      break;

    case _NET_STAT:
#ifdef debug
      printf(("query: %s, wait resp: %ld\r\n"), net_stat[stage_step].query_str, net_stat[stage_step].wait_tmr);
#endif
      send_GSM_str(net_stat[stage_step].query_str, strlen(net_stat[stage_step].query_str), net_stat[stage_step].wait_tmr, false);
      break;

    case _SEND_DATA:
#ifdef debug
      printf(("query: %s, wait resp: %ld\n"), send_data[stage_step].query_str, send_data[stage_step].wait_tmr);
#endif
      if (Res_Len <= 0)
      {
#ifdef debug
        printf(("There is no data to send\r\n"));
#endif
        gsm_stage++;        //if there is nothing to send, just move on
        stage_step = 0;
      }
      else
      {
        if (stage_step == 1 || stage_step == 3) //insert data to be sent
        {
#ifdef debug
          printf(("datalen: %d\r\n"), Res_Len);
#endif
          char temp_str[MAX_LEN] = "";
          //if(stage_step == 1)
          putStringInArray(send_data[stage_step].query_str, "{0}", RES_BUFF, temp_str, sizeof(temp_str));
          send_GSM_str(temp_str, strlen(temp_str), send_data[stage_step].wait_tmr, true);
          stage_step++;
        }
        else
          send_GSM_str(send_data[stage_step].query_str, strlen(send_data[stage_step].query_str), send_data[stage_step].wait_tmr, false);
      }
      break;

    case _CLOSE_NET:
#ifdef debug
      printf(("query: %s, wait resp: %ld\r\n"), close_net[stage_step].query_str, close_net[stage_step].wait_tmr);
#endif
      send_GSM_str(close_net[stage_step].query_str, strlen(close_net[stage_step].query_str), close_net[stage_step].wait_tmr, false);
      break;


    default:
#ifdef debug
      printf(("unknown gsm_stage: %d\r\n"), gsm_stage);
#endif
      break;
  }
}


void GSM_Response_Handler(void)
{
  bool resp = false;
  switch (gsm_stage)
  {
    case _DETECT_GSM_SIM:  //detect_gsm_sim
      if ((resp = GSM_str_is_valid(detect_gsm[stage_step].resp_str)))
      {
#ifdef debug
        printf(("GSM response is valid for stage_step: %d:%d\r\n"), gsm_stage, stage_step);
#endif
        //process of grabbing useful data
        if(stage_step == 0)
        {
        	gsm_pwr_tmr = 0;
        }
        else if (stage_step == 2) //grab IMEI
        {
          gsm_data_grabber((char*)gsm_Raw_Buffer, grab_len, "\n", '\r', IMEI, sizeof(IMEI));
#if LOGLEVEL
          printf(("IMEI: %s\r\n"), IMEI);
#endif
        }
        else if (stage_step == 3) //grab ICCID
        {
          gsm_data_grabber((char*)gsm_Raw_Buffer, grab_len, "\n", '\r', ICCID, sizeof(ICCID));
#if LOGLEVEL
          printf(("ICCID: %s\r\n"), ICCID);
#endif
        }
        else if (stage_step == 8) //check if SIM is roaming
        {
          if (strstr((char*)gsm_Raw_Buffer, "+CGREG: 0,5") != NULL) Isroaming = true;
        }
        else if (stage_step == 9)    //grab net ID
        {
          //if(net = id_network_name(gsm_Raw_Buffer) == -1) net = 4;
          net = id_network_name((char*)gsm_Raw_Buffer);
          if (strlen(custom_apn) <= 0 && net > -1 && net < 4) apn_net = net;

        }
        else if (stage_step == 10 && Isroaming) //differentiate between Flolive and JT
        {
          if (strstr((char*)gsm_Raw_Buffer, "FloLive") != NULL) apn_net = 4; //AT+CSPN?
          else apn_net = 5;
        }
      }
      else
      {
    	  if(stage_step == 0)
    	  {
    		  printf("gsm_pwr_tmr\r\n");
    		  gsm_pwr_tmr = 1;
    		  if(gsm_pwr_cnt >= 2)
    			  gsm_pwr_cnt = 0;
    		  //GSM_Is_Powered = false;
    		  //gsm_operation_delay = HAL_GetTick() + GSM_DELAY + 5000;
    	  }
      }
      break;

    case _INIT_GPRS: //init gprs connectivity
      if ((resp = GSM_str_is_valid(init_gprs[stage_step].resp_str)))
      {
#ifdef debug
        printf(("GSM response is valid for stage_step: %d:%d\r\n"), gsm_stage, stage_step);
#endif

        //process of grabbing useful data
        if (stage_step == 0)    //grab CSQ
        {
          gsm_data_grabber((char*)gsm_Raw_Buffer, grab_len, "+CSQ: ", ',', CSQ, sizeof(CSQ));
#if LOGLEVEL
          printf(("CSQ: %s\r\n"), CSQ);
#endif
        }
        else if (stage_step == 2)    //process jump of APN is already set
        {
          if (strstr((char*)gsm_Raw_Buffer, APN[net]) != NULL)
            stage_jump = 1;
        }
        else if (stage_step == 5) //process grab assigned IP
        {
          gsm_data_grabber((char*)gsm_Raw_Buffer, grab_len, "\n", '\n', IP, sizeof(IP));
          gprs_Is_available = true;
#if LOGLEVEL
          printf(("IP: %s\r\n"), IP);
#endif
        }
      }
      break;
    case _NTP_SERVER: // module graps GPRS time
      if ((resp = GSM_str_is_valid(ntp_server[stage_step].resp_str)))
      {
#ifdef debug
        printf(("GSM response is valid for stage_step: %d:%d\r\n"), gsm_stage, stage_step);
#endif
        if (stage_step == 1)
        {
          stage_jump = 1;
        }
        else if (stage_step == 5)
        {
          memset(ttimeBuf, 0, sizeof(ttimeBuf));
          tLen = gsm_data_grabber((char*)gsm_Raw_Buffer, grab_len, "+CCLK: \"", '+', ttimeBuf, sizeof(ttimeBuf));
          gprs_time_found = true;

        }
      }
      else
      {
        if (stage_step == 1 || stage_step == 4)
          resp = true;
        else if (stage_step == 5)
        {
          stage_step = 0;
        }
      }
      break;

    case _INIT_TCP: //init tcp communication
      if ((resp = GSM_str_is_valid(init_tcp[stage_step].resp_str)))
      {
#ifdef debug
        printf(("GSM response is valid for stage_step: %d:%d\r\n"), gsm_stage, stage_step);
#endif
        //process of grabbing useful data
        if (stage_step == 1)    //set tcp_alive true
        {
          tcp_alive = true;
          tcp_alive_timer = HAL_GetTick() + MAX_ALIVE_TIME;

#if LOGLEVEL
          printf(("tcp is opened!\r\n"));
#endif
        }
      }
      else
      {
        if (stage_step == 0)    //set tcp_alive true
        {
          if (strstr((char*)gsm_Raw_Buffer, "ALREADY CONNECT") != NULL)
          {
            tcp_alive = true;
            tcp_alive_timer = HAL_GetTick() + MAX_ALIVE_TIME;
#if LOGLEVEL
            printf(("OK, tcp connection exists\r\n"));
#endif
            stage_jump = 1;
          }
          else
          {
            tcp_alive = false;

          }
          resp = true;
        }
      }
      break;

    case _NET_STAT: //get network status info
      if ((resp = GSM_str_is_valid(net_stat[stage_step].resp_str)))
      {
#ifdef debug
        printf(("GSM response is valid for stage_step: %d:%d\r\n"), gsm_stage, stage_step);
#endif
        if (stage_step == 0)
        {
          char tem_bb[32] = "";
          gsm_data_grabber((char*)gsm_Raw_Buffer, grab_len, ",\"", ',', MCC, sizeof(MCC));
          strcat(tem_bb, MCC);
          strcat(tem_bb, ",");
          gsm_data_grabber((char*)gsm_Raw_Buffer, grab_len, tem_bb, ',', MNC, sizeof(MNC));
          memset(tem_bb, 0, sizeof(tem_bb));
          strcat(tem_bb, MNC);
          strcat(tem_bb, ",");
          gsm_data_grabber((char*)gsm_Raw_Buffer, grab_len, tem_bb, ',', LAC, sizeof(LAC));
          memset(tem_bb, 0, sizeof(tem_bb));
          strcat(tem_bb, LAC);
          strcat(tem_bb, ",");
          gsm_data_grabber((char*)gsm_Raw_Buffer, grab_len, tem_bb, ',', CID, sizeof(CID));

#if LOGLEVEL
          printf(("MCC: %s\r\n"), MCC);
          printf(("MNC: %s\r\n"), MNC);
          printf(("LAC: %s\r\n"), LAC);
          printf(("CID: %s\r\n"), CID);

#endif
        }
      }
      else
      {
        if (stage_step == 0)
        {
          resp = true;
        }
      }
      break;
    case _SEND_DATA:    //send
      if ((resp = GSM_str_is_valid(send_data[stage_step].resp_str)))
      {
#ifdef debug
        printf(("GSM response is valid for stage_step: %d:%d\r\n"), gsm_stage, stage_step);
#endif
        if (stage_step == 2)
        {
          tcp_alive_timer = HAL_GetTick() + MAX_ALIVE_TIME;
          dataAck = true;
          //stage_jump = 1;
        }
      }
      else
      {/*
        if (stage_step == 0)
        {*/
#ifdef debug
          printf(("we're unsure why this happened, at %d gsm step stage\r\n"), stage_step);
#endif
          //network_failed_restart();
          //retry = true;
          resp = true;
        /*}
        else if(stage_step == 2)
        {
        	resp = true;
        }*/
      }
      break;

    default:
#ifdef debug
      printf(("gsm_stage: %d\r\n"), gsm_stage);
#endif
      break;
  }

  if (resp)
  {
    if (stage_step < (GSM_MAX_STAGE_STEP[gsm_stage] - 1))
    {
      stage_step = stage_step + stage_jump + 1;
      stage_jump = 0;
    }
    else if (stage_step >= (GSM_MAX_STAGE_STEP[gsm_stage] - 1))
    {
      stage_step = 0;
      if (gsm_stage < MAX_GSM_STAGE - 1) //prevent process from going to the last stage, which is 'close net'
        gsm_stage++;
    }
  }
}

void setUp_GSM(void)
{
	init_apn_str();
	init_GSM_str();
	gsm_operation_delay = 0;
	//gsm_operation_delay = HAL_GetTick() + GSM_DELAY;
	tcp_alive_timer = HAL_GetTick() + MAX_ALIVE_TIME + 120000;
}

u16 process_GSM_Frames(const u8 *str, u16 str_len, char* resp, u16 respLen)
{
  if (str_len <= 0) return 0;
  u16 rLen = 0;
  if ((strstr((char*)str, "+PDP DEACT") != NULL) || (strstr((char*)str, "CLOSED") != NULL))
  {
#ifdef debug
    printf(("TCP closed\r\n"));
#endif
    retry = true;
  }
  else if (strstr((char*)str, "*PSUTTZ") != NULL)
  {
    memset(ttimeBuf, 0, sizeof(ttimeBuf));
    tLen = gsm_data_grabber((char*)gsm_Raw_Buffer, grab_len, "*PSUTTZ: ", '+', ttimeBuf, sizeof(ttimeBuf));
    gprs_time_found = true;
  }
  else if ((strstr((char*)str, server_start_str) != NULL) && (strstr((char*)str, end_str) != NULL) && (strstr((char*)str, IMEI) != NULL))
  {
	  rLen = gsm_data_grabber((char*)str, str_len, server_start_str, '#', resp, respLen);
	  send_tmr = HAL_GetTick();
#ifdef debug
    printf(("Valid server data! -> %s\r\n"), resp);
#endif

  }

  return rLen;
}

void GSM_Process(void)
{
	if (tmr_not_expired(HAL_GetTick(), gsm_operation_delay))
	{
		return;
	}

	GSM_Send_Handler();

	if ((grab_len =  rec_GSM_str()) > 0)
	{
#ifdef debug
		printf(("GSM recv: %s:%d\r\n"), gsm_Raw_Buffer, grab_len);
#endif
		GSM_Response_Handler();
	    if (inputLen == 0)
	    {
	    	memset(userInput, 0, sizeof(userInput));
	    	inputLen = process_GSM_Frames(gsm_Raw_Buffer, grab_len, userInput, sizeof(userInput));
	    }
	    	memset(gsm_Raw_Buffer, 0, sizeof(gsm_Raw_Buffer));
		grab_len = 0;
	}
/*
#ifdef debug
    if (inputLen == 0)
    {
    	char test_buf[MAX_LEN] = "";
    	u16 testLen = 0;
    	testLen = rec_str(test_buf, MAX_LEN);
    	inputLen = process_GSM_Frames(test_buf, testLen, userInput, sizeof(userInput));
    }
#endif
*/
	sendPacket_Handler();
	receivePacket_Handler();
	connectivity_watchdog();
}
