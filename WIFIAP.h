/*******************
WIFIAP.h
*******************/

#ifndef _WIFIAP_H__
#define _WIFIAP_H__

//导入Arduino核心头文件
#include "Arduino.h"

//参考文档
//https://blog.csdn.net/YANGJIERUN/article/details/129092371

class WIFIAP {
private:
public:
  WIFIAP();           //构造函数
  ~WIFIAP();          //析构函数
  void initMyWifi();  //
  void loopMyWifi();  //按照条件清除，清除旧信息。可能还是有问题
  void clearNVS();  //清除旧信息
};

#endif
