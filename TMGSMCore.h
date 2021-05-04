/*
 * File:   TMGSMCore.h
 * Author: Nilesh Mundphan
 * Created on March 04, 2021, 11:03 PM
 * Private Use Only
 */
#ifndef EMGSMLIB_H
#define EMGSMLIB_H

#include "Arduino.h"

#define DEBUGSerial   Serial

#define AT_OK           "OK"

#define HTTP_OK       	"200"
#define HTTP_BUSY     	"604"
#define HTTP_NERR     	"601"

#define GSMRes_timeout  25000
#define GSM_BUFFER 		256

class TMGSMCore
{
	public:
				TMGSMCore(Stream & stream,uint8_t rst_pin,boolean dgb=false);
		void	DEBUGLog(String debstr);

		void 	parseGSMRes(byte b);
		void 	gloop();
		void    read_response(int multiline=0,int timeout=5);
		boolean GSMCmd(const char *at_cmd,int multiline=0,int timeout=5);
		boolean begin();
		boolean reset();
		boolean pwr_key();
		String  model();
		String  battery();
		uint8_t battery_percentage();
		String  sim();
		uint8_t	signal();

		void 	GPSInit();
		void 	GPSGetLocation(char *glat,char *glon,char *gspeed,char *gSate);
		void 	GPSStop();

		void 	smshandle(void);
		void    HTTPHandle();

		void 	sms_send(const char *num,const char *msg);
		void    sms_read(char *m_number,char *sms);
		
		Stream & gsmSerial;	
		
		char 	buffer[GSM_BUFFER];
		boolean	sstart   ;
		byte 	pos      ;
		byte 	line     ;
		byte 	res_line ;
		
		boolean	isGSMOK;
		boolean	isGSMResponse;

		uint8_t _reset_pin;
		uint8_t _pwr_pin;
		volatile boolean debug;

		uint8_t  sms_id;
		boolean  ishttp;
		
		unsigned long GSMRes_time   = 0,  GSMReq_time   = 0;
		unsigned long HTTPReq_time  = 0,  HTTPRes_time  = 0;

};

#endif
