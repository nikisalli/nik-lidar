#include "upload.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

/* fill your ssid and password here */
const char* ssid = "ssid";
const char* password = "pass";

TaskHandle_t Task1;

void Task1code( void * parameter) {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    ArduinoOTA.setPort(3232);
    ArduinoOTA.setHostname("esp32");
    ArduinoOTA.setPassword("iotsharing");
    ArduinoOTA.onStart([]() {
        Serial.println("Start updating");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd updating");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.print("ESP IP address: ");
    Serial.println(WiFi.localIP());
    
    for (;;) {
        ArduinoOTA.handle();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void esp32server_setup() {
    xTaskCreatePinnedToCore(Task1code, "Task1", 50000, NULL, 1, &Task1, 0);
}