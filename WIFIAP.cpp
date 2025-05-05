#include "WIFIAP.h"
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
//用于设备域名 MDNS.begin("esp32")
#include <ESPmDNS.h>
//用于esp_wifi_restore() 删除保存的wifi信息
#include <esp_wifi.h>
#include <Preferences.h>
#include <nvs_flash.h>


//设置重置按键引脚,用于删除WiFi信息
const int resetPin = 0;
//WiFi连接超时时间，单位秒
int connectTimeOut_s = 15;

//设置AP热点名称
const char* AP_SSID = "设备配网WiFi";
//设置设备名
const char* HOST_NAME = "MY_ESP32";
//设置AP的IP地址
IPAddress apIP(192, 168, 4, 1);
//用于储存扫描到的WiFi ID
String scanNetworksID = "";

//暂时存储wifi账号密码
String wifi_ssid = "";
//暂时存储wifi账号密码
String wifi_pass = "";

//设置DNS端口号
const byte DNS_PORT = 53;
//设置Web端口号
const int webPort = 80;
//创建dnsServer实例
DNSServer dnsServer;
//开启web服务, 创建TCP SERVER,参数: 端口号,最大连接数
WebServer server(webPort);

// 上下两段HTML代码
String ROOT_HTML_1 = "<!DOCTYPE html><html><head>  <meta charset=\"UTF-8\">  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">  <meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\" />  <title>jiahangchun 登录页面</title>  <style>   #content,.login,.login-card a,.login-card h1,.login-help{text-align:center}body,html{margin:0;padding:0;width:100%;height:100%;display:table}#content{font-family:\'Source Sans Pro\',sans-serif;-webkit-background-size:cover;-moz-background-size:cover;-o-background-size:cover;background-size:cover;display:table-cell;vertical-align:middle}.login-card{padding:40px;width:274px;background-color:#F7F7F7;margin:0 auto 10px;border-radius:20px;box-shadow:8px 8px 15px rgba(0,0,0,.3);overflow:hidden}.login-card h1{font-weight:400;font-size:2.3em;color:#1383c6}.login-card h1 span{color:#f26721}.login-card img{width:70%;height:70%}.login-card input[type=submit]{width:100%;display:block;margin-bottom:10px;position:relative}.login-card input[type=text],input[type=password]{height:44px;font-size:16px;width:100%;margin-bottom:10px;-webkit-appearance:none;background:#fff;border:1px solid #d9d9d9;border-top:1px solid silver;padding:0 8px;box-sizing:border-box;-moz-box-sizing:border-box}.login-card input[type=text]:hover,input[type=password]:hover{border:1px solid #b9b9b9;border-top:1px solid #a0a0a0;-moz-box-shadow:inset 0 1px 2px rgba(0,0,0,.1);-webkit-box-shadow:inset 0 1px 2px rgba(0,0,0,.1);box-shadow:inset 0 1px 2px rgba(0,0,0,.1)}.login{font-size:14px;font-family:Arial,sans-serif;font-weight:700;height:36px;padding:0 8px}.login-submit{-webkit-appearance:none;-moz-appearance:none;appearance:none;border:0;color:#fff;text-shadow:0 1px rgba(0,0,0,.1);background-color:#4d90fe}.login-submit:disabled{opacity:.6}.login-submit:hover{border:0;text-shadow:0 1px rgba(0,0,0,.3);background-color:#357ae8}.login-card a{text-decoration:none;color:#666;font-weight:400;display:inline-block;opacity:.6;transition:opacity ease .5s}.login-card a:hover{opacity:1}.login-help{width:100%;font-size:12px}.list{list-style-type:none;padding:0}.list__item{margin:0 0 .7rem;padding:0}label{display:-webkit-box;display:-webkit-flex;display:-ms-flexbox;display:flex;-webkit-box-align:center;-webkit-align-items:center;-ms-flex-align:center;align-items:center;text-align:left;font-size:14px;}input[type=checkbox]{-webkit-box-flex:0;-webkit-flex:none;-ms-flex:none;flex:none;margin-right:10px;float:left}.error{font-size:14px;font-family:Arial,sans-serif;font-weight:700;height:25px;padding:0 8px;padding-top: 10px; -webkit-appearance:none;-moz-appearance:none;appearance:none;border:0;color:#fff;text-shadow:0 1px rgba(0,0,0,.1);background-color:#ff1215}@media screen and (max-width:450px){.login-card{width:70%!important}.login-card img{width:30%;height:30%}}  </style></head><body style=\"background-color: #e5e9f2\"><div id=\"content\"> <form name=\'input\' action=\'/configwifi\' method=\'POST\'>  <div class=\"login-card\">    <h1>WiFi登录</h1>   <form name=\"login_form\" method=\"post\" action=\"$PORTAL_ACTION$\">   <input type=\"text\" name=\"ssid\" placeholder=\"请输入 WiFi 名称\" id=\"auth_user\" list = \"data-list\"; style=\"border-radius: 10px\">    <datalist id = \"data-list\">";
String ROOT_HTML_2 = "<input type=\"password\" name=\"password\" placeholder=\"请输入 WiFi 密码\" id=\"auth_pass\"; style=\"border-radius: 10px\">      <div class=\"login-help\">        <ul class=\"list\">          <li class=\"list__item\">          </li>        </ul>      </div>   <input type=\"submit\" class=\"login login-submit\" value=\"确 定 连 接\" id=\"login\"; disabled; style=\"border-radius: 15px\"  >    </form> <!-- <form name=\'input\' action=\'/English\' method=\'POST\'>    <input type=\"submit\" class=\"login login-submit\" value=\"English\" id=\"login\"; disabled; style=\"border-radius: 15px\"  >    </form> --></body></html>";

