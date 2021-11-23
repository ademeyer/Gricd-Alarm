/*
 * gsm_string.c
 *
 *  Created on: Nov 6, 2021
 *      Author: Laplace
 */

#include "gsm_string.h"

const char *Glo 			= "gloflat";
const char *MTN 			= "web.gprs.mtnnigeria.net";
const char *Airtel 			= "internet.ng.airtel.com";
const char *NineMobile 		= "9mobile";
const char *Flolive	 		= "flolive.net";
const char *JTIOT 			= "JTM2M";

const char *APN[6];

const char  NETWORK_ID[5][2][8]             =
{
  { {"MTN"},      {"62130"} },
  { {"GLO"},      {"62150"} },
  { {"AIRTEL"},   {"Econet"} },
  { {"9Mobile"},  {"62160"} },
  { {"UNKOWN"},   {""} },
};

_gsm_process 						_GSM_PROCESS;
const u16                           GSM_MAX_STAGE_STEP[MAX_GSM_STAGE] 	= {DETECT_GSM_NO, INIT_GPRS_NO, NTP_SERVER_NO, INIT_TCP_NO, NET_STAT_NO, SEND_DATA_NO, SEND_DATA_NO, CLOSE_NET_NO};
u8                                	gsm_Raw_Buffer[MAX_LEN];
char                                custom_apn[32]                		=         "";
char                                server[32]                    		=         "104.131.53.102";
char                                host[8]                       		=         "2222";
char                                RES_BUFF[MAX_LEN]             		=         "";
char                                MCC[8]                        		=         "";
char                                MNC[8]                        		=         "";
char                                LAC[8]                        		=         "";
char                                CID[8]                        		=         "";
char                                IMEI[32]                      		=         "";
char                                ICCID[32]                     		=         "";
char                                CSQ[8]                        		=         "";
char                                IP[32]                        		=         "";
char                                GRD[32]                       		=         "GRD000000";
char								ttimeBuf[32]				  		=			"";
char								userInput[64]				  		=         "";
s8                                  net                           		=         -1;
s8                                  apn_net                       		=         -1;
u8                                  stage_step                    		=         0;
u8                                  stage_jump                    		=         0;
u8                                  tLen                          		=         0;
u16									inputLen				      		=         0;
u16                                 grab_len                      		=         0;
u16									Res_Len						  		=		  0;
u32                                 gsm_response_tmr              		=         0;
u32                                 gsm_operation_delay           		=         0;
u32                                 tcp_alive_timer               		=         0;
u32                                 routine_id                    		=         60000;
u8                                  gsm_stage                     		=         0;

bool                                tcp_alive                     		=         false;
bool                                gprs_Is_available             		=         false;
bool                                Isroaming                     		=         false;
bool                                gprs_time_found               		=         false;
bool								dataAck						  		=		  false;
bool								retry								= 		  false;

gsm_parameter detect_gsm    [DETECT_GSM_NO];
gsm_parameter init_gprs     [INIT_GPRS_NO];
gsm_parameter ntp_server    [NTP_SERVER_NO];
gsm_parameter init_tcp      [INIT_TCP_NO];
gsm_parameter net_stat      [NET_STAT_NO];
gsm_parameter send_data     [SEND_DATA_NO];
gsm_parameter close_net     [CLOSE_NET_NO];



void init_apn_str(void)
{
	APN[0] = Glo;
	APN[1] = MTN;
	APN[2] = Airtel;
	APN[3] = NineMobile;
	APN[4] = Flolive;
	APN[5] = JTIOT;
}

