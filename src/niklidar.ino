#include "upload.h"
#include "Arduino.h"
#include <WiFi.h>
#include <PCF8574.h>

#define STEP1 32
#define DIR1 33


PCF8574 pcf8574(0x38);
WiFiServer wifiServer(80);

void setup() {
    esp32server_setup();
    Wire.begin (21, 22);   // sda= GPIO_21 /scl= GPIO_22

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
    }

    wifiServer.begin();

    pinMode(STEP1, OUTPUT);
    pinMode(DIR1, OUTPUT);

    pcf8574.pinMode(P0, OUTPUT);
	pcf8574.begin();
    pcf8574.digitalWrite(P0, LOW);
}

void loop(){
    for(int i=0; i<1000; i++){
        digitalWrite(STEP1, HIGH);
        delay(1);
        digitalWrite(STEP1, LOW);
        delay(1);
    }
    digitalWrite(DIR1, 0);

    for(int i=0; i<1000; i++){
        digitalWrite(STEP1, HIGH);
        delay(1);
        digitalWrite(STEP1, LOW);
        delay(1);
    }
    digitalWrite(DIR1, 1);

    /*WiFiClient client = wifiServer.available();
    if (client) {
        while (client.connected()) {
            Scanner(client);
        }
        client.stop();
    }*/
}