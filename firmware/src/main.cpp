#include <Arduino.h>

#define PWMA 22
#define PWMA_CHANNEL 0
#define PWMB 23
#define PWMB_CHANNEL 1
#define AIN1 21
#define AIN2 19
#define BIN1 18
#define BIN2 4
#define STBY 27

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

  digitalWrite(STBY, HIGH);
 

  ledcWrite(PWMB_CHANNEL, 128);  
  
}

void loop() {
  // put your main code here, to run repeatedly:""
  Serial.println("BIN1 high and BIN2 low");
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  delay(5000);

  Serial.println("BIN1 low and BIN2 high");
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);

  delay(5000);
}

