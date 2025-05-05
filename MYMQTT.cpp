#include "MYMQTT.h"
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <SPI.h>
#define JDQ 16

const char* MQTT_SERVER = "iot.eclipse.org";
const int MQTT_PORT = 1883;
const char* MQTT_USRNAME = "admin";
const char* MQTT_PASSWD = "adminadmin";
const char* IN_TOPIC = "home/in/topic/";
const char* OUT_TOPIC = "home/out/topic/";
const char* CLIENT_ID = "scy-mqtt-client";  //当前设备的clientid标志
MYMQTT::MYMQTT() {
}

MYMQTT::~MYMQTT() {
  Serial.println("MYMQTT finish init code.");
}

//回调函数
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}



WiFiClient espClient;
PubSubClient client(MQTT_SERVER, MQTT_PORT, callback, espClient);


void reconnect() {
  // Note - the default maximum packet size is 128 bytes. If the
  // combined length of clientId, username and password exceed this use the
  // following to increase the buffer size:
  // client.setBufferSize(255);
  while (!client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD)) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(CLIENT_ID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(OUT_TOPIC, "hello world");
      // ... and resubscribe
      client.subscribe(IN_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void MYMQTT::initMqtt() {
  reconnect();
}

void MYMQTT::loopMqtt() {
  client.loop();
}

void MYMQTT::setCallBack() {
}
