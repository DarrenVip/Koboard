//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"


#define M1A 12
#define M1B 13
#define M1P 0

#define M2A 14
#define M2B 15
#define M2P 1


#define P25 25
#define PWM5 5

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif



BluetoothSerial SerialBT;
int freq = 5000;
int ledChannel = 0;
int resolution = 8;


void setup() {
  Serial.begin(115200);
  SerialBT.begin("BACR-ROBOT"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  
//  pinMode(M1A,OUTPUT);
//  pinMode(M1B,OUTPUT);
//  
//  digitalWrite(M1B,LOW);
//  ledcSetup(M1P, freq, resolution);
//  ledcAttachPin(M1A, M1P);
//  ledcWrite(M1P,0);
//
//
//  pinMode(M2A,OUTPUT);
//  pinMode(M2B,OUTPUT);
//  
//  digitalWrite(M2B,LOW);
//  ledcSetup(M2P, freq, resolution);
//  ledcAttachPin(M2A, M2P);
//  ledcWrite(M2P,0);

  pinMode(P25,OUTPUT);
  digitalWrite(P25,HIGH);
  ledcSetup(1, freq, resolution);
  ledcAttachPin(P25, 1);
  ledcWrite(1,0);
}

void loop() {
  if (Serial.available()) {
    Serial.println("OK");
  }
  
//  for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
//    ledcWrite(M1P, dutyCycle);
//    ledcWrite(M2P, dutyCycle);
//    ledcWrite(PWM5, dutyCycle);
//    Serial.println(dutyCycle);
//    delay(100);
//  }

  for (int dutyCycle = 0; dutyCycle <= 180 ; dutyCycle++) {
    
    ledcWrite(1, dutyCycle);
    delay(1000);
    Serial.println(dutyCycle);
  }
//
  for (int dutyCycle = 180; dutyCycle >= 0; dutyCycle--) {
    
    ledcWrite(1, dutyCycle);
    delay(1000);
    Serial.println(dutyCycle);
  }

  
}
