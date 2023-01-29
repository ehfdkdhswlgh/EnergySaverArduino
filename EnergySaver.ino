#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include "time.h"
#include "sntp.h"
#include "EEPROM.h"
#include <string.h>
#include <stdlib.h>

//AWS -> ESP32 : SUBSCRIBE (명령보내기)
//ESP32 -> AWS : PUBLISH (측정값 전송)
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/123/relay"
#define AWS_IOT_SUBSCRIBE_TOPIC2 "esp32/123/alarm/sub"
#define AWS_IOT_PUBLISH_TOPIC   "esp32/123/acs"


//GPIO34 ~ GPIO39  = 입력 전용 핀
//GPIO06 ~ GPIO11 = 사용금지 
//와이파이 연결 버튼 = GPIO 18번
//수동조작 버튼 GPIO 16 , GPIO 17번

#define RELAY1 19 //릴레이 1번 GPIO 19번
#define RELAY2 23 //릴레이 2번 GPIO 23번
#define ACS1 34 //ACS 1번 GPIO 34번
#define ACS2 35 //ACS 2번 GPIO 35번
#define DHTPIN 15 //DHT GPIO 15번
#define DHTTYPE DHT11

const int mVperAmp = 185;
int Watt = 0; //최종 출력
int Watt1 = 0; //ACS1번 측정치
int Watt2 = 0; //ACS2번 측정치
double VRMS1 = 0;
double AmpsRMS1 = 0;
double VRMS2 = 0;
double AmpsRMS2 = 0;
float h ;
float t;
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 32400;
const int   daylightOffset_sec = 0;
double vpp1;
double vpp2;
int whArr[2];
int counter = 0;
double wh = 0;
int relayState1 = 1;
int relayState2 = 1;
char wifiName[40] = "";
char wifiPassWord[40] = "";

int eepromIndex = 100; 

LiquidCrystal_I2C lcd (0x27, 16,2);
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
DHT dht(DHTPIN, DHTTYPE);


//0번주소에 알람배열의 위치(인덱스) 저장
// 100번주소부터 20단위로 알람구조체 저장


struct Alarm{ // 구조체 크기 = 12byte , but 20byte로 크기 고정할당할 예정
  
  int turn; //켜졌는지 꺼졋는지
  int outlet; //몇구
  int timestamp; //시간

};


