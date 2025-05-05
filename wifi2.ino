#include "WIFIAP.h"
//#include "MYHTTP.h"

WIFIAP wifiap;
//MYHTTP myHttp;

void setup() {
  Serial.begin(9600);
  Serial.println("setup");
  //wifiap.clearNVS();
  wifiap.initMyWifi();
}

void loop() {
  wifiap.loopMyWifi();
  //http get 链接
  //myHttp.getData("http://example.com/index.html");
}
