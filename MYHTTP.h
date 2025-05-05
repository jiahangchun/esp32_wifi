/*******************
MYHTTP.h
*******************/

#ifndef _MYHTTP_H__
#define _MYHTTP_H__

//导入Arduino核心头文件
#include "Arduino.h"

//参考文档
//https://blog.csdn.net/Naisu_kun/article/details/115549028

class MYHTTP {
private:
public:
  MYHTTP();                    //构造函数
  ~MYHTTP();                   //析构函数
  String getData(String url);  //
};
#endif
