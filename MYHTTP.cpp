#include "HardwareSerial.h"
#include "WString.h"
#include "MYHTTP.h"
#include <Arduino.h>
#include <HTTPClient.h>

MYHTTP::MYHTTP() {
}

MYHTTP::~MYHTTP() {
  Serial.println("MYHTTP finish init code.");
}

//初始化
String MYHTTP::getData(String url) {
  HTTPClient http;

  Serial.print("[HTTP] begin...\n");

  // configure server and url
  http.begin(url);
  //http.begin("192.168.1.12", 80, "/test.html");

  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {

      // get length of document (is -1 when Server sends no Content-Length header)
      int len = http.getSize();

      // create buffer for read
      uint8_t buff[128] = { 0 };

      // get tcp stream
      NetworkClient *stream = http.getStreamPtr();

      // read all data from server
      while (http.connected() && (len > 0 || len == -1)) {
        // get available data size
        size_t size = stream->available();

        if (size) {
          // read up to 128 byte
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

          // write it to Serial
          Serial.write(buff, c);

          if (len > 0) {
            len -= c;
          }
        }
        delay(1);
      }

      Serial.println("[HTTP] connection closed or file end.\n");
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  return "ddddddd";
}