void connectAWS()
{

  char emptyrom[40] = "";

  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();  
   
  Serial.println("Wait Smartconfig...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print ("Wait Smartconfig...");


  int smartConfigCount = 0;
  int lcdCount = 50;
  
  while (!WiFi.smartConfigDone()) {

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(lcdCount);
    delay(500);
    Serial.print(".");
    smartConfigCount = smartConfigCount + 1;
    lcdCount = lcdCount - 1;

    if(smartConfigCount == 50){
      Serial.print("Fail SmartConfig TimeOut Connect");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print ("TimeOut SmartConfig");
      delay(2000);
      lcd.clear();

      EEPROM.put(10, emptyrom);
      EEPROM.commit();
      EEPROM.put(50, emptyrom);
      EEPROM.commit();      
      
      
      return;
    }
    
  }

  Serial.println("");
  Serial.println("SmartConfig Success!!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print ("SmartConfig OK!!");
  delay(1000);
  
 
  Serial.println("Connect wifi...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print ("Connect wifi...");


  int lcdCount2 = 50;
  int wifiConnectCount = 0;
 
  while (WiFi.status() != WL_CONNECTED)
  {

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(lcdCount2);
    delay(500);
    Serial.print(".");
    wifiConnectCount = wifiConnectCount + 1;
    lcdCount2 = lcdCount2 - 1;

    if(wifiConnectCount == 50){
      Serial.print("Fail Wifi TimeOut Connect");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print ("TimeOut Connect");
      delay(2000);
      lcd.clear();   
      //WiFi.disconnect();

      EEPROM.put(10, emptyrom);
      EEPROM.commit();
      EEPROM.put(50, emptyrom);
      EEPROM.commit();      
      
      return;
    }
    
  }

  Serial.println("Wifi Success!!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println ("Wifi Success!!");
  lcd.setCursor(0, 1);
  lcd.print (WiFi.localIP());
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connect AWS...");
  lcd.setCursor(0, 0);
  lcd.print ("Connect AWS...     ");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print ("AWS IOT TIMEOUT!!!!");
    return;
  }
 
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC2);
  
 
  Serial.println("Connect Success!!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print ("Connect Success!!!");
  delay(1000);
  lcd.clear();

  sntp_set_time_sync_notification_cb( timeavailable );
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);


  String sname = WiFi.SSID();
  String spw = WiFi.psk();
  char saveWifiName[40];
  char saveWifiPw[40];
  strcpy(saveWifiName, sname.c_str());
  strcpy(saveWifiPw, spw.c_str());
  EEPROM.put(10, saveWifiName);
  EEPROM.commit();
  EEPROM.put(50, saveWifiPw);
  EEPROM.commit();
  
}



void publishMessage() // 전력 측정값 전송
{
  StaticJsonDocument<200> doc;
  doc["humidity"] = h;
  doc["temperature"] = t;
  doc["watt1"] = Watt1;
  doc["watt2"] = Watt2;
  doc["wh"] = wh;
  doc["relay1"] = relayState1;
  doc["relay2"] = relayState2;
 

  
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.println("---------------Message Published!!--------------");
}




void messageHandler(char* topic, byte* payload, unsigned int length) //AWS에서 보낸 명령을 받음
{
  Serial.print("incoming: ");
  Serial.println(topic);

  if ( strstr(topic, "esp32/123/relay") )
  {

    Serial.println("--------Relay Action Received !! -----------"); 
    
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);


    String RelayNum = doc["Relay"];
    int rNum = RelayNum.toInt();
    Serial.println(rNum);

    if(rNum == 1){ //1번 릴레이에 대한 명령일 때
       
       String Status = doc["Status"];
       int sNum = Status.toInt();
       Serial.println(sNum); 
       
       if(sNum == 0){
        digitalWrite(RELAY1, LOW);
        Serial.print("Relay1 is OFF"); 
        relayState1 = 0;
       }else if(sNum == 1){
        digitalWrite(RELAY1, HIGH);
        Serial.print("Relay1 is ON");
        relayState1 = 1;      
       }


    }else if(rNum == 2){ //2번 릴레이데 대한 명령일 때

       String Status = doc["Status"];
       int sNum = Status.toInt();
       Serial.println(sNum); 

       if(sNum == 0){
        digitalWrite(RELAY2, LOW);
        Serial.print("Relay2 is OFF"); 
        relayState2 = 0;
       }else if(sNum == 1){
        digitalWrite(RELAY2, HIGH);
        Serial.print("Relay2 is ON");      
        relayState2 = 1;
       }      
    }
    

  }
  else if ( strstr(topic, "esp32/123/alarm/sub") )
  {

    Serial.println("--------Alarm Action Received !! -----------"); 

    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);

    String outletStr = doc["Outlet"];
    int outletNum = outletStr.toInt();

    String action = doc["Action"];
    const char* actionChar = action.c_str();

    int actionNum = -1;
    
    if(strcmp(actionChar, "on") == 0){
      actionNum = 1;
    }else if(strcmp(actionChar, "off") == 0){
      actionNum = 0;
    }

    String timestampStr = doc["Timestamp"];
    const char* timestampChar = timestampStr.c_str();
    int   timestampInt = atoi(timestampChar);
    
    
    Alarm save = {actionNum,outletNum,timestampInt};

    EEPROM.put(eepromIndex, save);
    EEPROM.commit();

    eepromIndex = eepromIndex + 20;

    if(eepromIndex >= 500){
      eepromIndex = 100; //인덱스 초기화
    }

    EEPROM.put(0, eepromIndex);
    EEPROM.commit();

    Serial.print("outlet Num = ");
    Serial.println(outletNum);
    Serial.print("action Num = ");
    Serial.println(actionNum); 
    Serial.print("TimeStamp = ");
    Serial.println(timestampInt);  
    
  }

  Serial.println();
}