void init_GSM_str(void)
{
	//0 = creating the detect gsm string
	detect_gsm[0].query_str = "AT"; 					detect_gsm[0].resp_str = "OK"; 				detect_gsm[0].wait_tmr = 200;	                //0
	detect_gsm[1].query_str = "AT+CFUN= 1,1"; 			detect_gsm[1].resp_str = "OK"; 				detect_gsm[1].wait_tmr = 25000; 	            //1
	detect_gsm[2].query_str = "AT+GSN"; 				detect_gsm[2].resp_str = "OK"; 				detect_gsm[2].wait_tmr = 500; 					//2 = grab IMEI
	detect_gsm[3].query_str = "AT+CCID"; 				detect_gsm[3].resp_str = "OK"; 				detect_gsm[3].wait_tmr = 2000;                  //3 = grab ICCID
	detect_gsm[4].query_str = "AT+CENG=3"; 				detect_gsm[4].resp_str = "OK"; 				detect_gsm[4].wait_tmr = 500;                   //4
	detect_gsm[5].query_str = "ATE0"; 					detect_gsm[5].resp_str = "OK"; 				detect_gsm[5].wait_tmr = 200;                   //5
	detect_gsm[6].query_str = "AT+CNMI=2,2,0,0,0"; 		detect_gsm[6].resp_str = "OK"; 				detect_gsm[6].wait_tmr = 500;                   //6
	detect_gsm[7].query_str = "AT+CMGF=1"; 				detect_gsm[7].resp_str = "OK"; 				detect_gsm[7].wait_tmr = 500;                   //7
	detect_gsm[8].query_str = "AT+CGREG?"; 				detect_gsm[8].resp_str = "OK"; 				detect_gsm[8].wait_tmr = 500;                   //8 = process roaming network affirmation
	detect_gsm[9].query_str = "AT+COPS?"; 				detect_gsm[9].resp_str = "+COPS: 0,0,"; 	detect_gsm[9].wait_tmr = 1000;                  //9 = process APN ID
	detect_gsm[10].query_str = "AT+CSPN?"; 				detect_gsm[10].resp_str = "OK"; 			detect_gsm[10].wait_tmr = 1000;                 //10 = reaffirm a roaming network, re-process a apn

	//1 = creating the init gprs string
	init_gprs[0].query_str = "AT+CSQ";  						init_gprs[0].resp_str = "OK"; 		init_gprs[0].wait_tmr = 200;                    //0 = grab network signal strength
	init_gprs[1].query_str = "AT+CGATT=1";  					init_gprs[1].resp_str = "OK"; 		init_gprs[1].wait_tmr = 200;                    //1
	init_gprs[2].query_str = "AT+CSTT?";  						init_gprs[2].resp_str = "OK"; 		init_gprs[2].wait_tmr = 300;                    //2 = process response for possible jump of APN settings
	init_gprs[3].query_str = "AT+CSTT = \"{0}\", \"\", \"\"";  	init_gprs[3].resp_str = "OK"; 		init_gprs[3].wait_tmr = 300;           			//3 = process sending for inserting APN settings
	init_gprs[4].query_str = "AT+CIICR";  						init_gprs[4].resp_str = "OK"; 		init_gprs[4].wait_tmr = 3000;                   //4 = bring up GPRS
	init_gprs[5].query_str = "AT+CIFSR";  						init_gprs[5].resp_str = "."; 		init_gprs[5].wait_tmr = 1500;                   //5 = grab assigned network IP

	//2 = creating the ntp server string
	ntp_server[0].query_str = "AT+CNTPCID=1";  					ntp_server[0].resp_str = "OK"; 					ntp_server[0].wait_tmr = 1000;      //0
	ntp_server[1].query_str = "AT+CNTP?";  						ntp_server[1].resp_str = "202.120.2.101,4"; 	ntp_server[1].wait_tmr = 1000;      //1 = process response for possible jump of ntp server settings
	ntp_server[2].query_str = "AT+CNTP=\"202.120.2.101\",4";  	ntp_server[2].resp_str = "OK"; 					ntp_server[2].wait_tmr = 1000;		//2
	ntp_server[3].query_str = "AT+CNTPCID=1";  					ntp_server[3].resp_str = "OK"; 					ntp_server[3].wait_tmr = 1000;      //3
	ntp_server[4].query_str = "AT+CNTP";  						ntp_server[4].resp_str = "+CNTP: 1"; 			ntp_server[4].wait_tmr = 10000;     //4
	ntp_server[5].query_str = "AT+CCLK?";  						ntp_server[5].resp_str = "+CCLK: "; 			ntp_server[5].wait_tmr = 1000;      //5 = process time stamp

	//3 = creating the init tcp string
	init_tcp[0].query_str = "AT+CIPSTART=\"TCP\",\"{0}\",\"{1}\"";  init_tcp[0].resp_str = "OK"; 			init_tcp[0].wait_tmr = 10000;    		//0 = process sending for inserting server details
	init_tcp[1].query_str = "AT+CIPSTATUS";  						init_tcp[1].resp_str = "CONNECT OK"; 	init_tcp[1].wait_tmr = 500;             //1 = process connected status

	//4 = creating the net stat string
	net_stat[0].query_str = "AT+CENG?";  net_stat[0].resp_str = "+CENG:"; 			net_stat[0].wait_tmr = 1000;                           			//0 = process response to grab LAC, CELLID, MCC and MNC

	//5 = creating the send data string
	send_data[0].query_str = "AT+CIPSEND"; 		send_data[0].resp_str = ">"; 			send_data[0].wait_tmr = 1000;                            	//0
	send_data[1].query_str = "{0}";  			send_data[1].resp_str = ""; 			send_data[1].wait_tmr = 500;                            	//1 = process sending stage to insert message format
	send_data[2].query_str = "";  				send_data[2].resp_str = "SEND OK"; 		send_data[2].wait_tmr = 2000;                               //2 = process response for successful sent data
	//send_data[3].query_str = "AT+SHUT";  		send_data[3].resp_str = "OK"; 			send_data[3].wait_tmr = 500;                            	//1 = process sending stage to insert message format

	//6 = creating the close net string
	close_net[0].query_str = "AT+SHUT"; 		close_net[0].resp_str = "OK"; 			close_net[0].wait_tmr = 500;                             	//0

}

s8 id_network_name(const char* mccmnc)
{
  s8 net_id = -1;
  for (int i = 0; i < MAX_APN_DEFINED; i++)
  {
    if (strstr(mccmnc, NETWORK_ID[i][0]) != NULL || strstr(mccmnc, NETWORK_ID[i][1]) != NULL)
    {
      net_id = i;
      break;
    }
    else
      net_id = -1;
  }

  return net_id;
}

void network_failed_restart(void)
{
  gsm_stage = 0;
  stage_step = 0;
  tcp_alive = false;
  gsm_pwr_cnt = 0;
  gsm_pwr_tmr = 1;
  //gprs_Is_available = false;
}


