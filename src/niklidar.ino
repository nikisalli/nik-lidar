//#include "upload.h"
#include "queue_list.h" //needs full path because .ino files are sh!t
#include "lidar_buffer.h"  //needs full path because .ino files are sh!t
#include "config.h"
#include <WiFi.h>
#include <PCF8574.h>
#include <YDLidar.h>

#define STEP1 32
#define DIR1 33
#define YDLIDAR_MOTRO_EN 4 
#define SWITCH_OUT 12
#define SWITCH 13
#define CW 0
#define CCW 1
#define SIZE_OF_SCAN_BUFFER 1024
#define MAX_POS 4000 // 1600 * 2.5
#define BUF_SIZE 1800

PCF8574 pcf8574(0x38);
WiFiServer wifiServer(80);
YDLidar lidar;
QueueList<scanPoint> scans;
buffer<uint8_t> buf(BUF_SIZE);

bool isScanning = false;   

unsigned int pos = 0;
bool dir = CW;

uint8_t header[10] = {0xFF,0xFE,0xFD,0xFC,0xFB,0xFA,0xF9,0xF8,0x0F7,0xF6};

void lidar_parser(Stream& S) {
    if (lidar.waitScanDot() == RESULT_OK) {
        scanPoint _point = lidar.getCurrentScanPoint();
        if(scans.count() <= SIZE_OF_SCAN_BUFFER){
            scans.push(_point);
        }else{
            scans.pop();
            scans.push(_point);
        }
    }else{
        //S.println(" YDLIDAR get Scandata failed!!");
        //restartScan(S);
    }
}

void setup() {
    disableCore0WDT();
    disableCore1WDT();

    Serial.begin(2000000);
    Serial2.begin(128000);
    pinMode(YDLIDAR_MOTRO_EN, OUTPUT);
    pinMode(STEP1, OUTPUT);
    pinMode(DIR1, OUTPUT);
    pinMode(SWITCH_OUT, OUTPUT);
    pinMode(SWITCH, INPUT_PULLDOWN);

    digitalWrite(SWITCH_OUT, HIGH);
    //esp32server_setup();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    WiFi.setHostname(hostname);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    lidar.begin(Serial2, 128000);
    Wire.begin (21, 22);   // sda= GPIO_21 /scl= GPIO_22
    wifiServer.begin();
    pcf8574.pinMode(P0, OUTPUT);
	pcf8574.begin();
    pcf8574.digitalWrite(P0, LOW);

    while(Serial2.read() >= 0){};

    digitalWrite(DIR1, CCW);
    dir = CCW;

    while(!digitalRead(SWITCH)){
        digitalWrite(STEP1, HIGH);
        delay(1);
        digitalWrite(STEP1, LOW);
        delay(1);
    }

    delay(500);

    digitalWrite(DIR1, CW);
    dir = CW;

    for(int i=0; i<120; i++){
        digitalWrite(STEP1, HIGH);
        delay(1);
        digitalWrite(STEP1, LOW);
        delay(1);
    }

    delay(500);

    digitalWrite(DIR1, CCW);
    dir = CCW;

    while(!digitalRead(SWITCH)){
        digitalWrite(STEP1, HIGH);
        delay(15);
        digitalWrite(STEP1, LOW);
        delay(15);
    }

    digitalWrite(DIR1, CW);
    dir = CW;

    for(int i=0; i<100; i++){
        digitalWrite(STEP1, HIGH);
        delay(2);
        digitalWrite(STEP1, LOW);
        delay(2);
    }

    delay(1000);
}

