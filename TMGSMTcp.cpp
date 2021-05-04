#include "TMGSMTcp.h"

#define AT_SEND_OK      "SEND OK"
#define AT_CONNECT_OK   "CONNECT OK"

#define RXBUFFERMAX 128
volatile uint8_t buffer_len=0;
volatile uint8_t read_len=0;

volatile uint8_t rxBuffer[RXBUFFERMAX];
volatile uint8_t rx_Buffer_Head=-1,rx_Buffer_Tail=-1;

TMGSMTcp::TMGSMTcp(Stream & gsmSerial,uint8_t rst_pin,boolean dgb):TMGSMCore(gsmSerial,rst_pin,dgb){
    reset_counter=0;
    isGPRSConnected =false;
	rx_Buffer_Head=-1,rx_Buffer_Tail=-1;
}

int TMGSMTcp::getbufflen(){
	
	if((buffer_len - read_len) > 0){
		return (buffer_len - read_len);	
	}
	else {
		buffer_len=0;
		read_len=0;
		return 0;
	}
	/*
    if(rx_Buffer_Head < 0 ){
		Serial.println("Head at -1");
        return 0;
    }
    else if(rx_Buffer_Head < rx_Buffer_Tail){
        return(rx_Buffer_Tail - rx_Buffer_Head + 1);
    }
    else if(rx_Buffer_Head > rx_Buffer_Tail){
        return (RXBUFFERMAX - rx_Buffer_Head + rx_Buffer_Tail + 1);
    }
    else{
		Serial.println("Else Condition");
        return 0;
    }
	*/
}

void TMGSMTcp::addBuff(int rxData){
    if((rx_Buffer_Tail == RXBUFFERMAX-1 && (rx_Buffer_Head == 0)) || ((rx_Buffer_Tail + 1) == rx_Buffer_Head))
    {
        rx_Buffer_Head++;
    }

    if(rx_Buffer_Head == RXBUFFERMAX || rx_Buffer_Head == -1){
        rx_Buffer_Head=0;
    }

    if(rx_Buffer_Tail == RXBUFFERMAX - 1){
        rx_Buffer_Tail=0;
    }
    else{
        rx_Buffer_Tail++;
    }

    rxBuffer[rx_Buffer_Tail]=rxData;
}

void TMGSMTcp::GPRSSetup(){
    DEBUGLog("GPRS Setup");
    GSMCmd("AT+SAPBR=0,1");
    GSMCmd("AT+CIPSHUT");
    GSMCmd("AT+SAPBR=3,1,\"APN\",\"vodafone\"");
    GSMCmd("AT+SAPBR=1,1");
    GSMCmd("AT+CSTT=\"vodafone\",\"\",\"\"");
    GSMCmd("AT+CIICR");
    GSMCmd("AT+CIFSR");
    isGPRSConnected = true;
}

boolean TMGSMTcp::GConnected(){
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
    //DEBUGSerial.println(String("Bearer: ")+String(st));
    //DEBUGSerial.println(String("IP    : ")+String(ip));
    if(atoi(st)==1){
        isGPRSConnected = true;
        return true;
    }else{
        isGPRSConnected = false;
        return false;
    }
}

int TMGSMTcp::connect(const char *host, uint16_t port)
{
    while(!GConnected()){
        GPRSSetup();
    }

    GSMCmd("AT+CIPSHUT");
    GSMCmd("AT+CIPMUX=0");
    GSMCmd("AT+CIPRXGET=1");

    gsmSerial.print("AT+CIPSTART=\"TCP\",\"");
    gsmSerial.print(host);
    gsmSerial.print("\"");
    gsmSerial.print(",\"");
    gsmSerial.print(port);
    gsmSerial.print("\"");
    gsmSerial.write('\r');
    read_response(2);

    if(strstr(buffer,"CONNECT OK")){
        _state=GPRS_CONECTED;
    }
    else{
        _state=GPRS_DISCONECTED;
    }

    return _state;
}

