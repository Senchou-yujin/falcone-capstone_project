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
const char* serverIP = "192.168.126.152"; // Leader IP
const int serverPort = 81; // WebSocket port

// Motor Driver Pins
#define ENA_1 5   // Motor A PWM 
#define IN1_1 13  // Motor A direction
#define IN2_1 12  
#define ENB_1 18  // Motor B PWM
#define IN3_1 14  
#define IN4_1 27  
#define ENA_2 19  // Motor C PWM
#define IN1_2 26  
#define IN2_2 25  
#define ENB_2 21  // Motor D PWM
#define IN3_2 33  
#define IN4_2 32  

// Movement states
enum MovementState {
  HOLD_POSITION,
  MOVE_FORWARD,
  MOVE_BACKWARD, 
  ROTATE_LEFT,
  ROTATE_RIGHT
};
MovementState currentMovement = HOLD_POSITION;

const String deviceID = "Falcone2"; // Change for each follower

#define BATTERY_PIN 34

// GPS and MPU setup  
HardwareSerial gpsSerial(1);  // Use UART1 for GPS
TinyGPSPlus gps;
Adafruit_MPU6050 mpu;

WebSocketsClient webSocket;
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 2000;

// Alignment control
bool isAligning = false;
unsigned long alignmentStartTime = 0;
unsigned long lastAlignmentUpdate = 0;
const unsigned long ALIGNMENT_UPDATE_INTERVAL = 100; // ms
const unsigned long ALIGNMENT_TIMEOUT = 20000; // 20 seconds

// Stop all motors
void stopAllMotors() {
  static bool alreadyStopped = false;
  
  if (!alreadyStopped) {
    Serial.println("Stopping All Motors");
    digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, LOW);
    digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, LOW);
    digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, LOW);
    digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, LOW);
    alreadyStopped = true;
  }
}

void setupMotors() {
  pinMode(ENA_1, OUTPUT); pinMode(IN1_1, OUTPUT); pinMode(IN2_1, OUTPUT);
  pinMode(ENB_1, OUTPUT); pinMode(IN3_1, OUTPUT); pinMode(IN4_1, OUTPUT);
  pinMode(ENA_2, OUTPUT); pinMode(IN1_2, OUTPUT); pinMode(IN2_2, OUTPUT);
  pinMode(ENB_2, OUTPUT); pinMode(IN3_2, OUTPUT); pinMode(IN4_2, OUTPUT);
  stopAllMotors();
}

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
      String message = String((char*)payload);

      // Parse the command and target
      String target = "Falcone2"; // Default target for this follower
      String command = message;

      int colonPos = message.indexOf(':');
      if (colonPos != -1) {
        target = message.substring(0, colonPos);
        command = message.substring(colonPos + 1);
      }

      String myDeviceID = "Falcone2"; // Set this to match the follower's device name

      // Execute commands if they are for this device or broadcast to all
      if (target == myDeviceID || target == "All") {
        Serial.printf("Executing for %s: %s\n", myDeviceID.c_str(), command.c_str());

        if (command == "MOVE_FORWARD") {
          digitalWrite(IN1_1, HIGH); digitalWrite(IN2_1, LOW);
          digitalWrite(IN3_1, HIGH); digitalWrite(IN4_1, LOW);
          digitalWrite(IN1_2, HIGH); digitalWrite(IN2_2, LOW);
          digitalWrite(IN3_2, HIGH); digitalWrite(IN4_2, LOW);
          Serial.println("Moving Forward");
        }
        else if (command == "MOVE_BACKWARD") {
          digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, HIGH);
          digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, HIGH);
          digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, HIGH);
          digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, HIGH);
          Serial.println("Moving Backward");
        }
        else if (command == "ROTATE_LEFT") {
          digitalWrite(IN1_1, HIGH); digitalWrite(IN2_1, LOW);
          digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, HIGH);
          digitalWrite(IN1_2, HIGH); digitalWrite(IN2_2, LOW);
          digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, HIGH);
          Serial.println("Rotating Left");
        }
        else if (command == "ROTATE_RIGHT") {
          digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, HIGH);
          digitalWrite(IN3_1, HIGH); digitalWrite(IN4_1, LOW);
          digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, HIGH);
          digitalWrite(IN3_2, HIGH); digitalWrite(IN4_2, LOW);
          Serial.println("Rotating Right");
        }
        else if (command == "STOP") {
          digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, LOW);
          digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, LOW);
          digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, LOW);
          digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, LOW);
          Serial.println("Motors Stopped");
        }
        else {
          Serial.println("Unknown command: " + command);
        }
      }
      break;
  }
}

int readBatteryStatus() {
  int value = analogRead(BATTERY_PIN);
  float voltage = value * (3.3 / 4095.0) * ((20.0 + 10.0) / 10.0); // Adjust multiplier for 20k and 10k resistors

  if (voltage < 3.3) return 0;     // Critical
  else if (voltage < 3.6) return 1; // Low
  else return 2;                    // Good
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  // RX, TX
  analogReadResolution(12);
  setupMotors();

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

  // Process GPS data if available
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (millis() - lastSendTime >= SEND_INTERVAL) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float angleX = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
    float angleY = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;

    int status = 0;
    if (abs(angleX) > 45 || abs(angleY) > 45) status = 1;
    if (abs(angleX) > 135 || abs(angleY) > 135) status = 2;

    int battery = readBatteryStatus();

    String jsonData = "{\"id\":\"" + deviceID + "\",";
    if (gps.location.isValid()) {
      jsonData += "\"lat\":" + String(gps.location.lat(), 6) + 
                  ",\"lng\":" + String(gps.location.lng(), 6) + ",";
    } else {
      jsonData += "\"lat\":null,\"lng\":null,";
      Serial.println("GPS: No valid fix");
    }
    jsonData += "\"status\":" + String(status) + 
                ",\"temp\":" + String(temp.temperature) + 
                ",\"battery\":" + String(battery) + "}";

    webSocket.sendTXT(jsonData);
    Serial.println("Sent: " + jsonData);

    lastSendTime = millis();
  }
}