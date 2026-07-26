#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "credentials.cpp"

#define PWMA 22
#define PWMA_CHANNEL 0
#define PWMB 23
#define PWMB_CHANNEL 1
#define AIN1 21
#define AIN2 19
#define BIN1 18
#define BIN2 4
#define STBY 27


#define currentPort 8889

WiFiUDP wifiUDP;
int character;
String msg;
float linear;
float angular;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);


  ledcSetup(PWMB_CHANNEL, 20000, 8); 
  ledcAttachPin(PWMB, PWMB_CHANNEL);            
  ledcWrite(PWMB_CHANNEL, 128); 

  digitalWrite(STBY, HIGH);
  
  WiFi.begin(ssid, password);

  IPAddress local_IP(192, 168, 4, 210);
  IPAddress gateway(192, 168, 4, 1);     // your router's address
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(local_IP, gateway, subnet);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  wifiUDP.begin(currentPort);


}

void loop() {
  // put your main code here, to run repeatedly:""
  /*Serial.println("BIN1 high and BIN2 low");
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  delay(5000);

  Serial.println("BIN1 low and BIN2 high");
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);

  delay(5000);*/

  wifiUDP.parsePacket();
  character = wifiUDP.read();
  while(character != -1) {
    msg += (char)character;
    character = wifiUDP.read();
  }

  linear = msg.substring(0, msg.indexOf(",")).toFloat();
  angular = msg.substring(msg.indexOf(",") + 1).toFloat();

  Serial.println(msg);
  Serial.println(linear);
  Serial.println(angular);

  msg = "";
  delay(2000);
}

