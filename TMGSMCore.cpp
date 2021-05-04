#include <TMGSMCore.h>

TMGSMCore::TMGSMCore(Stream & stream,uint8_t rst_pin,boolean dgb) : gsmSerial(stream){
      //this->gsmSerial     = stream;
      this->_reset_pin    = rst_pin;
      this->debug         = dgb;
      this->isGSMResponse = false;
      this->sstart        = false;
      this->isGSMOK       = false;
      this->ishttp        = false;
      this->pos           = 0;
      this->line          = 0;
      this->res_line      = 0;
      this->sms_id        = 0;
      //pinMode(this->_reset_pin, OUTPUT);
      //pinMode(this->_pwr_pin, INPUT_PULLUP);
      //digitalWrite(this->_pwr_pin, LOW);
} 

void TMGSMCore::DEBUGLog(String debstr){
    if(this->debug){
        DEBUGSerial.println(debstr);
    }
}

void TMGSMCore::parseGSMRes(byte b){
    //DEBUGSerialPort.write(b);
    if(b=='\n'){
        if(pos==0){
            memset(buffer, 0, sizeof(buffer));
            pos     = 0;
            line    = 0;
            sstart  = 1;
        }
        else{
            line++;
            buffer[pos++]='^';

            if(line > res_line){
                isGSMResponse=true;
                if(strstr(buffer,"+CMTI:")){
                    smshandle();
                }
                else if(strstr(buffer,"+HTTPACTION")){
                    HTTPHandle();
                }
            }

            if(isGSMResponse){
                res_line  = 0;
                sstart    = 0;
                pos       = 0;
                line      = 0;
            }
        }
    }

    if(sstart){
        if(b!='\r')
            if(b!='\n')
                buffer[pos++]=b;
    }
}

void  TMGSMCore::smshandle(void){
    char *msg_id,*end;
    msg_id=strchr(buffer,',');
    msg_id++;
    end=strchr(msg_id,'^');
    *end='\0';
    sms_id=atoi(msg_id);
}

void  TMGSMCore::HTTPHandle(){
    char *st,*res_len,*end;
    st=strchr(buffer,',');
    st++;
    end=strchr(st,',');
    res_len=end+1;
    *end='\0';
    if(strstr(st,HTTP_OK) != NULL){
        HTTPRes_time=millis();
        DEBUGLog("HTTP OK -->> HTTP Time: "+String(HTTPRes_time-HTTPReq_time));
        ishttp=true;
    }
    else if(strstr(st,HTTP_BUSY) != NULL){
        DEBUGLog("\nHTTP Busy");
    }
    else if(strstr(st,HTTP_NERR) != NULL){
        DEBUGLog("\nNetwork Error");
    }
}

void TMGSMCore::gloop(){
  if(gsmSerial.available()) {
        parseGSMRes(gsmSerial.read());
    }
}

void TMGSMCore::read_response(int multiline,int timeout){

    GSMReq_time=millis();
    res_line=multiline;
    isGSMOK = true;
    isGSMResponse=false;
    while(isGSMResponse==false){
        unsigned long now = millis();
        if(now-GSMReq_time > GSMRes_timeout){
            DEBUGLog("GSMRes_timeout");
            isGSMOK = false;
            break;
        }
        while(gsmSerial.available()) {
            parseGSMRes(gsmSerial.read());
        }
        delay(1);
    }
    delay(timeout);
    while(gsmSerial.available()) {
        gsmSerial.read();
    }
    GSMRes_time=millis();

    DEBUGLog("--> " + String(buffer));
    DEBUGLog("--> Time Taken: "+ String(GSMRes_time-GSMReq_time) );
    res_line=0;
    gsmSerial.flush();
}


boolean TMGSMCore::GSMCmd(const char *at_cmd,int multiline,int timeout){
    gsmSerial.print(at_cmd);
    gsmSerial.write('\r');
    read_response(multiline,timeout);

    if(!isGSMOK){
        DEBUGLog("GSM Not Responding");
        begin();
        return false;
    }


    if(strstr(buffer,"OK")){
        return true;
    }else if(strstr(buffer,"ERROR")){
        return false;
    }
}

//-------------Restart Module------------------//
boolean  TMGSMCore::reset(){
    DEBUGSerial.println("Restart GSM Module");
    digitalWrite(_reset_pin, LOW);
    delay(1000);
    digitalWrite(_reset_pin, HIGH);
    //delay(15000);    
}

boolean  TMGSMCore::pwr_key(){
    DEBUGSerial.println("POWER KEY HIGH");
    digitalWrite(_pwr_pin, HIGH);
    delay(2000);
    DEBUGSerial.println("POWER KEY LOW");
    digitalWrite(_pwr_pin, LOW);
    delay(3000);
    DEBUGSerial.println("POWER DOWN/UP");
    
}

boolean 	TMGSMCore::begin(){
    //gsmSerial.begin(9600);
    DEBUGLog("AT Test");
    if(!GSMCmd("AT")){
        DEBUGLog("GSM Not Responding");
        return false;
    }

    DEBUGLog("ECHO OFF");

    if(!GSMCmd("ATE0")){
        return false;
    }

    DEBUGLog("Text Mode");
    if(!GSMCmd("AT+CMGF=1")){
        return false;
    }
    
    GSMCmd("AT+IPR?"); 

    return true;
}

