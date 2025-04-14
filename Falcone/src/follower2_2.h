#include <WiFi.h>
#include <WebSocketsClient.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ArduinoJson.h>

const char* ssid = "Senchou";
const char* password = "@5qifyddn";
const char* serverIP = "192.168.254.152"; // Leader IP
const int serverPort = 81; // WebSocket port

const String deviceID = "Falcone2"; // Change for each follower

#define BATTERY_PIN 34

// GPS and MPU setup
HardwareSerial gpsSerial(1);  // Use UART1 for GPS
TinyGPSPlus gps;
Adafruit_MPU6050 mpu;

WebSocketsClient webSocket;
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 4000;

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected from server");
      break;
    case WStype_CONNECTED:
      Serial.println("Connected to server");
      break;
    case WStype_TEXT:
      Serial.printf("Received: %s\n", payload);
      break;
  }
}

int readBatteryStatus() {
  int value = analogRead(BATTERY_PIN);
  float voltage = value * (3.3 / 4095.0) * 2; // Use correct voltage divider ratio

  if (voltage < 3.3) return 0;     // Critical
  else if (voltage < 3.6) return 1; // Low
  else return 2;                    // Good
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  // RX, TX
  analogReadResolution(12);

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }

  Serial.println("MPU6050 Found!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  webSocket.begin(serverIP, serverPort, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();

  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (millis() - lastSendTime >= SEND_INTERVAL) {
    if (gps.location.isValid()) {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);

      float angleX = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
      float angleY = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;

      int status = 0;
      if (abs(angleX) > 45 || abs(angleY) > 45) status = 1;
      if (abs(angleX) > 135 || abs(angleY) > 135) status = 2;

      int battery = readBatteryStatus();

      DynamicJsonDocument doc(256);
      doc["id"] = deviceID;
      doc["lat"] = gps.location.lat();
      doc["lng"] = gps.location.lng();
      doc["status"] = status;
      doc["temp"] = temp.temperature;
      doc["battery"] = battery;

      String jsonData;
      serializeJson(doc, jsonData);
      webSocket.sendTXT(jsonData);
      Serial.println("Sent: " + jsonData);
    } else {
      Serial.println("GPS: No valid fix");
    }
    lastSendTime = millis();
  }
}
