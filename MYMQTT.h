/*******************
MYMQTT.h
*******************/

#ifndef _MYMQTT_H__
#define _MYMQTT_H__

//导入Arduino核心头文件
#include "Arduino.h"

//参考文档
//https://blog.csdn.net/Naisu_kun/article/details/115549028

class MYMQTT {
private:
public:
  MYMQTT();            //构造函数
  ~MYMQTT();           //析构函数
  void setCallBack();  //设置回调函数
  void initMqtt();
  void loopMqtt();
};
#endif
