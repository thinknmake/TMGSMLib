/*
 * File:   EMTCP_Demo.ino
 * Author: Nilesh Mundphan
 * Created on March 04, 2021, 11:03 PM
 * Private Use Only
 */
 
#include <TMGSMTcp.h>
#include <PubSubClient.h>

#define SerialAT Serial2

#define MQTT_SERVER     "io.adafruit.com"
#define MQTT_PORT       1883
#define AIO_USERNAME    "username"
#define AIO_KEY         "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define PUB_PATH        "username/feeds/mqtt-test"
#define SUB_PATH        "username/feeds/mqtt-test"

#define STATUS_LED      2
#define UPDATE_TIME     10000
#define DEBUGSerialPort Serial

TMGSMTcp mclient(SerialAT,13,false);//(Serial,RST_PIN,Debug)
PubSubClient client(mclient);

unsigned long now=0,prev=0,id=0,val=0;
uint8_t battery = 0, bsignal  = 0;
char msg[64];

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void connection() {
    if (!client.connected()) {
        Serial.println("Connecting To Server");
        if(client.connect("GPRS_Client_02",AIO_USERNAME,AIO_KEY)) {
            Serial.println("Connected To Server ");
            delay(200);
            client.subscribe(SUB_PATH);
            Serial.println("SUB To Server ");
        }else {
            Serial.println("Fail to Connect Server");
            delay(3000);
        }
    }
    else{
       Serial.print("Connection OK : ");Serial.println(id++);
    }
}

void setup() {
    DEBUGBegin();
    SerialAT.begin(9600);
    delay(2000);
    Serial.println("Starting GSM MQTT");
    DEBUGLog("Started System");
    DEBUGLog("GSM Library MQTT Example");
    DEBUGLog("Coded By Nilesh Mundphan");
    delay(5000);   
    mclient.begin();
    DEBUGLog(String("GSM Modem  : ")+String(mclient.model()));
    DEBUGLog(String("Battery    : ")+String(mclient.battery()));
    DEBUGLog(String("SIM Card   : ")+String(mclient.sim()));
    DEBUGLog(String("Signal     : ")+String(mclient.signal()));

    mclient.GPRSSetup();
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(callback);
    connection();
    pinMode(STATUS_LED,OUTPUT);
}

void loop(){
    client.loop();
    now=millis();
    if( now - prev > UPDATE_TIME){
        prev=millis();
        connection();
        battery=mclient.battery_percentage();
        bsignal=mclient.signal();
        DEBUGLog(String("Battery  : ")+String(battery));
        DEBUGLog(String("Signal   : ")+String(bsignal));
        sprintf(msg,"EM :%d,%d,%d",val,bsignal,battery);
        Serial.println(msg);
        digitalWrite(STATUS_LED,LOW);
        if(client.publish(PUB_PATH, msg))
        {
            Serial.println("Publish OK");
            val++;
            digitalWrite(STATUS_LED,HIGH);
        }
        else{
            Serial.println("Publish Failed");
        }
    }
    delay(100);
}

void DEBUGBegin(){
    DEBUGSerialPort.begin(115200);
}

void DEBUGLog(String dstr){
    DEBUGSerialPort.println(dstr);
}