#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <esp_task_wdt.h>

uint8_t falcone3Mac[] = {0x94, 0x54, 0xC5, 0xB5, 0xF9, 0x74}; // Replace with actual MAC
uint8_t falcone2Mac[] = {0xCC, 0xDB, 0xA7, 0x94, 0x94, 0xF4}; // Replace with actual MAC

#define BATTERY_PIN 34 // Pin for battery voltage measurement
// Structure to store data for all devices
typedef struct message_struct {
  char id[10];
  float lat;
  float lng;
  char status[20];
  float temp;
  float battery;
  char command[20]; // Added command field
} message_struct;

// GPS and MPU setup
TinyGPSPlus gps;
Adafruit_MPU6050 mpu;
HardwareSerial GPS(1); // Using Serial1 for GPS on pins 16,17

message_struct allDevices[3];      // Array to store data for Falcone2 and Falcone3
unsigned long lastReceivedTime = 0;
bool newDataAvailable = false;

void sendToFollowers(const char* deviceId, const char* command) {
    int deviceIndex = -1;
  
    // Find which device in the array matches the deviceId
    for (int i = 0; i < 3; i++) {
      if (strcmp(allDevices[i].id, deviceId) == 0) {
        deviceIndex = i;
        break;
      }
    }
  
    // If device found, set the command and send it
    if (deviceIndex != -1) {
      // Update the command in that specific device's struct
      strncpy(allDevices[deviceIndex].command, command, sizeof(allDevices[deviceIndex].command) - 1);
      allDevices[deviceIndex].command[sizeof(allDevices[deviceIndex].command) - 1] = '\0'; // Ensure null-termination
  
      // Determine which MAC to send to
      esp_err_t result = ESP_OK;
      if (deviceIndex == 1) {
        result = esp_now_send(falcone2Mac, (uint8_t *)&allDevices[deviceIndex], sizeof(message_struct));
      } else if (deviceIndex == 2) {
        result = esp_now_send(falcone3Mac, (uint8_t *)&allDevices[deviceIndex], sizeof(message_struct));
      }
  
      // Result check
      if (result == ESP_OK) {
        Serial.printf("Sent command '%s' to %s successfully\n", command, deviceId);
      } else {
        Serial.printf("Failed to send command '%s' to %s\n", command, deviceId);
      }
    } else {
      Serial.printf("Device ID '%s' not found!\n", deviceId);
    }
  }

// Callback when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingBytes, int len) {
    if (len != sizeof(message_struct)) {
        Serial.println("Received data size mismatch!");
        return;
    }

    message_struct receivedData;
    memcpy(&receivedData, incomingBytes, sizeof(message_struct));
    lastReceivedTime = millis();
    newDataAvailable = true;

    for (int i = 1; i < 3; i++) {
        if (strcmp(receivedData.id, allDevices[i].id) == 0) {
            allDevices[i] = receivedData;
            break;
        }
    }

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}


void printAllDeviceData() {
  Serial.println("=== All Device Data ===");
  for (int i = 0; i < 3; i++) {
    esp_task_wdt_reset(); // Feed the watchdog timer
    Serial.print("ID: ");
    Serial.println(allDevices[i].id);
    Serial.print("Latitude: ");
    Serial.println(allDevices[i].lat, 6);
    Serial.print("Longitude: ");
    Serial.println(allDevices[i].lng, 6);
    Serial.print("Status: ");
    Serial.println(allDevices[i].status);
    Serial.print("Temperature: ");
    Serial.println(allDevices[i].temp);
    Serial.print("Battery: ");
    Serial.println(allDevices[i].battery);
    Serial.println("--------------------");
  }
}

