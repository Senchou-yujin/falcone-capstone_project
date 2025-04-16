#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h>
#include <TinyGPS++.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ArduinoJson.h>

const char* ssid = "Senchou";
const char* password = "@5qifyddn";

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

WebServer server(80);
WebSocketsServer webSocket(81);

// GPS setup
HardwareSerial GPS(1); // RX=16, TX=17
TinyGPSPlus gps;

// MPU6050 setup
Adafruit_MPU6050 mpu;

// Battery monitoring
#define BATTERY_PIN 34

// Alignment control
bool isAligning = false;
unsigned long alignmentStartTime = 0;
unsigned long lastAlignmentUpdate = 0;
const unsigned long ALIGNMENT_UPDATE_INTERVAL = 100; // ms
const unsigned long ALIGNMENT_TIMEOUT = 20000; // 20 seconds

// Store device data
struct Device {
  String id;
  float lat, lng;
  int status; // 0=normal, 1=tilted, 2=flipped
  float temperature;
  int battery; // 0=critical, 1=low, 2=good
  unsigned long lastUpdate;
};
Device devices[3] = {
  { "Falcone2", 0, 0, 0, 0, 0, 0 },
  { "Falcone3", 0, 0, 0, 0, 0, 0 },
  { "Falcone1", 0, 0, 0, 0, 0, 0 }
};

void updateAndBroadcastPositions() {
  String json = "{\"devices\":[";
  for (int i = 0; i < 3; i++) {
    json += "{";
    json += "\"id\":\"" + devices[i].id + "\",";
    json += "\"lat\":" + String(devices[i].lat, 6) + ",";
    json += "\"lng\":" + String(devices[i].lng, 6) + ",";
    json += "\"status\":" + String(devices[i].status) + ",";
    json += "\"temp\":" + String(devices[i].temperature, 1) + ",";
    json += "\"battery\":" + String(devices[i].battery);
    json += "}";
    if (i < 2) json += ",";
  }
  json += "]}";
  webSocket.broadcastTXT(json);
  Serial.println("Broadcast: " + json);
}

void broadcastAlignmentData(float yaw) {
  String alignData = "{\"alignment\":{\"yaw\":" + String(yaw, 2) + "}}";
  webSocket.broadcastTXT(alignData);
}

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

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_TEXT) {
    String message = (char*)payload;
    Serial.printf("Raw Received: %s\n", message.c_str());

    // Enhanced command parsing (handles both "COMMAND" and "TARGET:COMMAND")
    String target = "Falcone1"; // Default target if no prefix
    String command = message;
    
    int colonPos = message.indexOf(':');
    if (colonPos != -1) {
      target = message.substring(0, colonPos);
      command = message.substring(colonPos + 1);
    }

    String myDeviceID = "Falcone1"; // Set this to match your device name
    
    // Handle incoming data from other devices
    if (message.startsWith("{") && message.endsWith("}")) {
      // Parse JSON-like data (e.g., {"id":"Falcone1","lat":12.345678,"lng":98.765432,"status":0,"temp":25.5,"battery":2})
      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, message);
      if (!error) {
        String deviceID = doc["id"];
        for (int i = 0; i < 3; i++) {
          if (devices[i].id == deviceID) {
            devices[i].lat = doc["lat"];
            devices[i].lng = doc["lng"];
            devices[i].status = doc["status"];
            devices[i].temperature = doc["temp"];
            devices[i].battery = doc["battery"];
            devices[i].lastUpdate = millis();
            Serial.printf("Updated data for %s: lat=%.6f, lng=%.6f\n", deviceID.c_str(), devices[i].lat, devices[i].lng);
            break;
          }
        }
        updateAndBroadcastPositions(); // Broadcast updated positions
      } else {
        Serial.println("Failed to parse incoming JSON data");
      }
    }
    // Execute if command is for this device or broadcast to all
    else if (target == myDeviceID || target == "All") {
      Serial.printf("Executing for %s: %s\n", myDeviceID.c_str(), command.c_str());
      
      // Handle movement commands
      if (command == "MOVE_FORWARD") {
        currentMovement = MOVE_FORWARD;
        digitalWrite(IN1_1, HIGH); digitalWrite(IN2_1, LOW);
        digitalWrite(IN3_1, HIGH); digitalWrite(IN4_1, LOW);
        digitalWrite(IN1_2, HIGH); digitalWrite(IN2_2, LOW);
        digitalWrite(IN3_2, HIGH); digitalWrite(IN4_2, LOW);
        //webSocket.broadcastTXT(myDeviceID + ":MOVING_FORWARD");
      }
      else if (command == "MOVE_BACKWARD") {
        currentMovement = MOVE_BACKWARD;
        digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, HIGH);
        digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, HIGH);
        digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, HIGH);
        digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, HIGH);
        //webSocket.broadcastTXT(myDeviceID + ":MOVING_BACKWARD");
      }
      else if (command == "ROTATE_LEFT") {
        currentMovement = ROTATE_LEFT;
        digitalWrite(IN1_1, HIGH); digitalWrite(IN2_1, LOW);
        digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, HIGH);
        digitalWrite(IN1_2, HIGH); digitalWrite(IN2_2, LOW);
        digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, HIGH);
        //webSocket.broadcastTXT(myDeviceID + ":ROTATING_LEFT");
      }
      else if (command == "ROTATE_RIGHT") {
        currentMovement = ROTATE_RIGHT;
        digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, HIGH);
        digitalWrite(IN3_1, HIGH); digitalWrite(IN4_1, LOW);
        digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, HIGH);
        digitalWrite(IN3_2, HIGH); digitalWrite(IN4_2, LOW);
        //webSocket.broadcastTXT(myDeviceID + ":ROTATING_RIGHT");
      }
      // Enhanced STOP command handling
      else if (command == "STOP") {
        currentMovement = HOLD_POSITION;
        isAligning = false;
        digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, LOW);
        digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, LOW);
        digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, LOW);
        digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, LOW);
        //webSocket.broadcastTXT(myDeviceID + ":STOPPED");
        Serial.println("Motors completely stopped");
      }
      // Alignment control
      else if (command == "START_ALIGN") {
        isAligning = true;
        currentMovement = HOLD_POSITION;
        alignmentStartTime = millis();
        digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, LOW);
        digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, LOW);
        digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, LOW);
        digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, LOW);
        //webSocket.broadcastTXT(myDeviceID + ":ALIGNING");
        Serial.println("Alignment started - motors stopped");

        // Pause GPS updates
        while (GPS.available() > 0) {
          GPS.read(); // Discard GPS data
        }
      }
      else {
        Serial.println("Unknown command: " + command);
      }
    }
    // Forward commands to other devices
    else if (target == "Falcone2" || target == "Falcone3") {
      webSocket.broadcastTXT(message);
      Serial.println("Command forwarded to " + target);
    }
    else {
      Serial.println("Unknown target: " + target);
    }
  }
}

