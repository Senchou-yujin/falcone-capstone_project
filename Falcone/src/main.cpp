#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

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
    
    delay(1000);
    
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
    
    if (gps.location.isValid()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();
        
        if(WiFi.status() == WL_CONNECTED) {
            WiFiClient client;
            HTTPClient http;
            
            http.begin(client, serverName);
            http.addHeader("Content-Type", "application/json");
            
            String httpRequestData = "{\"api_key\":\"" + apiKey + "\",\"field1\":\"" + String(latitude, 6) + "\",\"field2\":\"" + String(longitude, 6) + "\"}";
            int httpResponseCode = http.POST(httpRequestData);
            
            Serial.print("Latitude: ");
            Serial.print(latitude, 6);
            Serial.print(" Longitude: ");
            Serial.print(longitude, 6);
            Serial.print(" ");
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            
            http.end();
        } else {
            Serial.println("WiFi Disconnected");
        }
    } else {
        Serial.println("No Signal");
    }
    
    delay(10000);
}
