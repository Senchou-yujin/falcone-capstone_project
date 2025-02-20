#include <TinyGPS++.h>
#include <SoftwareSerial.h>

int RXPin = 16;
int TXPin = 17;
int GPSBaud = 9600;

TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(GPSBaud);
}

void loop() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      sendJSON();
    }
  }
}

void sendJSON() {
  if (gps.location.isValid()) {
    Serial.print("{\"latitude\": ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", \"longitude\": ");
    Serial.print(gps.location.lng(), 6);
    Serial.println("}");
  } else {
    Serial.println("{\"error\": \"GPS signal not available\"}");
  }
  delay(1000);
}
