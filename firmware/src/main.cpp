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
#define LEFT_ENCODERG 34
#define LEFT_ENCODERY 35
#define RIGHT_ENCODERG 36
#define RIGHT_ENCODERY 39


#define currentPort 8889

WiFiUDP wifiUDP;
int character;
String msg;

float linear;
float angular;
float length=0.3;

float v_left;
float v_right;

int d_left;
int d_right;

int LG_Count = 0;
int RG_Count = 0;
int LD = 0;
int RD = 0;

int dutyConversion(float v_wheel);
void d_reverse(int highPin, int lowPin);
void RspeedDirectionCheck();
void LspeedDirectionCheck();

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
  pinMode(LEFT_ENCODERG, INPUT);
  pinMode(LEFT_ENCODERY, INPUT);
  pinMode(RIGHT_ENCODERG, INPUT);
  pinMode(RIGHT_ENCODERY, INPUT);

  ledcSetup(PWMA_CHANNEL, 20000, 8); 
  ledcAttachPin(PWMA, PWMA_CHANNEL);            
  ledcWrite(PWMA_CHANNEL, 12); 
  
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

  attachInterrupt(digitalPinToInterrupt(LEFT_ENCODERG), LspeedDirectionCheck, RISING);
  attachInterrupt(digitalPinToInterrupt(RIGHT_ENCODERG), RspeedDirectionCheck, RISING);

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
  if(character != -1){
    msg = "";
  }
  while(character != -1) {
    msg += (char)character;
    character = wifiUDP.read();
  }
  
  linear = msg.substring(0, msg.indexOf(",")).toFloat();
  angular = msg.substring(msg.indexOf(",") + 1).toFloat();

  v_left = linear - angular*(length/2);
  v_right = linear + angular*(length/2);

  if(v_left > 0){
    d_reverse(AIN1, AIN2);
  }
  else{
    d_reverse(AIN2, AIN1);
  }

  if(v_right > 0){
    d_reverse(BIN1, BIN2);
  }
  else{
    d_reverse(BIN2, BIN1);
  }

  if(v_left == 0 && v_right == 0){
    digitalWrite(STBY, LOW);
  }
  else{
    digitalWrite(STBY, HIGH);
  }

  d_left = dutyConversion(v_left);
  d_right = dutyConversion(v_right);

  Serial.println(msg);
  Serial.println(linear);
  Serial.println(angular);
  Serial.println(v_left);
  Serial.println(v_right);
  Serial.println(d_left);
  Serial.println(d_right);

  ledcWrite(PWMA_CHANNEL, d_left);
  ledcWrite(PWMB_CHANNEL, d_right);


  delay(2000);
}

int dutyConversion(float v_wheel) {
  int duty = (int)((fabs(v_wheel)/0.35)*255);
  if(duty > 255){
    return 255;
  }
  else if(duty > 12){
    return duty;
  }
  else{
    return 0;
  }
}

void d_reverse(int highPin, int lowPin){
  digitalWrite(highPin, HIGH);
  digitalWrite(lowPin, LOW);
}

void RspeedDirectionCheck(){
  RG_Count++;
  if(digitalRead(RIGHT_ENCODERY) == HIGH){
    RD = 1;
  }
  else{
    RD = 0;
  }
  
}

void LspeedDirectionCheck(){
  LG_Count++;
  if(digitalRead(LEFT_ENCODERY) == HIGH){
    LD = 1;
  }
  else{
    LD = 0;
  }
  
}

