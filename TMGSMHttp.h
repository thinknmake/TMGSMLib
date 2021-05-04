/*
 * File:   TMGSMHttp.h
 * Author: Nilesh Mundphan
 * Created on March 04, 2021, 11:03 PM
 * Private Use Only
 */
#ifndef TMGSMHttp_H
#define TMGSMHttp_H

#include "Arduino.h"
#include "TMGSMCore.h"

class TMGSMHttp : public TMGSMCore
{
	public:
	TMGSMHttp(Stream & gsmSerial,uint8_t rst_pin,boolean dgb=false);

	void    GPRSSetup();
	boolean Connected();

	void    HTTPInit();
	void    HTTPParaIPPort(char *ip,char *port);
	void    HTTPContent(char *content);
	void    HTTPUserdata(char *userdata);
	void    HTTPUrl(const char *url);
	void    HTTPData(String udata,int len,int time_out);
	void    HTTPRead(char *hdata);
	void    HTTPSsl();
	void    HTTPGet();
	void    HTTPPost();
	void    HTTPEnd();

	boolean isGPRSConnected;
};

#endif