void loop(){
    WiFiClient client = wifiServer.available();
    if (client) {
        int prev = 0;
        unsigned long time = 0;
        int samples = 0;
        while (client.connected()) {
            if(buf.size == BUF_SIZE){
                client.write(header, sizeof(header)/sizeof(uint8_t));
                client.write(buf.arr, buf.size);
                buf.clear();
            }
            lidar_parser(Serial);
            if(isScanning){
                if(scans.count() > 0){
                    scanPoint _point;
                    _point = scans.pop();
                    float distance = _point.distance; //distance value in mm unit
                    float angle    =  _point.angle; //anglue value in degree

                    if(distance > 0){
                        buf.append((uint8_t)(((uint16_t)(angle*100)) >> 8));
                        buf.append((uint8_t)(((uint16_t)(angle*100)) & 0xFF));
                        buf.append((uint8_t)(((uint16_t)distance) >> 8));
                        buf.append((uint8_t)(((uint16_t)distance) & 0xFF));
                        buf.append((uint8_t)(((uint16_t)(pos)) >> 8));
                        buf.append((uint8_t)(((uint16_t)(pos)) & 0xFF));
                        
                        samples++;
                    }

                    if(prev-angle > 300){
                        int t = millis() - time;
                        time = millis();
                        Serial.println( "rev! time: " + String(t) + 
                                        " freq:" + String(1000.0/t) + 
                                        " samples:" + String(samples) + 
                                        " sps:" + String((float)samples/(t/1000.0)));
                        samples = 0;

                        pos += 1 - dir*2;

                        digitalWrite(STEP1, HIGH);
                        delayMicroseconds(50);
                        digitalWrite(STEP1, LOW);

                        if(pos >= MAX_POS){
                            digitalWrite(YDLIDAR_MOTRO_EN, LOW);
                            while(1);
                            dir = !dir;
                            digitalWrite(DIR1, dir);
                        }
                    }
                    prev = angle;
                }
            }else{
                digitalWrite(YDLIDAR_MOTRO_EN, LOW);
                restartScan(Serial);
            }
        }
        client.stop();
    }
}

void restartScan(Stream& S){
    device_info deviceinfo;
    if (lidar.getDeviceInfo(deviceinfo, 100) == RESULT_OK) {
        int _samp_rate=4;
        String model;
        float freq = 7.0f;
        switch(deviceinfo.model){
            case 1:
                model="F4";
                _samp_rate=4;
                freq = 7.0;
                break;
            case 4:
                model="S4";
                _samp_rate=4;
                freq = 7.0;
                break;
            case 5:
                model="G4";
                _samp_rate=9;
                freq = 7.0;
                break;
            case 6:
                model="X4";
                _samp_rate=5;
                freq = 7.0;
                break;
            default:
                model = "Unknown";
        }

        uint16_t maxv = (uint16_t)(deviceinfo.firmware_version>>8);
        uint16_t midv = (uint16_t)(deviceinfo.firmware_version&0xff)/10;
        uint16_t minv = (uint16_t)(deviceinfo.firmware_version&0xff)%10;
        if(midv==0){
            midv = minv;
            minv = 0;
        }

        S.print("Firmware version:");
        S.print(maxv,DEC);
        S.print(".");
        S.print(midv,DEC);
        S.print(".");
        S.println(minv,DEC);

        S.print("Hardware version:");
        S.println((uint16_t)deviceinfo.hardware_version,DEC);

        S.print("Model:");
        S.println(model);

        S.print("Serial:");
        for (int i=0;i<16;i++){
            S.print(deviceinfo.serialnum[i]&0xff, DEC);
        }
        S.println("");

        S.print("[YDLIDAR INFO] Current Sampling Rate:");
        S.print(_samp_rate,DEC);
        S.println("K");

        S.print("[YDLIDAR INFO] Current Scan Frequency:");
        S.print(freq,DEC);
        S.println("Hz");
        delay(100);
        device_health healthinfo;
        if (lidar.getHealth(healthinfo, 100) == RESULT_OK){
            // detected...
            S.print("[YDLIDAR INFO] YDLIDAR running correctly! The health status:");
            S.println( healthinfo.status==0?"well":"bad");
            digitalWrite(YDLIDAR_MOTRO_EN, HIGH);
            vTaskDelay(pdMS_TO_TICKS(500));
            if(lidar.startScan() == RESULT_OK){
                isScanning = true;
                //start motor in 1.8v
                //digitalWrite(YDLIDAR_MOTRO_EN, HIGH);
                S.println("Now YDLIDAR is scanning ......");
                //vTaskDelay(pdMS_TO_TICKS(1000));
            }else{
                S.println("start YDLIDAR is failed!  Continue........");
            }
        }else{
            S.println("cannot retrieve YDLIDAR health");
        }
    }else{
        S.println("YDLIDAR get DeviceInfo Error!!!");
    }
}