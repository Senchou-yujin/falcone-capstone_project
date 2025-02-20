#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include "UbidotsEsp32Mqtt.h"

#define RXPin 16
#define TXPin 17
#define GPSBaud 9600

const char *UBIDOTS_TOKEN = "BBUS-9GPdqG2AEMYlZh6wCFuJto04xjaRxl";  // Put here your Ubidots TOKEN BBUS-9GPdqG2AEMYlZh6wCFuJto04xjaRxl
const char *WIFI_SSID = "Senchou";      // Wi-Fi SSID
const char *WIFI_PASS = "@5qifyddn";      // Wi-Fi password
const char *DEVICE_LABEL = "falcone1";   // Put here your Device label to which data  will be published

//Topics
const char *DevLatitude = "lat"; 
const char *Devlatitude = "long";

TinyGPSPlus gps;
HardwareSerial gpsSerial(1);  // Use ESP32's hardware serial
Ubidots ubidots(UBIDOTS_TOKEN);

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
  Serial.println("Waiting for GPS signal...");

  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setup();
  ubidots.reconnect();

}

void loop() {
  if (!ubidots.connected())
  {
    ubidots.reconnect();
  }

  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    // Serial.write(c);  // Print raw GPS NMEA data
    gps.encode(c);
  }

  if (gps.location.isValid()) {
    // Serial.print("Latitude: ");
    // Serial.println(gps.location.lat(), 6);
    // Serial.print("Longitude: ");
    // Serial.println(gps.location.lng(), 6);
  } else {
    Serial.println("GPS signal not available");
  }

  // Send latitude and longitude as separate variables
char latStr[20], longStr[20];  
dtostrf(gps.location.lat(), 8, 6, latStr);  
dtostrf(gps.location.lng(), 8, 6, longStr);  

ubidots.add("Longitude", atof(longStr));
ubidots.add("Latitude", atof(latStr));  
Serial.print("Latitude: ");
Serial.println(latStr);
Serial.print("Longitude: ");
Serial.println(longStr);


// Publish data
if (ubidots.publish(DEVICE_LABEL)) {
    Serial.println("GPS data sent to Ubidots!");
} else {
    Serial.println("Failed to send GPS data!");
}

  delay(1000);
}
