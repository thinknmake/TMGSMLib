/*
 * File:   TMGSM_Demo.ino
 * Author: Nilesh Mundphan
 * Created on March 04, 2021, 11:03 PM
 * Private Use Only
 */

#include "TMGSMCore.h"

//For Uno, Nano Software Serial 
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(3, 2); // RX, TX

#define SerialAT Serial2

TMGSMCore gsm(SerialAT,13,true);//(Serial,RST_PIN,Debug)

#define DEBUGSerialPort Serial

void setup(){
    DEBUGBegin();
    delay(1000);
    SerialAT.begin(9600);
    DEBUGLog("Started System");
    DEBUGLog("GSM Basic Test \n");
    DEBUGLog("Code By Nilesh Mundphan")
    if(!gsm.begin()){
        DEBUGLog("Check GSM");
        while(1){}
    }

    DEBUGLog(String("Battery  : ")+String(gsm.battery()));
    DEBUGLog(String("SIM Card : ")+String(gsm.sim()));
    DEBUGLog(String("Signal   : ")+String(gsm.signal()));
    DEBUGLog("Sending SMS");
    gsm.sms_send("+91xxxxxxxxxxxx","Think n Make SMS Test");
}

void loop(){
    gsm.gloop();
    if(gsm.sms_id){
        char mobile_number[16],sms[32];
        gsm.sms_read(mobile_number,sms);
        DEBUGLog(String("Mobile Number : ")+String(mobile_number));
        DEBUGLog(String("Text SMS      : ")+String(sms));
        gsm.sms_send(mobile_number,"Thanks For Your Response !");
    }
    delay(10);
}

void DEBUGBegin(){
    DEBUGSerialPort.begin(115200);
}

void DEBUGLog(String dstr){
    DEBUGSerialPort.println(dstr);
}
