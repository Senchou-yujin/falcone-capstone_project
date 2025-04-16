#include <WiFi.h>
#include <WebSocketsClient.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

const char* ssid = "Senchou";
const char* password = "@5qifyddn";
const char* serverIP = "192.168.7.152"; // Leader IP
const int serverPort = 81; // Leader WebSocket port

const String deviceID = "Falcone1"; // Change to "Falcone2" for second follower

HardwareSerial gpsSerial(1); // RX=16, TX=17
TinyGPSPlus gps;
Adafruit_MPU6050 mpu;

WebSocketsClient webSocket;
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 4000; // Send every 4 seconds

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected from server");
      break;
    case WStype_CONNECTED:
      Serial.println("Connected to server");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  webSocket.begin(serverIP, serverPort, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();

  // Read GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (millis() - lastSendTime >= SEND_INTERVAL) {
    if (gps.location.isValid()) {
      // Get MPU6050 data
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      
      // Calculate tilt angles (simplified)
      float angleX = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
      float angleY = atan2(-a.acceleration.x, sqrt(a.acceleration.y*a.acceleration.y + a.acceleration.z*a.acceleration.z)) * 180 / PI;
      
      // Determine status (0=normal, 1=tilted, 2=flipped)
      int status = 0;
      if (abs(angleX) > 45 || abs(angleY) > 45) status = 1;
      if (abs(angleX) > 135 || abs(angleY) > 135) status = 2;

      // Format: "ID,lat,lng,accelX,accelY,accelZ,temp,status"
      String data = deviceID + "," + 
                   String(gps.location.lat(), 6) + "," +
                   String(gps.location.lng(), 6) + "," +
                   String(a.acceleration.x, 2) + "," +
                   String(a.acceleration.y, 2) + "," +
                   String(a.acceleration.z, 2) + "," +
                   String(temp.temperature, 1) + "," +
                   String(status);
      
      webSocket.sendTXT(data);
      Serial.println("Sent: " + data);
    } else {
      Serial.println("GPS: No valid fix");
    }
    lastSendTime = millis();
  }
}