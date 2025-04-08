#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h>
#include <TinyGPS++.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

const char* ssid = "Senchou";
const char* password = "@5qifyddn";

WebServer server(80);
WebSocketsServer webSocket(81);

// GPS setup
HardwareSerial GPS(1); // RX=16, TX=17
TinyGPSPlus gps;

// MPU6050 setup
Adafruit_MPU6050 mpu;

// Battery monitoring (simulated - connect actual battery monitoring hardware)
#define BATTERY_PIN 34

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
  { "Falcone1", 0, 0, 0, 0, 0, 0 },
  { "Falcone2", 0, 0, 0, 0, 0, 0 },
  { "Self", 0, 0, 0, 0, 0, 0 }
};

void stopAllMotors() {
  for(int i=0; i<4; i++) setMotorSpeed(i, 0);
}

void setupMotors() {
  pinMode(ENA_1, OUTPUT); pinMode(IN1_1, OUTPUT); pinMode(IN2_1, OUTPUT);
  pinMode(ENB_1, OUTPUT); pinMode(IN3_1, OUTPUT); pinMode(IN4_1, OUTPUT);
  pinMode(ENA_2, OUTPUT); pinMode(IN1_2, OUTPUT); pinMode(IN2_2, OUTPUT); 
  pinMode(ENB_2, OUTPUT); pinMode(IN3_2, OUTPUT); pinMode(IN4_2, OUTPUT);
  stopAllMotors();
}

// Remove all speed-related variables and functions
// Replace with simple directional control

void setMotorDirection(uint8_t motor, bool forward) {
  switch(motor) {
    case 0: // Motor A
      digitalWrite(IN1_1, forward); 
      digitalWrite(IN2_1, !forward);
      break;
    case 1: // Motor B
      digitalWrite(IN3_1, forward);
      digitalWrite(IN4_1, !forward);
      break;
    case 2: // Motor C
      digitalWrite(IN1_2, forward);
      digitalWrite(IN2_2, !forward);
      break;
    case 3: // Motor D
      digitalWrite(IN3_2, forward);
      digitalWrite(IN4_2, !forward);
      break;
  }
}

void stopAllMotors() {
  for(int i=0; i<4; i++) {
    digitalWrite(IN1_1 + i, LOW); // Turn off all direction pins
  }
}

void executeFormationMovement() {
  if (isAligning) {
    // Alignment uses simple left/right correction
    float yawError = calculateYawError();
    if(yawError > 5) {
      // Turn right
      setMotorDirection(0, false); // Left back
      setMotorDirection(1, true);  // Right forward
    } 
    else if(yawError < -5) {
      // Turn left
      setMotorDirection(0, true);  // Left forward
      setMotorDirection(1, false); // Right back
    }
    return;
  }

  switch(currentMovement) {
    case MOVE_FORWARD:
      for(int i=0; i<4; i++) setMotorDirection(i, true);
      break;
    case MOVE_BACKWARD:
      for(int i=0; i<4; i++) setMotorDirection(i, false);
      break;
    case ROTATE_LEFT:
      setMotorDirection(0, true);   // Left forward
      setMotorDirection(1, false);  // Right backward
      setMotorDirection(2, true);   // Left forward
      setMotorDirection(3, false);  // Right backward
      break;
    case ROTATE_RIGHT:
      setMotorDirection(0, false);  // Left backward
      setMotorDirection(1, true);   // Right forward
      setMotorDirection(2, false);  // Left backward
      setMotorDirection(3, true);   // Right forward
      break;
    case HOLD_POSITION:
      stopAllMotors();
      break;
  }
}

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

void handleStructuredData(const String& data) {
  // Parse the data string: "ID,lat,lng,status,temp,battery"
  int commas[5];
  int index = 0;
  for (int i = 0; i < data.length() && index < 5; i++) {
    if (data.charAt(i) == ',') {
      commas[index++] = i;
    }
  }

  if (index == 5) { // We found all 5 commas
    String id = data.substring(0, commas[0]);
    float lat = data.substring(commas[0] + 1, commas[1]).toFloat();
    float lng = data.substring(commas[1] + 1, commas[2]).toFloat();
    int status = data.substring(commas[2] + 1, commas[3]).toInt();
    float temp = data.substring(commas[3] + 1, commas[4]).toFloat();
    int battery = data.substring(commas[4] + 1).toInt();

    for (int i = 0; i < 2; i++) { // Only update Falcone1/2 (skip Self)
      if (devices[i].id == id) {
        devices[i].lat = lat;
        devices[i].lng = lng;
        devices[i].status = status;
        devices[i].temperature = temp;
        devices[i].battery = battery;
        devices[i].lastUpdate = millis();
        break;
      }
    }
    updateAndBroadcastPositions();
  }
}

int readBatteryStatus() {
  // Read battery voltage and return status
  int value = analogRead(BATTERY_PIN);
  float voltage = value * (3.3 / 4095.0) * 2; // Adjust for voltage divider
  
  if (voltage < 3.3) return 0;     // Critical
  else if (voltage < 3.6) return 1; // Low
  else return 2;                    // Good
}

void updateSelfData() {
  // Get MPU6050 data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  // Calculate tilt angles
  float angleX = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
  float angleY = atan2(-a.acceleration.x, sqrt(a.acceleration.y*a.acceleration.y + a.acceleration.z*a.acceleration.z)) * 180 / PI;
  
  // Determine status (0=normal, 1=tilted, 2=flipped)
  int status = 0;
  if (abs(angleX) > 45 || abs(angleY) > 45) status = 1;
  if (abs(angleX) > 135 || abs(angleY) > 135) status = 2;

  // Update self data
  devices[2].status = status;
  devices[2].temperature = temp.temperature;
  devices[2].battery = readBatteryStatus();
  devices[2].lastUpdate = millis();
}

void setup() {
  Serial.begin(115200);
  GPS.begin(9600, SERIAL_8N1, 16, 17);
  analogReadResolution(12);
  setupMotors(); // Initialize motor control

  // Initialize MPU6050
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
    File file = SPIFFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  webSocket.onEvent(webSocketEvent);
  server.begin();
  webSocket.begin();
}

void loop() {
  server.handleClient();
  webSocket.loop();
  executeFormationMovement();

  // Read and process GPS data
  while (GPS.available() > 0) {
    if (gps.encode(GPS.read())) {
      if (gps.location.isValid()) {
        devices[2].lat = gps.location.lat();
        devices[2].lng = gps.location.lng();
        updateSelfData();
        
        // Serial.print("Self Updated: ");
        // Serial.print(devices[2].lat, 6);
        // Serial.print(", ");
        // Serial.print(devices[2].lng, 6);
        // Serial.print(" | Status: ");
        // Serial.print(devices[2].status);
        // Serial.print(" | Temp: ");
        // Serial.print(devices[2].temperature);
        // Serial.print(" | Battery: ");
        // Serial.println(devices[2].battery);
        
        updateAndBroadcastPositions();
      }
    }
  }
}