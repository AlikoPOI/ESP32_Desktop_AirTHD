/******************主程序main.cpp******************/
#include <SPI.h> //导入库
#include <TFT_eSPI.h>//屏幕驱动
#include <MyFont.h>//中文字符库
#include <pic.h>//天气图标库
#include <NTPClient.h>//时间服务提供程序
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>//HTTP服务
#include <DHT.h>
#include <PubSubClient.h>//MQTT服务

TFT_eSPI tft = TFT_eSPI(); 

void showMyFont(int32_t x, int32_t y, const char c[3], uint32_t color) { 
    for (int k = 0; k <= 59; k++)// 根据字库的字数调节循环的次数
    if (hanzi[k].Index[0] == c[0] && hanzi[k].Index[1] == c[1] && hanzi[k].Index[2] == c[2])
    { 
        tft.drawBitmap(x, y, hanzi[k].hz_Id, hanzi[k].hz_width, 16, color);
    }
}
  /*******************整句汉字显示****************/
void showMyFonts(int32_t x, int32_t y, const char str[], uint32_t color) { 
    //显示整句汉字，字库比较简单，上下、左右输出是在函数内实现
    int x0 = x;
    for (int i = 0; i < strlen(str); i += 3) {
      showMyFont(x0, y, str+i, color);
      x0 += 17;
    }
}

const char *ssid     = "Redmi K40 for ESP32";//WiFi名称
const char *password = "halo2525";//WiFi密码

const char *host = "api.seniverse.com";//心知天气服务器地址
//用来存储报文得到的字符串
String now_address="",now_temperature="",now_weather="",now_weather_code="";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"ntp.aliyun.com");
//腾讯云MQTT设置
#define mqttServer "XW7LGI4ZMZ.iotcloud.tencentdevices.com"
#define mqttPort 1883
#define ClientId "XW7LGI4ZMZAirTHD"
#define User "XW7LGI4ZMZAirTHD;12010126;GA9K4;5251413323"
#define Pass "cf6b109538b2a70423df25be2947c276843afb3de0e455b2aef9a14e480edbd9;hmacsha256"
#define TOPIC "$thing/up/property/XW7LGI4ZMZ/AirTHD"

WiFiClient espClient;               //创建网络连接客户端
PubSubClient client(espClient);     //创建mqtt客户端

//DHT11引脚设置
#define DHTPIN 0
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);           //DHT实例化

// 声明一个定时器变量
hw_timer_t *timer = NULL;
int interruptCount = 0;
//声明一个定时器标志位
bool timer_flag = false;

