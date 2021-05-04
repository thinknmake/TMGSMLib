/*
 * File:   EMHTTP_Demo.ino
 * Author: Nilesh Mundphan
 * Created on March 04, 2021, 11:03 PM
 * Private Use Only
 */

#include "TMGSMHttp.h"
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(3, 2); // RX, TX

#define SerialAT Serial2

#define DEBUGSerialPort   Serial
#define UPLOAD_LED        2
#define GET_URL           "http://www.embeddedmakers.com/time.php"
#define POST_URL          "http://io.adafruit.com/api/v2/nilesh_mundphan01/feeds/light-detector/data"
#define POST_URL1         "http://www.embeddedmakers.com/em/http_post.php"

#define UPDATE_TIME       10000

unsigned long prev_update = 0;
uint8_t battery=0,signal  = 0;
long upload_index=0;
TMGSMHttp gsm(SerialAT,13,false); //(Serial,RST_PIN,Debug)

void setup(){

    pinMode(UPLOAD_LED,OUTPUT);
    digitalWrite(UPLOAD_LED,HIGH);
    digitalWrite(UPLOAD_LED,LOW);
    
    DEBUGBegin();
    SerialAT.begin(9600);    
    
    delay(2000);
    digitalWrite(UPLOAD_LED,HIGH);

    DEBUGLog("Started System");

    if(!gsm.begin()){
        DEBUGLog("Check GSM");
        while(1){}
    }

    DEBUGLog(String("Battery  : ")+String(gsm.battery()));
    DEBUGLog(String("SIM Card : ")+String(gsm.sim()));
    DEBUGLog(String("Signal   : ")+String(gsm.signal()));

    delay(5000);
    DEBUGLog("GPRS Setup");
    gsm.GPRSSetup();

    while(!gsm.Connected()){
      gsm.GPRSSetup();
      delay(1000);
    }

    DEBUGLog("HTTP Init");
    gsm.HTTPInit();

}

void loop(){
    gsm.gloop();

    unsigned long now = millis();
    if(now - prev_update > UPDATE_TIME){
        prev_update = millis();
        DEBUGLog(String("Battery  : ")+String(gsm.battery_percentage()));
        DEBUGLog(String("Signal   : ")+String(gsm.signal()));

        while(!gsm.Connected()){
            gsm.GPRSSetup();
            DEBUGLog("HTTP END");
            gsm.HTTPEnd();
            DEBUGLog("HTTP INIT");
            gsm.HTTPInit();
            delay(1000);
        }
        HTTP_Get();
        HTTP_Post(upload_index);
        HTTP_Post1(upload_index);
        digitalWrite(UPLOAD_LED,LOW);
        upload_index++;
    }

    if(gsm.ishttp){
        char httpdata[64];
        DEBUGLog("\nReading ............");
        gsm.HTTPRead(httpdata);
        DEBUGLog("HTTP DATA :"+ String(httpdata)+"\n");
        digitalWrite(UPLOAD_LED,HIGH);
    }
}

void DEBUGBegin(){
    DEBUGSerialPort.begin(115200);
    DEBUGLog("GSM GPRS HTTP LIB \n");
    DEBUGLog("Code By Nilesh Mundphan");
}

void DEBUGLog(String dstr){
    DEBUGSerialPort.println(dstr);
}

void HTTP_Get(){
    DEBUGLog("HTTP GET");
    gsm.HTTPUrl(GET_URL);
    gsm.HTTPGet();
}

void HTTP_Post(int idx){
    String sstr = "{\"value\": "+String(idx)+" }";        
    gsm.HTTPEnd();
    gsm.HTTPInit();
    gsm.HTTPUrl(POST_URL);
    gsm.HTTPContent("application/json");
    gsm.HTTPUserdata("X-AIO-Key:xxxxxxxxxxxxxxxxxxxxxxxxxxx");
    gsm.HTTPData(sstr,sstr.length(),3000);
    DEBUGLog("HTTP POST");  //Json Data 
    gsm.HTTPPost();   
}

void HTTP_Post1(int idx){
    String sstr = "id=" + String(idx);        
    gsm.HTTPEnd();
    gsm.HTTPInit();
    //gsm.HTTPParaIPPort("103.67.239.138","80"); If DNS Fails then use IP and port of server
    gsm.HTTPUrl(POST_URL1);
    gsm.HTTPContent("application/x-www-form-urlencoded");
    gsm.HTTPData(sstr,sstr.length(),3000);
    DEBUGLog("HTTP POST 1");//Plan Data   
    gsm.HTTPPost();   
}