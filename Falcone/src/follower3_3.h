#include <WiFi.h>
#include <WebSocketsServer.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Adafruit_MPU6050.h>

// WiFi credentials
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

bool isAligning = false;
unsigned long alignmentStartTime = 0;
unsigned long lastAlignmentUpdate = 0;
const unsigned long ALIGNMENT_UPDATE_INTERVAL = 100; // ms
const unsigned long ALIGNMENT_TIMEOUT = 20000; // 20 seconds

// WebSocket server
WebSocketsServer webSocket = WebSocketsServer(81);
// MPU6050 setup
Adafruit_MPU6050 mpu;

// Battery monitoring
#define BATTERY_PIN 34

// GPS setup
TinyGPSPlus gps;
HardwareSerial GPS(1); // Use UART1 for GPS
#define RXD2 16
#define TXD2 17

// Device-specific data
const char* deviceID = "Falcone2"; // Change to "Falcone2" for the second follower


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

  void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    if (type == WStype_CONNECTED) {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("WebSocket client #%u connected from %s\n", num, ip.toString().c_str());
    } else if (type == WStype_DISCONNECTED) {
        Serial.printf("WebSocket client #%u disconnected\n", num);
    } else if (type == WStype_TEXT) {
        String message = (char*)payload;
        Serial.printf("Raw Received: %s\n", message.c_str());

        // Parse the command
        String command = message;

        // Handle movement commands
        if (command == "MOVE_FORWARD") {
            currentMovement = MOVE_FORWARD;
            digitalWrite(IN1_1, HIGH); digitalWrite(IN2_1, LOW);
            digitalWrite(IN3_1, HIGH); digitalWrite(IN4_1, LOW);
            digitalWrite(IN1_2, HIGH); digitalWrite(IN2_2, LOW);
            digitalWrite(IN3_2, HIGH); digitalWrite(IN4_2, LOW);

            // Send JSON response
            String jsonResponse = "{\"id\":\"Self\",\"command\":\"MOVING_FORWARD\"}";
            webSocket.broadcastTXT(jsonResponse);
        } else if (command == "MOVE_BACKWARD") {
            currentMovement = MOVE_BACKWARD;
            digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, HIGH);
            digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, HIGH);
            digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, HIGH);
            digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, HIGH);

            // Send JSON response
            String jsonResponse = "{\"id\":\"Self\",\"command\":\"MOVING_BACKWARD\"}";
            webSocket.broadcastTXT(jsonResponse);
        } else if (command == "ROTATE_LEFT") {
            currentMovement = ROTATE_LEFT;
            digitalWrite(IN1_1, HIGH); digitalWrite(IN2_1, LOW);
            digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, HIGH);
            digitalWrite(IN1_2, HIGH); digitalWrite(IN2_2, LOW);
            digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, HIGH);

            // Send JSON response
            String jsonResponse = "{\"id\":\"Self\",\"command\":\"ROTATING_LEFT\"}";
            webSocket.broadcastTXT(jsonResponse);
        } else if (command == "ROTATE_RIGHT") {
            currentMovement = ROTATE_RIGHT;
            digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, HIGH);
            digitalWrite(IN3_1, HIGH); digitalWrite(IN4_1, LOW);
            digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, HIGH);
            digitalWrite(IN3_2, HIGH); digitalWrite(IN4_2, LOW);

            // Send JSON response
            String jsonResponse = "{\"id\":\"Self\",\"command\":\"ROTATING_RIGHT\"}";
            webSocket.broadcastTXT(jsonResponse);
        } else if (command == "STOP") {
            currentMovement = HOLD_POSITION;
            digitalWrite(IN1_1, LOW); digitalWrite(IN2_1, LOW);
            digitalWrite(IN3_1, LOW); digitalWrite(IN4_1, LOW);
            digitalWrite(IN1_2, LOW); digitalWrite(IN2_2, LOW);
            digitalWrite(IN3_2, LOW); digitalWrite(IN4_2, LOW);

            // Send JSON response
            String jsonResponse = "{\"id\":\"Self\",\"command\":\"STOPPED\"}";
            webSocket.broadcastTXT(jsonResponse);
            delay(1000); // Optional delay to allow the message to be sent before restarting
            ESP.restart(); // Restart the ESP32
        } else {
            Serial.println("Unknown command: " + command);
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

void setup() {
    Serial.begin(115200);
    GPS.begin(9600, SERIAL_8N1, RXD2, TXD2);
    setupMotors();

    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
        while (1) delay(10);
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected. IP: " + WiFi.localIP().toString());

    // Start WebSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket server started");
}

void loop() {
    webSocket.loop();

    // Read GPS data and send it to the WebSocket client
    while (GPS.available() > 0) {
        char c = GPS.read();
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        float angleX = atan2(a.acceleration.y, a.acceleration.z) * 180 / PI;
        float angleY = atan2(-a.acceleration.x, sqrt(a.acceleration.y*a.acceleration.y + a.acceleration.z*a.acceleration.z)) * 180 / PI;
        
        int status = 0;
        if (abs(angleX) > 45 || abs(angleY) > 45) status = 1;
        if (abs(angleX) > 135 || abs(angleY) > 135) status = 2;
        
        if (gps.encode(c)) {
            if (gps.location.isValid() && gps.location.lat() != 0 && gps.location.lng() != 0) {
                String json = "{";
                json += "\"id\":\"" + String(deviceID) + "\",";
                json += "\"lat\":" + String(gps.location.lat(), 6) + ",";
                json += "\"lng\":" + String(gps.location.lng(), 6) + ",";
                json += "\"status\":" + String(status) + ","; // Example status (0 = normal)
                json += "\"temp\":" + String(temp.temperature, 1) + ","; // Example temperature
                json += "\"battery\":" + String(readBatteryStatus());
                json += "}";
        
                webSocket.broadcastTXT(json);
                Serial.println("Broadcasting: " + json);
            } else {
                Serial.println("Invalid GPS data, skipping broadcast.");
            }
        }
    }
}