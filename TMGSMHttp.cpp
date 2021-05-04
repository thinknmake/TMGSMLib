#include <TMGSMHttp.h>

TMGSMHttp::TMGSMHttp(Stream & gsmSerial,uint8_t rst_pin,boolean dgb): TMGSMCore(gsmSerial,rst_pin,dgb){
      isGPRSConnected=false;
}

//------------------GPRS ------------------------//

void TMGSMHttp::GPRSSetup(){

    DEBUGLog("ECHO OFF");

    if(!GSMCmd("ATE0")){
      return;
    }

    gsmSerial.flush();

    DEBUGLog("CONTYPE GPRS");
    if(!GSMCmd("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"")){
      return;
    }

    DEBUGLog("APN Vodafone");
    if(!GSMCmd("AT+SAPBR=3,1,\"APN\",\"vodafone\"")){
      return;
    }

    DEBUGLog("UP Profile");
    if(!GSMCmd("AT+SAPBR=1,1")){
      return;
    }
}

boolean TMGSMHttp::Connected(){
    char *st,*ip,*end;
    GSMCmd("AT+SAPBR=2,1",2);
    st=strchr(buffer,',');
    st++;
    end=strchr(st,',');
    ip=end+1;
    *end='\0';
    ip=strchr(ip,'"');
    ip++;
    end=strchr(ip,'"');
    *end='\0';
    if(atoi(st)==1){
      DEBUGLog(String("Bearer: ")+String(st));
      DEBUGLog(String("IP    : ")+String(ip));
      isGPRSConnected = true;
      return true;
    }else{
      isGPRSConnected = false;
      return false;
    }
}

//-------------------HTTP Functions -------------------//
void TMGSMHttp::HTTPInit(){
    GSMCmd("AT+HTTPINIT");
    GSMCmd("AT+HTTPPARA=\"CID\",1");
    GSMCmd("AT+HTTPPARA=\"TIMEOUT\",\"30\"");
}

void TMGSMHttp::HTTPEnd(){
    GSMCmd("AT+HTTPTERM");
}

void TMGSMHttp::HTTPSsl(){
    GSMCmd("AT+HTTPSSL=1");
}

void TMGSMHttp::HTTPParaIPPort(char *ip,char *port){
    //GSMCmd("AT+HTTPPARA=\"PROIP\",\"103.67.239.138\"");
    //GSMCmd("AT+HTTPPARA=\"PROPORT\",\"80\"");    
    gsmSerial.print("AT+HTTPPARA=\"PROIP\",\"");
    gsmSerial.print(ip);
    gsmSerial.write('\"');
    gsmSerial.write('\r');
    HTTPReq_time=millis();
    read_response();
    
    gsmSerial.print("AT+HTTPPARA=\"PROPORT\",\"");
    gsmSerial.print(port);
    gsmSerial.write('\"');
    gsmSerial.write('\r');
    HTTPReq_time=millis();
    read_response();
}

void TMGSMHttp::HTTPUrl(const char *url)
{
    gsmSerial.print("AT+HTTPPARA=\"URL\",\"");
    gsmSerial.print(url);
    gsmSerial.write('\"');
    gsmSerial.write('\r');
    HTTPReq_time=millis();
    read_response();
}

void TMGSMHttp::HTTPContent(char *content)
{
  //GSMCmd("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"");
  //GSMCmd("AT+HTTPPARA=\"CONTENT\",\"application/json\"");    
    gsmSerial.print("AT+HTTPPARA=\"CONTENT\",\"");
    gsmSerial.print(content);
    gsmSerial.write('\"');
    gsmSerial.write('\r');
    HTTPReq_time=millis();
    read_response();
}

void TMGSMHttp::HTTPUserdata(char *userdata)
{
    //GSMCmd("AT+HTTPPARA=\"USERDATA\",\"X-AIO-Key:4f74bb2b914844f8a982bb4b9723806b\"");
    gsmSerial.print("AT+HTTPPARA=\"USERDATA\",\"");
    gsmSerial.print(userdata);
    gsmSerial.write('\"');
    gsmSerial.write('\r');
    HTTPReq_time=millis();
    read_response();
}

void TMGSMHttp::HTTPData(String udata,int len,int time_out){
    //GSMCmd("AT+HTTPDATA=64,3000");
    gsmSerial.print("AT+HTTPDATA=");
    gsmSerial.print(len);
    gsmSerial.write(',');
    gsmSerial.print(time_out);
    gsmSerial.write('\r');
    HTTPReq_time=millis();
    read_response();
    gsmSerial.print(udata);
    HTTPReq_time=millis();
    read_response();

}

void TMGSMHttp::HTTPPost(){
    GSMCmd("AT+HTTPACTION=1");
    HTTPReq_time=millis();
}

void TMGSMHttp::HTTPGet(){
    GSMCmd("AT+HTTPACTION=0");
    HTTPReq_time=millis();
}

void TMGSMHttp::HTTPRead(char *hdata){
    char *hd,*e;
    GSMCmd("AT+HTTPREAD",2);
    hd=strchr(buffer,'^');
    hd++;
    e=strchr(hd,'^');
    *e='\0';
    strcpy(hdata,hd);
    ishttp=false;
}

//AT+SAPBR=3,1,"CONTYPE","GPRS"
//AT+SAPBR=3,1,"APN","vodafone"
//AT+SAPBR=1,1
//AT+SAPBR=2,1
//AT+HTTPINIT
//AT+HTTPPARA="CID",1
//AT+HTTPPARA="TIMEOUT","30"
//AT+HTTPPARA="URL","http://embeddedmakers.com/time.php"
//AT+HTTPPARA="URL",http://103.67.239.138/time.php
//AT+HTTPACTION=0
//AT+HTTPREAD
//AT+HTTPTERM
//AT+CDNSGIP=www.embeddedmakers.com