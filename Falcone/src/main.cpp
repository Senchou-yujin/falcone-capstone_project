#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

const char* ssid = "Senchou";
const char* password = "@5qifyddn";
const char* serverName = "http://api.thingspeak.com/update";
String apiKey = "76OSCBNZLVLSBLGO";

void setup() {
    Serial.begin(115200);
    gpsSerial.begin(9600, SERIAL_8N1, 16, 17);
    WiFi.begin(ssid, password);
    
    if (!display.begin(SCREEN_ADDRESS, OLED_RESET)) {
    Serial.println(F("SH1106 allocation failed"));
    for (;;);
    }

    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println(F("GPS Initializing..."));
    display.display();
    
    Serial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED) {
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
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("GPS Data:"));
    
    if (gps.location.isValid()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();
        
        display.print(F("Lat: "));
        display.println(latitude, 6);
        display.print(F("Lng: "));
        display.println(longitude, 6);
        
        if(WiFi.status() == WL_CONNECTED) {
            WiFiClient client;
            HTTPClient http;
            
            http.begin(client, serverName);
            http.addHeader("Content-Type", "application/json");
            
            String httpRequestData = "{\"api_key\":\"" + apiKey + "\",\"field1\":\"" + String(latitude, 6) + "\",\"field2\":\"" + String(longitude, 6) + "\"}";
            int httpResponseCode = http.POST(httpRequestData);
            
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            
            http.end();
        } else {
            Serial.println("WiFi Disconnected");
        }
    } else {
        display.println(F("No Signal"));
    }
    
    display.display();
    delay(10000);
}
