#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(1);
Adafruit_MPU6050 mpu;

const char* ssid = "Senchou";
const char* password = "@5qifyddn";
const char* serverName = "http://api.thingspeak.com/update";
String apiKey = "0Y7VQODYQGD4N1F9";

void setup() {
    Serial.begin(115200);
    gpsSerial.begin(9600, SERIAL_8N1, 16, 17);
    WiFi.begin(ssid, password);
    
    delay(1000);

    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
        while (1) {
            delay(10);
        }
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float ax = a.acceleration.x;
    float ay = a.acceleration.y;
    float az = a.acceleration.z;

    float angleX = atan2(ay, az) * 180.0 / PI;
    float angleY = atan2(ax, az) * 180.0 / PI;

    Serial.print("Angle X: "); Serial.print(angleX); Serial.print(" degrees");
    Serial.print(", Angle Y: "); Serial.print(angleY); Serial.println(" degrees");

    int statusValue = 0;  // Default: Normal

    if ((abs(angleX) >= 70 && abs(angleX) <= 110) || (abs(angleY) >= 70 && abs(angleY) <= 110)) {  
        Serial.println("WARNING: Cart is HIGHLY TILTED!");
        statusValue = 1;  // Tilted
    } 
    if (abs(angleX) >= 140 || abs(angleY) >= 140) {  
        Serial.println("ALERT: Cart is FLIPPED OVER!");
        statusValue = 2;  // Flipped
    }

    if (gps.location.isValid()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();
        
        if (WiFi.status() == WL_CONNECTED) {
            WiFiClient client;
            HTTPClient http;
            
            http.begin(client, serverName);
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");

            String httpRequestData = "api_key=" + apiKey + 
                                     "&field1=" + String(latitude, 6) + 
                                     "&field2=" + String(longitude, 6) + 
                                     "&field3=" + String(statusValue);
                                     
            int httpResponseCode = http.POST(httpRequestData);
            
            Serial.print("Latitude: ");
            Serial.print(latitude, 6);
            Serial.print(" Longitude: ");
            Serial.print(longitude, 6);
            Serial.print(" Status: ");

            if (statusValue == 0) Serial.print("Normal");
            else if (statusValue == 1) Serial.print("Tilted");
            else if (statusValue == 2) Serial.print("Flipped");

            Serial.print(" HTTP Response code: ");
            Serial.println(httpResponseCode);
            
            http.end();
        } else {
            Serial.println("WiFi Disconnected");
        }
    } else {
        Serial.println("No Signal");
    }
    
    delay(17000);  // 17 seconds transmission
}
