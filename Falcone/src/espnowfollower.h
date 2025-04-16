#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Define MAC address of the leader device
uint8_t leaderMac[] = {0xCC, 0xDB, 0xA7, 0x93, 0xB7, 0x00};

// Structure to send data
typedef struct message_struct {
  char id[10];
  float lat;
  float lng;
  char status[20];
  float temp;
  float battery;  // Now sending actual battery voltage
} message_struct;

message_struct myData;

const char* deviceID = "Falcone2"; // Set your unique ID
#define BATTERY_PIN 34

// GPS and MPU setup
TinyGPSPlus gps;
Adafruit_MPU6050 mpu;
HardwareSerial GPS(1); // Using Serial1 for GPS on pins 16,17

// ESP-NOW send callback
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
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
  GPS.begin(9600, SERIAL_8N1, 16, 17);
  analogReadResolution(12);

  WiFi.mode(WIFI_STA); // Required for ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(onDataSent); // Register send callback

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }

  Serial.println("MPU6050 Found!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, leaderMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.println("Peer added successfully");
  } else {
    Serial.println("Failed to add peer");
  }

  // Set device ID once
  memset(&myData, 0, sizeof(myData));
  strcpy(myData.id, deviceID);

  Serial.println("Follower ready to send data");
}

void loop() {
  // Bypass GPS process and use hardcoded latitude and longitude
  float hardcodedLat = 14.810054; 
  float hardcodedLng = 120.885328; 
  // Get MPU data
  sensors_event_t a, g, temp;
  if (mpu.getEvent(&a, &g, &temp)) {
    float angleX = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
    float angleY = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;
  
    int status = 0;
    if (abs(angleX) > 45 || abs(angleY) > 45) status = 1;
    if (abs(angleX) > 135 || abs(angleY) > 135) status = 2;
  
    float batteryVoltage = readBatteryStatus();
  
    // Populate data
    strcpy(myData.id, deviceID);
    myData.lat = hardcodedLat;
    myData.lng = hardcodedLng;
    myData.temp = temp.temperature;
    myData.battery = batteryVoltage;
    snprintf(myData.status, sizeof(myData.status), "%d", status);
  
    // Send data
    esp_err_t result = esp_now_send(leaderMac, (uint8_t*)&myData, sizeof(myData));
    if (result == ESP_OK) {
    Serial.println("=== Sent Data ===");
    Serial.print("ID: ");
    Serial.println(myData.id);
    Serial.print("Latitude: ");
    Serial.println(myData.lat, 6);
    Serial.print("Longitude: ");
    Serial.println(myData.lng, 6);
    Serial.print("Status: ");
    Serial.println(myData.status);
    Serial.print("Temperature: ");
    Serial.println(myData.temp, 2);
    Serial.print("Battery: ");
    Serial.println(myData.battery, 2);
    } else {
    Serial.println("Error sending data");
    }
  } else {
    Serial.println("Failed to get MPU6050 data");
  }
  
  delay(5000); // Send every 5 seconds
  }






// void loop() {
//   // Continuously read GPS data
//   while (GPS.available()) {
//     gps.encode(GPS.read());
//   }

//   if (gps.location.isValid()) {
//     Serial.println("Valid GPS fix");

//     // Get MPU data
//     sensors_event_t a, g, temp;
//     if (mpu.getEvent(&a, &g, &temp)) {
//       float angleX = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
//       float angleY = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180 / PI;

//       int status = 0;
//       if (abs(angleX) > 45 || abs(angleY) > 45) status = 1;
//       if (abs(angleX) > 135 || abs(angleY) > 135) status = 2;

//       float batteryVoltage = readBatteryVoltage();

//       // Populate data
//       myData.lat = gps.location.lat();
//       myData.lng = gps.location.lng();
//       myData.temp = temp.temperature;
//       myData.battery = batteryVoltage;
//       snprintf(myData.status, sizeof(myData.status), "%d", status);

//       // Send data
//       esp_err_t result = esp_now_send(leaderMac, (uint8_t*)&myData, sizeof(myData));
//       if (result == ESP_OK) {
//         Serial.println("Data sent successfully");
//       } else {
//         Serial.println("Error sending data");
//       }
//     } else {
//       Serial.println("Failed to get MPU6050 data");
//     }
//   } else {
//     Serial.println("GPS: No valid fix");
//   }

//   delay(5000); // Send every 5 seconds
// }
    