
#define M1A 12
#define M1B 13
#define M1P 0

#define M2A 14
#define M2B 15
#define M2P 1

int freq = 5000;
int ledChannel = 0;
int resolution = 8;


void setup() {
	
	Serial.begin(115200);

	pinMode(M1A,OUTPUT);
	pinMode(M1B,OUTPUT);

	digitalWrite(M1B,LOW);
	ledcSetup(M1P, freq, resolution);
	ledcAttachPin(M1A, M1P);
	ledcWrite(M1P,0);


	pinMode(M2A,OUTPUT);
	pinMode(M2B,OUTPUT);

	digitalWrite(M2B,LOW);
	ledcSetup(M2P, freq, resolution);
	ledcAttachPin(M2A, M2P);
	ledcWrite(M2P,0);

}

void loop() {
	
	if(Serial.available()){
		Serial.println("OK");
	}
  
	for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
		ledcWrite(M1P, dutyCycle);
		ledcWrite(M2P, dutyCycle);
		Serial.println(dutyCycle);
		delay(100);
	}
	for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {
		ledcWrite(M1P, dutyCycle);
		ledcWrite(M2P, dutyCycle);
		Serial.println(dutyCycle);
		delay(100);
	}

}
