#include "WIFIAP.h"
//#include "MYHTTP.h"
#include "MYMQTT.h"

WIFIAP wifiap;
//MYHTTP myHttp;
MYMQTT myMqtt;

void setup() {
  Serial.begin(9600);
  Serial.println("setup");
  //wifiap.clearNVS();
  wifiap.initMyWifi();
  // myMqtt.initMqtt();
}

void loop() {
  wifiap.loopMyWifi();
  //http get 链接
  //myHttp.getData("http://example.com/index.html");
  //mqtt链接
  // myMqtt.loopMqtt();
}