// 方法声明
void initSoftAP();
void initDNS();
void initWebServer();
bool scanWiFi();
void handleRoot();
void handleNotFound();
void handleConfigWifi();
void wifiConfig();

/*
 * 连接WiFi
 */
void connectToWiFi() {
  //设置设备名
  WiFi.hostname(HOST_NAME);
  Serial.println("start connectToWiFi");
  //设置为STA模式并连接WIFI
  WiFi.mode(WIFI_STA);
  //设置自动连接
  WiFi.setAutoReconnect(true);

  //wifi_ssid不为空，意味着从网页读取到wifi
  if (wifi_ssid != "") {
    Serial.println("用web配置信息连接.");
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());  //c_str(),获取该字符串的指针
    wifi_ssid = "";
    wifi_pass = "";
  } else {
    //未从网页读取到wifi
    Serial.println("用nvs保存的信息连接.");
    //begin()不传入参数，默认连接上一次连接成功的wifi
    WiFi.begin();
  }
  //用于连接计时，如果长时间连接不成功，复位设备
  int Connect_time = 0;
  //等待WIFI连接成功
  while (WiFi.status() != WL_CONNECTED) {
    //一共打印30个点点
    Serial.print(".");
    delay(500);
    Connect_time++;

    //长时间连接不上，重新进入配网页面
    if (Connect_time > 2 * connectTimeOut_s) {
      //主要目的是为了换行符
      Serial.println("");
      //开始配网功能
      Serial.println("WIFI autoconnect fail, start AP for webconfig now...");
      wifiConfig();
      //跳出 防止无限初始化
      return;
    }
  }
  //如果连接成功
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WIFI connect Success");
    Serial.printf("SSID:%s", WiFi.SSID().c_str());
    Serial.printf(", PSW:%s\r\n", WiFi.psk().c_str());
    Serial.print("LocalIP:");
    Serial.print(WiFi.localIP());
    Serial.print(" ,GateIP:");
    Serial.println(WiFi.gatewayIP());
    Serial.print("WIFI status is:");
    Serial.print(WiFi.status());
    //停止开发板所建立的网络服务器。
    server.stop();
  }
}


/*
 * 配置配网功能,开启AP功能
 */
void wifiConfig() {
  initSoftAP();
  initDNS();
  initWebServer();
  scanWiFi();
}

/*
 * 进入AP模式
 */
void initSoftAP() {
  WiFi.mode(WIFI_AP);                                          //配置为AP模式
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));  //设置AP热点IP和子网掩码
  if (WiFi.softAP(AP_SSID))                                    //开启AP热点,如需要密码则添加第二个参数
  {
    //打印相关信息
    Serial.println("initSoftAP ESP-32S SoftAP is right.");
    Serial.print("initSoftAP Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());                                             //接入点ip
    Serial.println(String("MAC address = ") + WiFi.softAPmacAddress().c_str());  //接入点mac
  } else {
    //开启AP热点失败
    Serial.println("WiFiAP Failed");
    delay(1000);
    Serial.println("restart now...");
    ESP.restart();  //重启复位esp32
  }
}



/*
 * 开启DNS服务器
 */
void initDNS() {
  if (dnsServer.start(DNS_PORT, "*", apIP))  //判断将所有地址映射到esp32的ip上是否成功
  {
    Serial.println("start dnsserver success.");
  } else {
    Serial.println("start dnsserver failed.");
  }
}



/*
 * 初始化WebServer
 */
void initWebServer() {
  if (MDNS.begin("esp32")) {
    //给设备设定域名esp32,完整的域名是esp32.local
    Serial.println("MDNS responder started");
  }
  //必须添加第二个参数HTTP_GET，以下面这种格式去写，否则无法强制门户
  server.on("/", HTTP_GET, handleRoot);                   //  当浏览器请求服务器根目录(网站首页)时调用自定义函数handleRoot处理，设置主页回调函数，必须添加第二个参数HTTP_GET，否则无法强制门户
  server.on("/configwifi", HTTP_POST, handleConfigWifi);  //  当浏览器请求服务器/configwifi(表单字段)目录时调用自定义函数handleConfigWifi处理
  server.onNotFound(handleNotFound);                      //当浏览器请求的网络资源无法在服务器找到时调用自定义函数handleNotFound处理
  server.begin();                                         //启动TCP SERVER
  Serial.println("WebServer started!");
}


