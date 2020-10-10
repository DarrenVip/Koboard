#include <Arduino.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "gap.h"
#include "gamepad.h"


//初始化左一(12,13)电机
void initMotorL1(){
	pinMode(16,OUTPUT);
	pinMode(17,OUTPUT);
	digitalWrite(16,LOW);
	ledcSetup(0, 2000, 8);
	ledcAttachPin(17, 0);
	ledcWrite(0,0);
}

//初始化右一(16,17)电机
void initMotorR1(){
	pinMode(12,OUTPUT);
	pinMode(13,OUTPUT);
	digitalWrite(12,LOW);
	ledcSetup(1, 2000, 8);
	ledcAttachPin(13, 1);
	ledcWrite(1,0);
}
//设置左一电机速度
void setPwmL1(int dutyCycle){
  if(dutyCycle<0){
     digitalWrite(16,HIGH);
     ledcWrite(0,255+dutyCycle);
  }else{
    digitalWrite(16,LOW);
    ledcWrite(0, dutyCycle);
  }
}
//设置右一电机速度
void setPwmR1(int dutyCycle){
  if(dutyCycle<0){
     digitalWrite(12,HIGH);
     ledcWrite(1,255+dutyCycle);
  }else{
    digitalWrite(12,LOW);
    ledcWrite(1, dutyCycle);
  }
  
}

//手柄链接状态改变
void onConnection() {
  if(GP.isConnected()){
    Serial.println("gamepad connected.");
  }else{
    Serial.println("gamepad unconnect.");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("The device started, now you can pair it with gamepad!");
  GP.begin(); //开始自动扫描并连接手柄
  GP.attachOnConnect(onConnection);

  //初始化左一电机
  initMotorL1();
  //初始化右一电机
  initMotorR1();
}
void loop() {
	//连接成功
	if(GP.isConnected()) {
		//按键按下
		if ( GP.data.button.A ){
			Serial.println("A Button");
      setPwmR1(-255/2);
      setPwmL1(-255/2);
		}
		else if ( GP.data.button.B ){
			Serial.println("B Button");
      setPwmL1(255/2);
      setPwmR1(-255/2);
    }
		else if ( GP.data.button.X ){
			Serial.println("X Button");
      setPwmL1(-255/2);
      setPwmR1(255/2);
    }else if ( GP.data.button.Y ){
      Serial.println("Y Button");
      setPwmR1(255/2);
      setPwmL1(255/2);
		}else{
      setPwmR1(0);
      setPwmL1(0);
	  }
  }else{
    setPwmR1(0);
    setPwmL1(0);
  }
}