size_t TMGSMTcp::write(const uint8_t *buf, size_t size)
{

  if(!GConnected()){
    GPRSSetup();
    return 0;
  }

  GSMReq_time=millis();
  gsmSerial.print("AT+CIPSEND=");
	gsmSerial.println(size);

  isGSMOK=true;
  isGSMResponse=false;
  while(isGSMResponse==false){
      unsigned long now = millis();
      if(now-GSMReq_time > GSMRes_timeout){
          DEBUGSerial.println("GSMRes_timeout");
          isGSMOK=false;
          break;
      }
      if(gsmSerial.read()=='>'){
        isGSMResponse=true;
        break;
      }
      delay(1);
  }

  GSMRes_time=millis();
  //DEBUGSerial.print("--> Send Time Taken: ");DEBUGSerial.println(GSMRes_time-GSMReq_time);
  //delay(10);
  gsmSerial.write(buf, size);
  read_response(0,20);

  if(!isGSMOK){
      begin();
      GPRSSetup();
      _state=GPRS_DISCONECTED;
      return 0;
  }

  if(strstr(buffer, "SEND OK"))
  {
      return size;
  }
  else{
      DEBUGLog("BUFFER: "+String(buffer));
      return 0;
  }
}

void TMGSMTcp::fillbuffer(int len){
    char *k1;
    int i=0;
    gsmSerial.print("AT+CIPRXGET=2,");
    gsmSerial.print(len);
    gsmSerial.write('\r');
    read_response(2);
    k1=strchr(buffer,'^');
    k1++;
    buffer_len=len;
    while(i < len){
        //addBuff(k1[i]);
        rxBuffer[i]=k1[i];
        if(debug){
            DEBUGSerial.print(" ");DEBUGSerial.print(k1[i],HEX);
        }else{
            delay(5);
        }
        i++;
    }
    DEBUGSerial.println();
}

int TMGSMTcp::available()
{
    int len = getbufflen();
    if(len==0){
        GSMCmd("AT+CIPRXGET=4",2,50);
        if(strstr(buffer,"+CIPRXGET: 4,")){
            char *start,*end;
            int rbufidx=0;
            start=strchr(buffer,',');
            start++;
            end=strchr(start,'^');
            *end='\0';
            rbufidx=atoi(start);
            DEBUGLog(String(start));
            if(rbufidx > 0){
                fillbuffer(rbufidx);
            }
        }
    }

    return len;
}

int TMGSMTcp::read()
{
	 int ch=0;
     if((buffer_len-read_len) > 0)
	 {
		ch=rxBuffer[read_len];
		read_len++;	
		return ch; 
	 }else{
		return 0; 
	 }		 
	  /*
	  int ch;
      if(rx_Buffer_Head == -1)
      {
          return 0;
      }

      ch=rxBuffer[rx_Buffer_Head];

  		if(rx_Buffer_Head == rx_Buffer_Tail)
  		{
  				rx_Buffer_Head = rx_Buffer_Tail = -1;
  		}
  		else
  		{
  			rx_Buffer_Head++;
       	if(rx_Buffer_Head == RXBUFFERMAX)
  			rx_Buffer_Head=0;
  		}
  	return ch;
	*/
}

uint8_t TMGSMTcp::connected()
{
    GSMCmd("AT+CIPSTATUS",2);
    if(strstr(buffer,"CONNECT OK")){
      _state=GPRS_CONECTED;
    }else{
      _state=GPRS_DISCONECTED;
    }
  	return _state;
}

int TMGSMTcp::connect(IPAddress ip, uint16_t port){

}

size_t TMGSMTcp::write(uint8_t)
{

}

int TMGSMTcp::read(uint8_t *buf, size_t size)
{

}

int TMGSMTcp::peek()
{

}

void TMGSMTcp::flush()
{
	buffer_len=0;
	read_len=0;
}

void TMGSMTcp::stop()
{
    //if (! cmd_check("AT+CIPCLOSE", AT_OK,1000))return false;
    //GSMCmd("AT+CIPCLOSE");
    _state=GPRS_DISCONECTED;
}

TMGSMTcp::operator bool(){

}