void calculateVPP()
{
  int readValue1;
  int readValue2;         
  int maxValue1 = 0;           
  int minValue1 = 4096;
  int maxValue2 = 0;
  int minValue2 = 4096;
          
   uint32_t start_time = millis();
   
   while((millis()-start_time) < 1000) //약 1초 소모
   {
       readValue1 = analogRead(ACS1);
       readValue2 = analogRead(ACS2);
   
       if (readValue1 > maxValue1) 
       {
           maxValue1 = readValue1;
           
       }
       if (readValue1 < minValue1) 
       {
           minValue1 = readValue1;
       }

       if (readValue2 > maxValue2) 
       {
           maxValue2 = readValue2;
           
       }
       if (readValue2 < minValue2) 
       {
           minValue2 = readValue2;
       }
       
   }
   
   vpp1 = ((maxValue1 - minValue1) * 5)/4096.0;
   vpp2 = ((maxValue2 - minValue2) * 5)/4096.0;
      
}




void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
}

double median(double a, double b, double c)
{
  if (a >= b)
    if (b >= c)
      return b;
    else if (a <= c)
      return a;
    else
      return c;
  else if (a > c)
    return a;
  else if (b > c)
    return c;
  else
    return b;
}



void setup(){
  
  Serial.begin(115200);
  
  dht.begin();
  
  lcd. init ();
  lcd. backlight ();
  lcd.clear();


  EEPROM.begin(500); //500byte 사용 


//  eeprom에 저장된 값은 수정후 재빌드 + 컴파일 해도 값이 살아있음.
//  최초 한번만 실행할 것
//  int NotUseDeleteMe = 100;
//  EEPROM.put(0, NotUseDeleteMe); 
//  EEPROM.commit();
//  Serial.println("EEPROM index RESET!");


  
  EEPROM.get(0, eepromIndex); //인덱스 가져오기
  

  pinMode (RELAY1, OUTPUT);  
  digitalWrite(RELAY1, HIGH);
  pinMode (RELAY2, OUTPUT);  
  digitalWrite(RELAY2, HIGH);


  pinMode(16, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
  pinMode(18, INPUT_PULLUP);

  for(int i = 0; i < 5; i++){
    whArr[i] = 0;
  }


  char tmpWifiName[40];
  char tmpWifiPW[40];

  EEPROM.get(10, tmpWifiName);
  EEPROM.get(50, tmpWifiPW);

     

  if((int)strlen(tmpWifiName) > 1 && (int)strlen(tmpWifiName) <= 40){ //저장된 값이 있을때

            Serial.println("----------------Already Have WIFI Info--------------");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print ("Found saved Wifi");
            Serial.println((char*)tmpWifiName);
            WiFi.begin((char*) tmpWifiName, (char*) tmpWifiPW);
          
            int wifiConnectCount = 0;
            int timeoutCount = 50;
           
            while (WiFi.status() != WL_CONNECTED)
            {
              lcd.clear();
              lcd.setCursor(0, 1);
              lcd.print(timeoutCount);
              delay(500);
              Serial.print(".");
              wifiConnectCount = wifiConnectCount + 1;
              timeoutCount = timeoutCount - 1;
          
              if(wifiConnectCount == 50){
                Serial.print("Fail Wifi TimeOut Connect");
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print ("TimeOut Connect");
                delay(2000);
                lcd.clear();   
                return;
              }
              
            }
          
            Serial.println("Wifi Success!!");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.println ("Wifi Success!!");
            lcd.setCursor(0, 1);
            lcd.print (WiFi.localIP());
           
            // Configure WiFiClientSecure to use the AWS IoT device credentials
            net.setCACert(AWS_CERT_CA);
            net.setCertificate(AWS_CERT_CRT);
            net.setPrivateKey(AWS_CERT_PRIVATE);
           
            // Connect to the MQTT broker on the AWS endpoint we defined earlier
            client.setServer(AWS_IOT_ENDPOINT, 8883);
           
            // Create a message handler
            client.setCallback(messageHandler);
           
            Serial.println("Connect AWS...");
            lcd.setCursor(0, 0);
            lcd.print ("Connect AWS...     ");
           
            while (!client.connect(THINGNAME))
            {
              Serial.print(".");
              delay(100);
            }
           
            if (!client.connected())
            {
              Serial.println("AWS IoT Timeout!");
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print ("AWS IOT TIMEOUT!!!!");
              return;
            }
           
            client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
            client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC2);
            
           
            Serial.println("Connect Success!!");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print ("Connect Success!!!");
            delay(1000);
            lcd.clear();
          
            sntp_set_time_sync_notification_cb( timeavailable );
            configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
          

  }
 

  
}