//时间信息展示
void Display_Time() {

  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  //打印时间
  int currentSec = timeClient.getSeconds();
  int currentMinute = timeClient.getMinutes();
  int currentHour = timeClient.getHours();
  int weekDay = timeClient.getDay();

  //将epochTime换算成年月日
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;

  tft.fillRect(102,42,22,14,TFT_BLACK);     //部分区域清屏，刷新秒
  //10+2+10=22，“数字”分辨率10*14像素，连续显示时间隔2像素
  tft.setCursor(102, 42, 1);                //设置文本起始坐标
  tft.setTextColor(TFT_RED);                //设置文本颜色为白色
  tft.setTextSize(2);                       //设置文字的大小 (1~7)
  if (currentSec < 10){                     //将 0-9 变为00-09
    tft.println(0);                    
    tft.setCursor(114, 42, 1);              
    tft.setTextColor(TFT_RED);              
    tft.setTextSize(2);                      
    tft.println(currentSec);                     
  }
  else{
    tft.println(currentSec);               //显示文字
  }  


  if (currentSec==0){                       //刷新分
    tft.fillRect(55,28,44,28,TFT_BLACK);    //20+4+20=44 
  }
  tft.setCursor(55, 28, 1);                
  //tft.setTextFont(7);
  tft.setTextColor(TFT_CYAN);             
  tft.setTextSize(4);                     
  if (currentMinute < 10) {
    tft.println(0);                       
    tft.setCursor(79, 28, 1);               
    tft.setTextColor(TFT_CYAN);              
    tft.setTextSize(4);                     
    tft.println(currentMinute);               
  }
  else{
    tft.println(currentMinute);                
  }


  if (currentMinute==0 && currentSec==0){    //刷新时
    tft.fillRect(1,28,44,28,TFT_BLACK); 
  }
  tft.setCursor(1, 28, 1);            
  //tft.setTextFont(7);
  tft.setTextColor(TFT_CYAN);         
  tft.setTextSize(4);                
  if (currentHour < 10) {
    //tft.println(0);
    tft.setCursor(25, 28, 1);   
    tft.setTextColor(TFT_CYAN);     
    tft.setTextSize(4);                      
    tft.println(currentHour);        
  }
  else{
    tft.println(currentHour);        
  }

  tft.setCursor(40, 28, 1);                //时分分隔符
  tft.setTextColor(TFT_CYAN);       
  tft.setTextSize(4);                
  tft.println(":");                     

  tft.setCursor(89, 5, 1);                 //月日分隔符
  tft.setTextColor(TFT_WHITE);             
  tft.setTextSize(2);                     
  tft.println("/");                      

  if (currentHour==0 && currentMinute==0 && currentSec==0){      //刷新 日、周
    tft.fillRect(102,5,22,14,TFT_BLACK); 
    tft.fillRect(5,5,32,16,TFT_BLACK); 
  }
  tft.setCursor(102, 5, 1);                
  tft.setTextColor(TFT_YELLOW);          
  tft.setTextSize(2);                   
  if (monthDay < 10) {
    //tft.println(0);                         //"1_月01日",感觉太奇怪了,还是"1_月_1日"吧！
    tft.setCursor(114, 5, 1);         
    tft.setTextColor(TFT_YELLOW);    
    tft.setTextSize(2);             
    tft.println(monthDay);  
  }
  else {
    tft.println(monthDay);    
  }
  switch(weekDay){
    case 0: showMyFonts(5, 5, "周日", TFT_GREENYELLOW);break; 
    case 1: showMyFonts(5, 5, "周一", TFT_GREENYELLOW);break; 
    case 2: showMyFonts(5, 5, "周二", TFT_GREENYELLOW);break; 
    case 3: showMyFonts(5, 5, "周三", TFT_GREENYELLOW);break; 
    case 4: showMyFonts(5, 5, "周四", TFT_GREENYELLOW);break; 
    case 5: showMyFonts(5, 5, "周五", TFT_GREENYELLOW);break; 
    case 6: showMyFonts(5, 5, "周六", TFT_GREENYELLOW);break; 
    default: break;
  }

  if (monthDay==1 && currentHour==0 && currentMinute==0 && currentSec==0){   //刷新月
    tft.fillRect(65,5,22,14,TFT_BLACK); 
  }
  tft.setCursor(65, 5, 1);                 
  tft.setTextColor(TFT_YELLOW);        
  tft.setTextSize(2);                    
  if (currentMonth <10) {
    //tft.println(0);                        //"_1月_1日"比"1_月_1日"更好一点！
    tft.setCursor(77, 5, 1);               
    tft.setTextColor(TFT_YELLOW);          
    tft.setTextSize(2);                    
    tft.println(currentMonth);
  }
  else {
    tft.println(currentMonth);               
  }


  if (currentMonth==1 && monthDay==1 && currentHour==0 && currentMinute==0 && currentSec==0){  //刷新年
    tft.fillRect(102,28,23,7,TFT_BLACK); 
  }
  tft.setCursor(102, 28, 1);             
  tft.setTextColor(TFT_RED);          
  tft.setTextSize(1);                       
  tft.println(currentYear);               
  //delay(1000);
}