//---------------- SIM Info ---------------------//

String    TMGSMCore::model(){
    char *start,*end;
    if(GSMCmd("AT+GMM",2)){
          end=strchr(buffer,'^');
          *end='\0';
          return String(buffer);
    }
    return String("NULL");
}

String    TMGSMCore::battery(){
    char *start,*end;
    if(GSMCmd("AT+CBC",2)){
          start=strchr(buffer,',');
          start++;
          start=strchr(start,',');
          start++;
          end=strchr(start,'^');
          *end='\0';
          return String(start);
    }
      return String("NULL");
}

uint8_t TMGSMCore::battery_percentage(){
    char *start,*end;
    if(GSMCmd("AT+CBC",2)){
          start=strchr(buffer,',');
          start++;
          end=strchr(start,',');
          *end='\0';
          return atoi(start);
    }
    return 0;
}

String  TMGSMCore::sim(){
    char *start,*end;
    if(GSMCmd("AT+CSPN?",2)){
          start=strchr(buffer,'"');
          start++;
          end=strchr(start,'"');
          *end='\0';
        return String(start);
    }
    return String("NULL");
}

uint8_t TMGSMCore::signal(){
    char *start,*end;
    if(GSMCmd("AT+CSQ",2)){
        start=strchr(buffer,':');
        start++;
        start++;
        end=strchr(start,',');
        *end='\0';
        return atoi(start);
    }
    return 0;
}

//----------------GPS SIM808 Only--------------------------//

void TMGSMCore::GPSInit(){
    GSMCmd("AT+CGNSPWR=1");
}

void TMGSMCore::GPSStop(){
    GSMCmd("AT+CGNSPWR=0");
}

void TMGSMCore::GPSGetLocation(char *glat,char *glon,char *gspeed,char *gSate){
    char *lat,*lon,*speed,*Sate,*end;
    //GSMCmd("AT+CGNSPWR?",2);
    GSMCmd("AT+CGPSSTATUS?",2);
    //GSMCmd("AT+CGNSINF",2);

    if(strstr(buffer,"Location 3D Fix")){
        GSMCmd("AT+CGNSINF",2);

        lat=strchr(buffer,',');
        lat++;
        lat=strchr(lat,',');
        lat++;
        lat=strchr(lat,',');
        lat++;

        end=strchr(lat,',');
        lon=end+1;
        *end='\0';
        strncpy(glat,lat,strlen(lat));

        end=strchr(lon,',');
        speed=end+1;
        *end='\0';
        strncpy(glon,lon,strlen(lon));

        speed++;
        speed=strchr(speed,',');
        speed++;

        end=strchr(speed,',');
        Sate=end+1;
        *end='\0';
        strcpy(gspeed,speed);
        
        Sate=strchr(Sate,',');
        Sate++;
        Sate=strchr(Sate,',');
        Sate++;
        Sate=strchr(Sate,',');
        Sate++;
        Sate=strchr(Sate,',');
        Sate++;
        Sate=strchr(Sate,',');
        Sate++;
        Sate=strchr(Sate,',');
        Sate++;
        Sate=strchr(Sate,',');
        Sate++;
        Sate=strchr(Sate,',');
        Sate++;

        end=strchr(Sate,',');
        *end='\0';
        
        strcpy(gSate,Sate);
    
        DEBUGLog("Lat: "+String(lat));
        DEBUGLog("Lon: "+String(lon));
        DEBUGLog("Sp: "+String(speed));
        DEBUGLog("Sat: "+String(Sate));
        //sprintf(glat,"%s,%s,%s,%s",Sate,lat,lon,speed);
    }else{
        strcpy(glat,"0.000000");
        strcpy(glon,"0.000000");
        strcpy(gSate,"0");
        strcpy(gspeed,"0");
        DEBUGLog("Location Unknown");
    }
}

void TMGSMCore::sms_send(const char *num,const char *msg){
    gsmSerial.print("AT+CMGS=\"");
    gsmSerial.print(num);
    gsmSerial.print("\"");
    gsmSerial.write('\r');
    GSMReq_time=millis();
    isGSMOK=true;
    isGSMResponse=false;
    while(isGSMResponse==false){
        unsigned long now = millis();
        if(now-GSMReq_time > GSMRes_timeout){
            DEBUGLog("GSMRes_timeout");
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
    DEBUGLog("--> Send Time Taken: "+String(GSMRes_time-GSMReq_time));
    gsmSerial.flush();
    gsmSerial.print(msg);
    gsmSerial.write(0x1A);
}

void  TMGSMCore::sms_read(char *m_number,char *sms){
    char *tsms,*no,*end;
    gsmSerial.print("AT+CMGR=");
    gsmSerial.print(sms_id);
    gsmSerial.write('\r');
    read_response(3);
    no=strchr(buffer,',');
    no++;
    no++;
    end=strchr(no,'"');
    tsms=end+1;
    *end='\0';
    strcpy(m_number,no);
    tsms=strchr(tsms,'^');
    tsms++;
    end=strchr(tsms,'^');
    *end='\0';
    strcpy(sms,tsms);
    sms_id=0;
}