void sendToFollowers() {
    // Send data to Falcone2
    esp_err_t result2 = esp_now_send(falcone2Mac, (uint8_t *)&allDevices[0], sizeof(message_struct));
    if (result2 == ESP_OK) {
      Serial.println("Sent data to Falcone2 successfully");
    } else {
      Serial.println("Error sending to Falcone2");
    }
  
    // Send data to Falcone3
    esp_err_t result3 = esp_now_send(falcone3Mac, (uint8_t *)&allDevices[0], sizeof(message_struct));
    if (result3 == ESP_OK) {
      Serial.println("Sent data to Falcone3 successfully");
    } else {
      Serial.println("Error sending to Falcone3");
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
    WiFi.mode(WIFI_STA);
    analogReadResolution(12);
    GPS.begin(9600, SERIAL_8N1, 16, 17);

    if (!mpu.begin()) {
      Serial.println("Failed to find MPU6050 chip");
      while (1) {
        delay(10);
      }
    }
  
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
  
    // Register callback
    esp_now_register_recv_cb(OnDataRecv);

    esp_now_peer_info_t peerInfo = {};

    memcpy(peerInfo.peer_addr, falcone2Mac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (!esp_now_is_peer_exist(falcone2Mac)) {
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Failed to add Falcone2 as peer");
        }
    }

    memcpy(peerInfo.peer_addr, falcone3Mac, 6);

    if (!esp_now_is_peer_exist(falcone3Mac)) {
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Failed to add Falcone3 as peer");
        }
    }

    // Initialize Falcone1's data
    strcpy(allDevices[0].id, "Falcone1");
    allDevices[0].lat = 0.0;
    allDevices[0].lng = 0.0;
    strcpy(allDevices[0].status, "Unknown");
    allDevices[0].temp = 0.0;
    allDevices[0].battery = 0.0;
  
    // Initialize Falcone2's data
    strcpy(allDevices[1].id, "Falcone2");
    allDevices[1].lat = 0.0;
    allDevices[1].lng = 0.0;
    strcpy(allDevices[1].status, "Unknown");
    allDevices[1].temp = 0.0;
    allDevices[1].battery = 0.0;
  
    // Initialize Falcone3's data
    strcpy(allDevices[2].id, "Falcone3");
    allDevices[2].lat = 0.0;
    allDevices[2].lng = 0.0;
    strcpy(allDevices[2].status, "Unknown");
    allDevices[2].temp = 0.0;
    allDevices[2].battery = 0.0;
  
    // Serial.println("Falcone1 (Leader) ready to receive data");
    // Serial.println("Waiting for transmissions...");
  }

  void loop() {
    static unsigned long lastUpdateTime = 0;
    static unsigned long lastPrintTime = 0;
  
    // Update Falcone1's data every 5 seconds
    if (millis() - lastUpdateTime >= 5000) {
      lastUpdateTime = millis();
  
      // Update Falcone1's data
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
  
        // Populate Falcone1's data
        strcpy(allDevices[0].id, "Falcone1");
        allDevices[0].lat = hardcodedLat;
        allDevices[0].lng = hardcodedLng;
        allDevices[0].temp = temp.temperature;
        allDevices[0].battery = batteryVoltage;
        snprintf(allDevices[0].status, sizeof(allDevices[0].status), "%d", status);
  
        // Debugging output for Falcone1
        // Serial.println("=== Falcone1 Data ===");
        // Serial.print("ID: ");
        // Serial.println(allDevices[0].id);
        // Serial.print("Latitude: ");
        // Serial.println(allDevices[0].lat, 6);
        // Serial.print("Longitude: ");
        // Serial.println(allDevices[0].lng, 6);
        // Serial.print("Status: ");
        // Serial.println(allDevices[0].status);
        // Serial.print("Temperature: ");
        // Serial.println(allDevices[0].temp, 2);
        // Serial.print("Battery: ");
        // Serial.println(allDevices[0].battery, 2);
      } else {
        Serial.println("Failed to get MPU6050 data");
      }
    }
  
    // Print all device data when new data is received
    if (newDataAvailable) {
      newDataAvailable = false;
      printAllDeviceData();
    }
  
    // Print time since last received data every 10 seconds
    if (millis() - lastPrintTime >= 10000) {
      lastPrintTime = millis();
      if (lastReceivedTime > 0) {
        Serial.print("Time since last data: ");
        Serial.print((millis() - lastReceivedTime) / 1000);
        Serial.println(" seconds");
      } else {
        Serial.println("No data received yet");
      }
    }
  }