//天气信息展示
void Display_Weather() {
  //创建TCP连接
  WiFiClient client;

  const int httpPort = 80;
  if (!client.connect(host, httpPort))
  {
    Serial.println("connection failed");  //网络请求无响应打印连接失败
    return;
  }

  //URL请求地址 //改为你的api密钥和城市拼音
  String url ="https://api.seniverse.com/v3/weather/now.json?key=SRBeqcto8sgo-_bdh&location=suzhou&language=zh-Hans&unit=c";
  //发送网络请求
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "Connection: close\r\n\r\n");
  //delay(5000);
  for(int i = 0; i < 5; i++) {
    Display_Time();//利用等待 天气服务器响应的时间 更新时间信息
    delay(1000);   //间隔1秒访问ntp,刷新时间戳
  }

  //定义answer变量用来存放请求网络服务器后返回的数据
  String answer;
  while(client.available())
  {
    String line = client.readStringUntil('\r');
    answer += line;
  }
  //断开服务器连接
  client.stop();
  //Serial.println();
  //Serial.println("closing connection");

  //获得json格式的数据
  String jsonAnswer;
  int jsonIndex;
  //找到有用的返回数据位置i 返回头不要
  for (int i = 0; i < answer.length(); i++) {
    if (answer[i] == '{') {
      jsonIndex = i;
      break;
    }
  }
  jsonAnswer = answer.substring(jsonIndex);

  //解析获取到的json数据
  // Stream& input;
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, jsonAnswer);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  JsonObject results_0 = doc["results"][0];

  JsonObject results_0_location = results_0["location"];
  const char* results_0_location_id = results_0_location["id"]; // "WW0CWDZP17BC"
  const char* results_0_location_name = results_0_location["name"]; // "苏州"
  const char* results_0_location_country = results_0_location["country"]; // "CN"
  const char* results_0_location_path = results_0_location["path"]; // "苏州,苏州,江苏,中国"
  const char* results_0_location_timezone = results_0_location["timezone"]; // "Asia/Shanghai"
  const char* results_0_location_timezone_offset = results_0_location["timezone_offset"]; // "+08:00"

  JsonObject results_0_now = results_0["now"];
  const char* results_0_now_text = results_0_now["text"]; // "阴"
  const char* results_0_now_code = results_0_now["code"]; // "9"
  const char* results_0_now_temperature = results_0_now["temperature"]; // "3"

  const char* results_0_last_update = results_0["last_update"]; // "2022-01-29T17:25:01+08:00"

  now_address = results_0_location_name;
  now_weather = results_0_now_text;
  now_weather_code = results_0_now_code;
  now_temperature = results_0_now_temperature;

  // Serial.print("地区：");
  // Serial.print(results_0_location_path);
  // Serial.print("\n 时区：");
  // Serial.print(results_0_location_timezone_offset);
  // Serial.print("\n 天气：");
  // Serial.print(results_0_now_text);
  // Serial.print("\n 当前温度：");
  // Serial.print(results_0_now_temperature);


  tft.fillRect(5,65,32,16,TFT_BLACK);       //打印地区
  if(now_address=="苏州") {
    showMyFonts(5,65,"苏州" , TFT_GOLD);
  }
  #define X 4
  #define Y 110
  #define pX 68
  #define pY 68
  tft.fillRect(X,Y,64,16,TFT_BLACK);
  tft.fillRect(68,68,60,60,TFT_BLACK);
  switch (std::atoi(now_weather_code.c_str())) {//天气代码不是int类型，要先转换
  /************************************************************************************
  天气代码是由心知天气的API接口提供的，详见[天气现象代码说明]
  (https://seniverse.yuque.com/books/share/e52aa43f-8fe9-4ffa-860d-96c0f3cf1c49/yev2c3)
  *************************************************************************************/
    case 0: showMyFonts(X,Y,"晴" , TFT_GREEN); tft.pushImage(pX,pY, 51,51,p0_5151);break;
    case 1: showMyFonts(X,Y,"夜晚晴" , TFT_GREEN); tft.pushImage(pX,pY, 51,52,p1_5152);break;
    case 2: showMyFonts(X,Y,"晴" , TFT_GREEN); tft.pushImage(pX,pY, 51,51,p0_5151);break;
    case 3: showMyFonts(X,Y,"夜晚晴" , TFT_GREEN); tft.pushImage(pX,pY, 51,52,p1_5152);break;
    case 4: showMyFonts(X,Y,"多云" , TFT_GREEN); tft.pushImage(pX,pY, 60,47,p4_6047);break;
    case 5: showMyFonts(X,Y,"晴间多云" , TFT_GREEN); tft.pushImage(pX,pY, 60,44,p5_6044);break;
    case 6: showMyFonts(X,Y,"晴间多云" , TFT_GREEN); tft.pushImage(pX,pY, 60,51,p6_6051);break;
    case 7: showMyFonts(X,Y,"大部多云" , TFT_GREEN); tft.pushImage(pX,pY, 60,42,p7_6042);break;
    case 8: showMyFonts(X,Y,"大部多云" , TFT_GREEN); tft.pushImage(pX,pY, 56,49,p8_5649);break;
    case 9: showMyFonts(X,Y,"阴" , TFT_GREEN); tft.pushImage(pX,pY, 60,40,p9_6040);break;
    case 10: showMyFonts(X,Y,"阵雨" , TFT_GREEN); tft.pushImage(pX,pY, 60,59,p10_6059);break;
    case 11: showMyFonts(X,Y,"雷阵雨" , TFT_GREEN); tft.pushImage(pX,pY, 56,56,p11_5656);break;
    case 12: {//滚动显示，这里还需要优化
      tft.pushImage(pX,pY, 56,56,p12_5656);
      showMyFonts(X,Y,"雷阵雨伴" , TFT_GREEN);delay(500);
      tft.fillRect(X,Y,64,16,TFT_BLACK);
      showMyFonts(X,Y,"阵雨伴有" , TFT_GREEN);delay(500);
      tft.fillRect(X,Y,64,16,TFT_BLACK);
      showMyFonts(X,Y,"雨伴有冰" , TFT_GREEN);delay(500);
      tft.fillRect(X,Y,64,16,TFT_BLACK);
      showMyFonts(X,Y,"伴有冰雹" , TFT_GREEN);delay(500);
      tft.fillRect(X,Y,64,16,TFT_BLACK);
      showMyFonts(X,Y,"阵雨冰雹" , TFT_GREEN);break;
    }
    case 13: showMyFonts(X,Y,"小雨" , TFT_GREEN); tft.pushImage(pX,pY, 56,54,p13_5654);break;
    case 14: showMyFonts(X,Y,"中雨" , TFT_GREEN); tft.pushImage(pX,pY, 56,54,p14_5654);break;
    case 15: showMyFonts(X,Y,"大雨" , TFT_GREEN); tft.pushImage(pX,pY, 56,54,p15_5654);break;
    case 16: showMyFonts(X,Y,"暴雨" , TFT_GREEN); tft.pushImage(pX,pY, 56,54,p16_5654);break;
    case 17: showMyFonts(X,Y,"大暴雨" , TFT_GREEN); tft.pushImage(pX,pY, 57,54,p17_5754);break;
    case 18: showMyFonts(X,Y,"特大暴雨" , TFT_GREEN); tft.pushImage(pX,pY, 57,54,p18_5754);break;
    case 19: showMyFonts(X,Y,"冻雨" , TFT_GREEN); tft.pushImage(pX,pY, 56,57,p19_5657);break;
    case 20: showMyFonts(X,Y,"雨夹雪" , TFT_GREEN); tft.pushImage(pX,pY, 56,55,p20_5655);break;
    case 21: showMyFonts(X,Y,"阵雪" , TFT_GREEN); tft.pushImage(pX,pY, 56,56,p21_5656);break;
    case 22: showMyFonts(X,Y,"小雪" , TFT_GREEN); tft.pushImage(pX,pY, 56,53,p22_5653);break;
    case 23: showMyFonts(X,Y,"中雪" , TFT_GREEN); tft.pushImage(pX,pY, 56,53,p23_5653);break;
    case 24: showMyFonts(X,Y,"大雪" , TFT_GREEN); tft.pushImage(pX,pY, 56,53,p24_5653);break;
    case 25: showMyFonts(X,Y,"暴雪" , TFT_GREEN); tft.pushImage(pX,pY, 56,56,p25_5656);break;
    case 26: showMyFonts(X,Y,"浮尘" , TFT_GREEN); tft.pushImage(pX,pY, 53,45,p26_5345);break;
    case 27: showMyFonts(X,Y,"扬沙" , TFT_GREEN); tft.pushImage(pX,pY, 53,45,p26_5345);break;
    case 28: showMyFonts(X,Y,"沙尘暴" , TFT_GREEN); tft.pushImage(pX,pY, 58,34,p28_5834);break;
    case 29: showMyFonts(X,Y,"强沙尘暴" , TFT_GREEN); tft.pushImage(pX,pY, 58,34,p28_5834);break;
    case 30: showMyFonts(X,Y,"雾" , TFT_GREEN); tft.pushImage(pX,pY, 54,50,p30_5450);break;
    case 31: showMyFonts(X,Y,"霾" , TFT_GREEN); tft.pushImage(pX,pY, 56,50,p31_5650);break;
    case 32: showMyFonts(X,Y,"风" , TFT_GREEN); tft.pushImage(pX,pY, 56,44,p32_5644);break;
    case 33: showMyFonts(X,Y,"大风" , TFT_GREEN); tft.pushImage(pX,pY, 56,44,p32_5644);break;
    case 34: showMyFonts(X,Y,"飓风" , TFT_GREEN); tft.pushImage(pX,pY, 56,56,p34_5656);break;
    case 35: showMyFonts(X,Y,"热带风暴" , TFT_GREEN); tft.pushImage(pX,pY, 56,56,p34_5656);break;
    case 36: showMyFonts(X,Y,"龙卷风" , TFT_GREEN); tft.pushImage(pX,pY, 56,55,p36_5655);break;
    case 37: showMyFonts(X,Y,"冷" , TFT_GREEN); tft.pushImage(pX,pY, 51,58,p37_5158);break;
    case 38: showMyFonts(X,Y,"热" , TFT_GREEN); tft.pushImage(pX,pY, 51,51,p38_5151);break;
    case 99: showMyFonts(X,Y,"未知" , TFT_GREEN); tft.pushImage(pX,pY, 53,23,p99_5323);break;
    default: break;
  }
  tft.fillRect(5,87,63,16,TFT_BLACK);       
  tft.setCursor(5, 88, 1);                 
  tft.setTextColor(TFT_SKYBLUE);             
  tft.setTextSize(2);                    
  tft.println(now_temperature);               

  if ( (( std::atoi(now_temperature.c_str()) ) < 10) && (( std::atoi(now_temperature.c_str()) ) >= 0)) {
    showMyFonts(24,87,"℃",TFT_SKYBLUE); 
  }
  else{
    showMyFonts(40,87,"℃",TFT_SKYBLUE); 
  }
}