int readBatteryStatus() {
  int value = analogRead(BATTERY_PIN);
  float voltage = value * (3.3 / 4095.0) * 2;
  if (voltage < 3.3) return 0;
  else if (voltage < 3.6) return 1;
  else return 2;
}

void updateSelfData() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  float angleX = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
  float angleY = atan2(-a.acceleration.x, sqrt(a.acceleration.y*a.acceleration.y + a.acceleration.z*a.acceleration.z)) * 180 / PI;
  
  int status = 0;
  if (abs(angleX) > 45 || abs(angleY) > 45) status = 1;
  if (abs(angleX) > 135 || abs(angleY) > 135) status = 2;

  devices[2].status = status;
  devices[2].temperature = temp.temperature;
  devices[2].battery = readBatteryStatus();
  devices[2].lastUpdate = millis();
}

void setupMotors() {
  pinMode(ENA_1, OUTPUT); pinMode(IN1_1, OUTPUT); pinMode(IN2_1, OUTPUT);
  pinMode(ENB_1, OUTPUT); pinMode(IN3_1, OUTPUT); pinMode(IN4_1, OUTPUT);
  pinMode(ENA_2, OUTPUT); pinMode(IN1_2, OUTPUT); pinMode(IN2_2, OUTPUT);
  pinMode(ENB_2, OUTPUT); pinMode(IN3_2, OUTPUT); pinMode(IN4_2, OUTPUT);
  stopAllMotors();
}

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  GPS.begin(9600, SERIAL_8N1, 16, 17);
  analogReadResolution(12);
  setupMotors();

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("WiFi Connected. IP: " + WiFi.localIP().toString());

  server.on("/", HTTP_GET, []() {
    if(!handleFileRead("/index.html")) {
      server.send(404, "text/plain", "File not found");
    }
  });
  
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "File not found");
    }
  });

  server.on("/favicon.ico", HTTP_GET, []() {
    server.send(404, "text/plain", "File not found");
  });

  webSocket.onEvent(webSocketEvent);
  server.begin();
  webSocket.begin();
}

void loop() {
  server.handleClient();
  webSocket.loop();

  // GPS Handling - Only update if not aligning
  if (!isAligning) {
    while (GPS.available() > 0) {
      if (gps.encode(GPS.read())) {
        if (gps.location.isValid()) {
          devices[2].lat = gps.location.lat();
          devices[2].lng = gps.location.lng();
          updateSelfData();
          updateAndBroadcastPositions();
        }
      }
    }
  }
  else {
    // Explicitly clear GPS buffer during alignment
    while (GPS.available() > 0) {
      GPS.read(); // Discard GPS data
    }
  }

  // Alignment Handling
  if (isAligning) {
    if (millis() - alignmentStartTime > ALIGNMENT_TIMEOUT) {
      Serial.println("Alignment timeout - restarting ESP");
      ESP.restart();
    }
    
    // if (millis() - lastAlignmentUpdate >= ALIGNMENT_UPDATE_INTERVAL) {
    //   sensors_event_t a, g, temp;
    //   mpu.getEvent(&a, &g, &temp);
    //   float yaw = atan2(a.acceleration.y, a.acceleration.x) * 180 / PI;
    //   broadcastAlignmentData(yaw);
    //   lastAlignmentUpdate = millis();
    // }
  }
}