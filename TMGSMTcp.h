/*
 * File:   TMGSMTcp.h
 * Author: Nilesh Mundphan
 * Created on March 04, 2021, 11:03 PM
 * Private Use Only
 */
#ifndef TMGSMTcp_h
#define TMGSMTcp_h

#include <Arduino.h>
#include "Client.h"
#include "TMGSMCore.h"

#define GPRS_TIMEOUT     	-1
#define GPRS_CONECTED 		1
#define GPRS_DISCONECTED	0
#define GPRS_CMD_ERROR	 	-2
#define GPRS_DEBUG			1

class TMGSMTcp : public Client ,public TMGSMCore{
	public:
		TMGSMTcp(Stream & gsmSerial,uint8_t rst_pin,boolean dgb=false);

		int connect(const char *host, uint16_t port);
		int connect(IPAddress ip, uint16_t port);
		size_t write(uint8_t);
		size_t write(const uint8_t *buf, size_t size);
		int available();
		int read();
		int read(uint8_t *buf, size_t size);
		int peek();
		void flush();
		void stop();
		uint8_t connected();
		operator bool();
		uint8_t _state;

		int getbufflen();
		void addBuff(int rxData);
		void fillbuffer(int len);

		void    GPRSSetup();
		boolean GConnected();
		boolean isGPRSConnected;
		unsigned long GSMRes_time   = 0,  GSMReq_time   = 0;
		unsigned reset_counter=0;
};
#endif