//触摸按键
short int Touch_read;
void gettouch(){
  Touch_read = touchRead(Touch_read);
  delay(10);
  if(Touch_read<40){
    digitalWrite(TFT_BL, HIGH);
    timerWrite(timer, 0);
  }
}

// 定时器中断回调函数
void IRAM_ATTR timer_event()
{
  digitalWrite(TFT_BL, LOW);
  timer_flag = true;
}

// MQTT回调函数
void callback(char * topic,byte * payload,unsigned int length){
  DynamicJsonDocument doc(512);
  char charbuffer[512];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]");
  int i = 0;
  for(;i<length;i++){
    charbuffer[i] = (char)payload[i];
  }
  charbuffer[i] = '\0';
  DeserializationError error = deserializeJson(doc,charbuffer);

  if(error){
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
//   bool lightOn = doc["data"]["light_switch"];
//   bool dehumiOn = doc["data"]["dehumi_switch"];
}


//连接mqtt
void setupMQTT()
{
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback); 
  while (!client.connected())
  {
    Serial.println("Connecting MQTT");
    if(client.connect(ClientId,User,Pass))
    {
      Serial.println("MQTT connected successfully!");
      client.subscribe(TOPIC);
    }
    else
    {
      Serial.print("Failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

//温湿度数据显示并上报MQTT服务器
void DHTsensor(){
  int h = dht.readHumidity();
  int t = dht.readTemperature();

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("C \n"));

  tft.fillRect(39,141,63,160,TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  showMyFonts(4,140,"湿度" , TFT_GREEN);
  tft.setCursor(39,141);
  tft.print(h);
  // showMyFonts(40,140,"%" , TFT_GREEN);

  tft.fillRect(99,141,128,160,TFT_BLACK);
  showMyFonts(64,140,"温度" , TFT_GREEN);
  tft.setCursor(99,141);
  tft.print(t);
  // showMyFonts(64,140,"℃" , TFT_GREEN);

  // 封装json
  DynamicJsonDocument doc(512);
  DynamicJsonDocument jsdata(256);
  DynamicJsonDocument tempdata(32);
  DynamicJsonDocument humidata(32);
  // DynamicJsonDocument illudata(32);

  // tempdata["value"] = t;
  // humidata["value"] = h;
  // illudata["value"] = percent;
  jsdata["temp"] = t;
  jsdata["humi"] = h;
  // jsdata["illumi_current"] = illudata;
  doc["method"] = "report";
  doc["clientToke"] = "123";
  doc["params"] = jsdata;

  String str;
  serializeJson(doc, str);
  // Serial.println(str);

  // 发送MQTT
  char *p = (char *)str.c_str();
  if(client.publish(TOPIC,p) == true)
  {
    // Serial.println("Success sending message.");
  }
  else
  {
    Serial.println("Failed sending message.");
  }

  client.loop();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin();
  tft.init();                               //初始化
  tft.fillScreen(TFT_BLACK);                //清屏
  WiFi.begin(ssid, password);


  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  tft.println("");
  tft.println("WiFi connected"); //连接成功
  tft.print("IP address: \n");    //打印IP地址
  tft.println(WiFi.localIP());
  tft.println("Starting...");
  delay(500);
  tft.fillScreen(TFT_BLACK);
  setupMQTT();
  tft.drawLine(0,23, 128, 23, TFT_WHITE);  //画线
  tft.drawLine(0,60, 128, 60, TFT_WHITE);  //画线
  tft.drawLine(0,132, 128, 132, TFT_WHITE);  //画线
  timeClient.begin();
  timeClient.setTimeOffset(28800);  // + 1区 偏移3600， +8区 ：3600×8 = 28800
  tft.setSwapBytes(true);              // RGB->BGR，更改显示颜色模式。

  timer = timerBegin(0,80,true);						
  // 配置定时器 这里使用的是定时器0(一共四个0123) 
  // 80是这个定时器的分频系数 由于定时器基频是80Mhz 
  // 这里设置80 就是1Mhz 就能保证定时器1us记录一次 
  // true表面该定时器向上计数                                    
  timerAttachInterrupt(timer,&timer_event,true);		// 配置定时器的中断函数 true表示边沿触发
  timerAlarmWrite(timer,7000000,true);				// 设置定时器的报警值 当计时器计数值达到7000000时触发中断 true表示重加载   
  timerAlarmEnable(timer);							// 使能定时器报警

  touchAttachInterrupt(T7, gettouch, 40);//其中40为阈值，当通道T0上的值<40时，会触发中断
}

void loop() {

  // put your main code here, to run repeatedly:
  Display_Weather(); //天气信息每2分钟更新一次
  for(int i = 0; i < 115; i++) {
    Display_Time();
    delay(1000);   //间隔1秒访问ntp,刷新时间戳 

    if(timer_flag){
      timer_flag = false;
      DHTsensor();
    }

  }
}