void resetButton(){

  int count = 0;

  while(count <= 300){
    if(digitalRead(18) == HIGH){
      return;
    }
    count = count + 1;
    delay(1);
  }

  Serial.println("RESET");
  connectAWS();
  
}


void passiveButton1(){
  int count = 0;

  while(count <= 300){
    if(digitalRead(16) == HIGH){
      return;
    }
    count = count + 1;
    delay(1);
  }

  Serial.println("Passive Button1 Click!");

  if(relayState1 == 0){ //릴레이1이 이미 꺼져있을 때
     
       digitalWrite(RELAY1, HIGH);
       Serial.print("Relay1 is ON (caused by Passive Button1)");
       relayState1 = 1; 
    
  }else if(relayState1 == 1){ //릴레이1이 이미 켜져있을 때

       digitalWrite(RELAY1, LOW);
       Serial.print("Relay1 is OFF (caused by Passive Button1)");
       relayState1 = 0; 
    
  }

}


void passiveButton2(){
  int count = 0;

  while(count <= 300){
    if(digitalRead(17) == HIGH){
      return;
    }
    count = count + 1;
    delay(1);
  }

  Serial.println("Passive Button2 Click!");

  if(relayState2 == 0){ //릴레이2이 이미 꺼져있을 때
     
       digitalWrite(RELAY2, HIGH);
       Serial.print("Relay2 is ON (caused by Passive Button2)");
       relayState2 = 1; 
    
  }else if(relayState2 == 1){ //릴레이2이 이미 켜져있을 때

       digitalWrite(RELAY2, LOW);
       Serial.print("Relay2 is OFF (caused by Passive Button1)");
       relayState2 = 0; 
    
  }

}




unsigned int getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);

  now = now + 32400;
  return now;
}



