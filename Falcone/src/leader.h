#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h>
#include <TinyGPS++.h>

const char* ssid = "Senchou";
const char* password = "@5qifyddn";

WebServer server(80);
WebSocketsServer webSocket(81);

// GPS setup
HardwareSerial GPS(1); // RX=16, TX=17
TinyGPSPlus gps;

// Store device positions
struct Device {
  String id;
  float lat, lng;
  unsigned long lastUpdate;
};
Device devices[3] = {
  { "Falcone1", 0, 0, 0 },
  { "Falcone2", 0, 0, 0 },
  { "Self", 0, 0, 0 }
};

void updateAndBroadcastPositions() {
  String json = "{\"devices\":[";
  for (int i = 0; i < 3; i++) {
    json += "{\"id\":\"" + devices[i].id + "\",\"lat\":" + String(devices[i].lat, 6) +
            ",\"lng\":" + String(devices[i].lng, 6) + "}";
    if (i < 2) json += ",";
  }
  json += "]}";
  webSocket.broadcastTXT(json);
  Serial.println("Broadcast: " + json);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_TEXT) {
    String data = (char*)payload;
    Serial.println("Received: " + data);

    int commas[2];
    commas[0] = data.indexOf(',');
    commas[1] = data.indexOf(',', commas[0] + 1);

    if (commas[0] != -1 && commas[1] != -1) {
      String id = data.substring(0, commas[0]);
      float lat = data.substring(commas[0] + 1, commas[1]).toFloat();
      float lng = data.substring(commas[1] + 1).toFloat();

      for (int i = 0; i < 2; i++) { // Only update Falcone1/2 (skip Self)
        if (devices[i].id == id) {
          devices[i].lat = lat;
          devices[i].lng = lng;
          devices[i].lastUpdate = millis();
          break;
        }
      }
      updateAndBroadcastPositions();
    }
  }
}

void setup() {
  Serial.begin(115200);
  GPS.begin(9600, SERIAL_8N1, 16, 17);

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

  // Read and process GPS data
  while (GPS.available() > 0) {
    if (gps.encode(GPS.read())) {
      if (gps.location.isValid()) {
        devices[2].lat = gps.location.lat();
        devices[2].lng = gps.location.lng();
        devices[2].lastUpdate = millis();
        
        Serial.print("Self Updated: ");
        Serial.print(devices[2].lat, 6);
        Serial.print(", ");
        Serial.println(devices[2].lng, 6);
        
        updateAndBroadcastPositions();
      }
    }
  }

}