#include <Arduino.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
//#include "./stack/bt_types.h"
//#include "./stack/gap_api.h"
#include "gap.h"
#include "gamepad.h"

void onConnection() {
  if (GP.isConnected()) {
    Serial.println("gamepad connected.");
  }else{
    Serial.println("gamepad unconnect.");
  }
}
void setup() {
  Serial.begin(115200);
  GP.begin();
  GP.attachOnConnect(onConnection);
}

void loop() {
	if(GP.isConnected()) {
		if ( GP.data.button.A )
			Serial.println("A Button");
		if ( GP.data.button.B )
			Serial.println("B Button");
		if ( GP.data.button.X )
			Serial.println("X Button");
		if ( GP.data.button.Y )
			Serial.println("Y Button");
		if ( GP.data.button.up )
			Serial.println("Up Button");
		if ( GP.data.button.down )
			Serial.println("Down Button");
		if ( GP.data.button.left )
			Serial.println("Left Button");
		if ( GP.data.button.right )
			Serial.println("Right Button");
		  
		if ( GP.data.button.upright )
			Serial.println("Up Right");
		if ( GP.data.button.upleft )
			Serial.println("Up Left");
		if ( GP.data.button.downleft )
			Serial.println("Down Left");
		if ( GP.data.button.downright )
			Serial.println("Down Right");
		  
		if ( GP.data.button.triangle )
			Serial.println("Triangle Button");
		if ( GP.data.button.circle )
			Serial.println("Circle Button");
		if ( GP.data.button.cross )
			Serial.println("Cross Button");
		if ( GP.data.button.square )
			Serial.println("Square Button");
		  
		if ( GP.data.button.l1 )
			Serial.println("l1 Button");
		if ( GP.data.button.r1 )
			Serial.println("r1 Button");
		  
		if ( GP.data.button.l3 )
			Serial.println("l3 Button");
		if ( GP.data.button.r3 )
			Serial.println("r3 Button");
		  
		if ( GP.data.button.share )
			Serial.println("Share Button");
		if ( GP.data.button.options )
			Serial.println("Options Button");
		  
		  
		if ( GP.data.button.l2 ) {
			Serial.print("l2 button at ");
			Serial.println(GP.data.analog.button.l2, DEC);
		}
		if ( GP.data.button.r2 ) {
			Serial.print("r2 button at ");
			Serial.println(GP.data.analog.button.r2, DEC);
		}

		if ( GP.event.analog_move.stick.lx ) {
			Serial.print("Left Stick x at ");
			Serial.println(GP.data.analog.stick.lx, DEC);
		}
		if ( GP.event.analog_move.stick.ly ) {
			Serial.print("Left Stick y at ");
			Serial.println(GP.data.analog.stick.ly, DEC);
		}
		if ( GP.event.analog_move.stick.rx ) {
			Serial.print("Right Stick x at ");
			Serial.println(GP.data.analog.stick.rx, DEC);
		}
		if ( GP.event.analog_move.stick.ry ) {
			Serial.print("Right Stick y at ");
			Serial.println(GP.data.analog.stick.ry, DEC);
		}
	//Serial.println();
	//This delay is to make the Serial Print more human readable
	//Remove it when you're not trying to see the output
	//delay(1000);
    }
}