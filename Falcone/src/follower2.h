#include <WiFi.h>
#include <WebSocketsClient.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

const char* ssid = "Senchou";
const char* password = "@5qifyddn";
const char* serverIP = "192.168.7.152"; // Leader IP
const int serverPort = 81; // Leader WebSocket port

const String deviceID = "Falcone"; // Change to "Falcone2" for second follower

HardwareSerial gpsSerial(1); // RX=16, TX=17
TinyGPSPlus gps;

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
      // Send data in format: "DeviceID,lat,lng" (no altitude)
      String data = deviceID + "," + 
                   String(gps.location.lat(), 6) + "," +
                   String(gps.location.lng(), 6);
      
      webSocket.sendTXT(data);
      Serial.println("Sent: " + data);
    } else {
      Serial.println("GPS: No valid fix");
    }
    lastSendTime = millis();
  }
}