void loop() {

  if(digitalRead(18) == LOW){ 
    resetButton();
  }

  if(digitalRead(16) == LOW){
    passiveButton1();
  }
  
  if(digitalRead(17) == LOW){
    passiveButton2();
  }  

  h = dht.readHumidity();
  t = dht.readTemperature();
    
  calculateVPP(); //약 1초 소모
  double vpp1temp1 = vpp1;
  double vpp2temp1 = vpp2;
  calculateVPP(); //약 1초 소모
  double vpp1temp2 = vpp1;
  double vpp2temp2 = vpp2;
  calculateVPP(); //약 1초 소모
  double vpp1temp3 = vpp1;
  double vpp2temp3 = vpp2;

  vpp1 = median(vpp1temp1, vpp1temp2, vpp1temp3);
  vpp2 = median(vpp2temp1, vpp2temp2, vpp2temp3);

  //---------------------------------------------------------------------------------------------------------------------
  VRMS1 = (vpp1 / 2.0) *0.707;
  AmpsRMS1 = (((VRMS1 * 1000)/mVperAmp)-0.5); //보정 필요함
  Watt1 = (AmpsRMS1 * 220 / 1.35 - 13); //보정 필요함

  VRMS2 = (vpp2 / 2.0) *0.707;
  AmpsRMS2 = (((VRMS2 * 1000)/mVperAmp)-0.5); //보정 필요함
  Watt2 = (AmpsRMS2 * 220 / 1.35 - 13); //보정 필요함  
  //------------------------------------------------------------------------------------------------------------------------

  if(Watt1 < 0){
    Watt1 = 0;
  }

  if(Watt2 < 0){
    Watt2 = 0;
  }

  Watt = Watt1 + Watt2;

  int currentTimeStamp = -1;


  currentTimeStamp = getTime();


  Serial.print("ACS1 : ");
  Serial.print(Watt1);
  Serial.println("Watts");
  Serial.print("ACS2 : ");
  Serial.print(Watt2);
  Serial.println("Watts");
  Serial.print("Total ");
  Serial.print(Watt);
  Serial.println("Watts");
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
  Serial.print("current SSID  : ");
  Serial.println(WiFi.SSID());
  Serial.print("Current TimeStamp : ");
  Serial.println(currentTimeStamp);   
  Serial.print("Current Index : ");
  Serial.println(eepromIndex);
  Serial.print("relayState1 = ");
  Serial.println(relayState1);
  Serial.print("relayState2 = ");
  Serial.println(relayState2);

  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print("1:");
  lcd.print(Watt1);
  lcd.print("W");
  lcd.setCursor(8, 0);
  lcd.print("2:");
  lcd.print(Watt2);
  lcd.print("W");
  lcd.setCursor (0, 1);
  lcd.print("Hu:");
  lcd.print((int)h);
  lcd.print("%");
  lcd.setCursor(8, 1);
  lcd.print("Tem:");
  lcd.print((int)t);
  lcd.print("'C");


  whArr[counter] = Watt;

  if(counter >= 1) //6초에 한번씩 서버로 메시지를 전송함.
  {
    double total = 0;

    total = whArr[0] + whArr[1];

    wh = total / 600.0; //6초동안의 Wh
    Serial.print("6 second Wh = ");
    Serial.println(wh);


    //if(WiFi.status() == WL_CONNECTED){
      publishMessage(); 
    //}
    
    counter = 0;
  }
  else
  {    
    counter = counter + 1;
  }



     for(int i = 100; i < 500; i = i + 20){
  
      Alarm a;
      EEPROM.get(i, a);
  
  
      if(currentTimeStamp - 10 <= a.timestamp && a.timestamp <= currentTimeStamp){
  
  
        if(a.outlet == 1){
  
          if(a.turn == 1){ //1구 전원 on
            digitalWrite(RELAY1, HIGH);
            Serial.print("Relay1 is ON (caused by ALARM)");
            relayState1 = 1;
          }else if(a.turn == 0){ //1구 전원 off
            digitalWrite(RELAY1, LOW);
            Serial.print("Relay1 is OFF (caused by ALARM)");
            relayState1 = 0;
          }
          
        }else if(a.outlet == 2){ 
  
          if(a.turn == 1){ //2구 전원 on
            digitalWrite(RELAY2, HIGH);
            Serial.print("Relay2 is ON (caused by ALARM)");
            relayState2 = 1;
          }else if(a.turn == 0){ //2구 전원 off
            digitalWrite(RELAY2, LOW);
            Serial.print("Relay2 is OFF (caused by ALARM)");
            relayState2 = 0;
          }
    
        }
  
        // -1로 초기화해서 eeprom에 저장
        Alarm reset = {-1, -1, -1};
        EEPROM.put(i, reset);
        EEPROM.commit();
   
        break;
      }
      
      
    }



  


  
  client.loop(); //1루프 = 3초

  

}