/*
 * 扫描附近的WiFi，为了显示在配网界面
 */
bool scanWiFi() {
  Serial.println("scan start");
  Serial.println("--------->");
  // 扫描附近WiFi
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
    scanNetworksID += "<option>no networks found</option>";
    return false;
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      scanNetworksID += "<option>" + WiFi.SSID(i) + "</option>";
      delay(10);
    }
    scanNetworksID += "</datalist>";
    return true;
  }
}

/*
 * 处理网站根目录的访问请求
 */
void handleRoot() {
  if (server.hasArg("selectSSID")) {
    server.send(200, "text/html", ROOT_HTML_1 + scanNetworksID + ROOT_HTML_2);  //scanNetWprksID是扫描到的wifi
  } else {
    Serial.println("......handleRoot......");
    server.send(200, "text/html", ROOT_HTML_1 + scanNetworksID + ROOT_HTML_2);
  }
}

/*
 * 提交数据后的提示页面
 */
void handleConfigWifi()  //返回http状态
{
  if (server.hasArg("ssid"))  //判断是否有账号参数
  {
    Serial.print("got ssid:");
    wifi_ssid = server.arg("ssid");  //获取html表单输入框name名为"ssid"的内容

    Serial.println(wifi_ssid);
  } else  //没有参数
  {
    Serial.println("error, not found ssid");
    server.send(200, "text/html", "<meta charset='UTF-8'>error, not found ssid");  //返回错误页面
    return;
  }
  //密码与账号同理
  if (server.hasArg("password")) {
    Serial.print("got password:");
    wifi_pass = server.arg("password");  //获取html表单输入框name名为"pwd"的内容
    Serial.println(wifi_pass);
  } else {
    Serial.println("error, not found password");
    server.send(200, "text/html", "<meta charset='UTF-8'>error, not found password");
    return;
  }
  server.send(200, "text/html", "<meta charset='UTF-8'>SSID " + wifi_ssid + "<br />password:" + wifi_pass + "<br />get wifi info ,pls close page");  //返回保存成功页面
  delay(2000);
  WiFi.softAPdisconnect(true);  //参数设置为true，设备将直接关闭接入点模式，即关闭设备所建立的WiFi网络。
  server.close();               //关闭web服务
  WiFi.softAPdisconnect();      //在不输入参数的情况下调用该函数,将关闭接入点模式,并将当前配置的AP热点网络名和密码设置为空值.
  Serial.println("WiFi Connect SSID:" + wifi_ssid + "  PASS:" + wifi_pass);

  if (WiFi.status() != WL_CONNECTED)  //wifi没有连接成功
  {
    Serial.println("start...connectToWiFi()..");
    connectToWiFi();
  } else {
    Serial.println("connectToWiFi sucess use message old..");
  }
}

/*
 * 404情况
 */
void handleNotFound() {
  Serial.println("......handleNotFound......");
  handleRoot();
}



/*
 * 删除保存的wifi信息，这里的删除是删除存储在flash的信息。删除后wifi读不到上次连接的记录，需重新配网
 */
void restoreWiFi() {
  delay(5000);
  esp_wifi_restore();  //删除保存的wifi信息
  Serial.println("clear all message..");
  delay(1000);
}

/*
 * 检查wifi是否已经连接
 */
void checkConnect(bool reConnect) {
  if (WiFi.status() != WL_CONNECTED)  //wifi连接失败
  {
    if (reConnect == true && WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA) {
      Serial.println("WIFI do not connected.");
      Serial.println("WiFi Mode:");
      Serial.println(WiFi.getMode());
      Serial.println("connecting WiFi...");
      connectToWiFi();  //连接wifi函数
    }
  }
}

/*
 * 检测客户端DNS&HTTP请求
 */
void checkDNS_HTTP() {
  dnsServer.processNextRequest();  //检查客户端DNS请求
  server.handleClient();           //检查客户端(浏览器)HTTP请求
}





WIFIAP::WIFIAP() {
}

WIFIAP::~WIFIAP() {
  Serial.println("WIFI finish init code.");
}


//初始化
void WIFIAP::initMyWifi() {
  Serial.println(".....initMyWifi.....");
  delay(5000);
  connectToWiFi();
}


//初始化
void WIFIAP::loopMyWifi() {
  //携带相关条件后，可以清除数据
  // Serial.println("....reConnectMyWifi.....");
  // restoreWiFi();  //删除保存的wifi信息
  // ESP.restart();  //重启复位esp32
  // Serial.println("ESP.restart");
  checkDNS_HTTP();     //检测客户端DNS&HTTP请求，也就是检查配网页面那部分
  checkConnect(true);  //检测网络连接状态，参数true表示如果断开重新连接

  delay(30);
}

// 清除所有数据
void WIFIAP::clearNVS() {
}
