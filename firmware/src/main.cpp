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
WiFiUDP debugMonitor;
WiFiUDP odomSend;
int character;
String msg;

float linear;
float angular;
float length= 0.254;
float max_speed = 0.45; // m/s

float v_left;
float v_right;

int d_left;
int d_right;

volatile long LG_Count = 0;
volatile long RG_Count = 0;
volatile long LG_Count_prev = 0;
volatile long RG_Count_prev = 0;

int LD = 0; // left direction
int RD = 0; // right direction

float true_speed_left;
float true_speed_right;


unsigned long watchdogTime;
unsigned long myTime;
unsigned long sendTimeOdom;
unsigned long sendTimeMonitor;

int dutyConversion(float v_wheel);
void d_reverse(int highPin, int lowPin);
void RspeedDirectionCheck();
void LspeedDirectionCheck();

IPAddress laptop_ip(192, 168, 4, 51);

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
  
  ledcSetup(PWMB_CHANNEL, 20000, 8); 
  ledcAttachPin(PWMB, PWMB_CHANNEL);           
  
  WiFi.begin(ssid, password);

  IPAddress local_IP(192, 168, 4, 211);
  IPAddress gateway(192, 168, 4, 1);     // your router's address
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(local_IP, gateway, subnet);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  wifiUDP.begin(currentPort);

  debugMonitor.begin(8887);
  odomSend.begin(8886);

  attachInterrupt(digitalPinToInterrupt(LEFT_ENCODERG), LspeedDirectionCheck, RISING);
  attachInterrupt(digitalPinToInterrupt(RIGHT_ENCODERG), RspeedDirectionCheck, RISING);

  myTime = millis();
  watchdogTime = millis();
  sendTimeOdom = millis();

}

void loop() {
  // put your main code here, to run repeatedly:""

  wifiUDP.parsePacket();
  character = wifiUDP.read();
  if(character != -1 || millis() - watchdogTime > 400){
    msg = "";
    watchdogTime = millis();
  }
  while(character != -1) {
    msg += (char)character;
    character = wifiUDP.read();
  }
  
  linear = msg.substring(0, msg.indexOf(",")).toFloat();
  angular = msg.substring(msg.indexOf(",") + 1).toFloat();

  v_left = linear - angular*(length/2);
  v_right = linear + angular*(length/2);

  if(fabs(v_left) > fabs(max_speed)){
    v_right = max_speed * (v_right/v_left);
    if(v_left > 0){
      v_left = max_speed;
    }
    else{
      v_left = -max_speed;
    }
  }
  else if(fabs(v_right) > fabs(max_speed)){
    v_left = max_speed * (v_left/v_right);
    if(v_right> 0){
      v_right = max_speed;
    }
    else{
      v_right = -max_speed;
    }
  }


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
 

  ledcWrite(PWMA_CHANNEL, d_left);
  ledcWrite(PWMB_CHANNEL, d_right);

  if(millis() - myTime > 50){
    true_speed_left = ((LG_Count - LG_Count_prev)/408.0)*(3.14159*0.065)/0.05;
    true_speed_right = ((RG_Count - RG_Count_prev)/408.0)*(3.14159*0.065)/0.05;
    LG_Count_prev = LG_Count;
    RG_Count_prev = RG_Count;
    myTime = millis();
  }

  if(millis() - sendTimeMonitor > 1000){
    debugMonitor.beginPacket(laptop_ip, 8887);
    debugMonitor.print("msg: ");
    debugMonitor.println(msg);
    debugMonitor.print("linear: ");
    debugMonitor.println(linear);
    debugMonitor.print("angular: ");
    debugMonitor.println(angular);
    debugMonitor.print("v_left: ");
    debugMonitor.print(v_left);
    debugMonitor.print("  v_right: ");
    debugMonitor.println(v_right);
    debugMonitor.print("d_left: ");
    debugMonitor.print(d_left);
    debugMonitor.print("  d_right: ");
    debugMonitor.println(d_right);
    debugMonitor.print("true_speed_left: ");
    debugMonitor.print(true_speed_left);
    debugMonitor.print("  true_speed_right: "); 
    debugMonitor.println(true_speed_right);
    debugMonitor.print("LG_Count: ");
    debugMonitor.print(LG_Count);
    debugMonitor.print("  RG_Count: ");
    debugMonitor.println(RG_Count);
    debugMonitor.endPacket();
    sendTimeMonitor = millis();
  }

  if(millis() - sendTimeOdom > 50){
    odomSend.beginPacket(laptop_ip, 8886);
    odomSend.print(LG_Count);
    odomSend.print(",");
    odomSend.print(RG_Count);
    odomSend.endPacket();
    sendTimeOdom = millis();
  }


}

int dutyConversion(float v_wheel) {
  int duty = (int)((fabs(v_wheel)/max_speed)*255);
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
  if(digitalRead(RIGHT_ENCODERY) == HIGH){
    RD = 1;
    RG_Count--;
  }
  else{
    RD = 0;
    RG_Count++;
  }
  
}

void LspeedDirectionCheck(){
  if(digitalRead(LEFT_ENCODERY) == HIGH){
    LD = 1;
    LG_Count++;
  }
  else{
    LD = 0;
    LG_Count--;
  }
  
}

