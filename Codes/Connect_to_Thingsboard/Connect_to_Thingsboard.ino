#include <WiFi.h>
#include <ThingsBoard.h>

// WiFi credentials
constexpr char WIFI_SSID[] = "Senchou";
constexpr char WIFI_PASSWORD[] = "@5qifyddn";

// ThingsBoard credentials
constexpr char TOKEN[] = "rusj7srvima9dacs1nt3";
constexpr char THINGSBOARD_SERVER[] = "eu.thingsboard.cloud";
constexpr uint16_t THINGSBOARD_PORT = 1883U;

// WiFi and MQTT clients
WiFiClient wifiClient;
ThingsBoard tb(wifiClient);

// Timing variables
constexpr uint16_t TELEMETRY_INTERVAL = 2000U;
uint32_t lastTelemetrySend = 0;

// Initialize WiFi connection
void initWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

// Reconnect WiFi if disconnected
bool reconnectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  initWiFi();
  return true;
}

void setup() {
  Serial.begin(115200);
  initWiFi();
}

void loop() {
  if (!reconnectWiFi()) return;

  // Reconnect ThingsBoard if disconnected
  if (!tb.connected()) {
    Serial.print("Connecting to ThingsBoard... ");
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
      Serial.println("Failed!");
      return;
    }
    Serial.println("Connected.");
  }

  // Send random telemetry data every interval
  if (millis() - lastTelemetrySend > TELEMETRY_INTERVAL) {
    lastTelemetrySend = millis();
    
    float temperature = random(20, 35) + random(0, 10) * 0.1; // Random temp
    int humidity = random(30, 80); // Random humidity

    Serial.print("Sending Data -> Temp: ");
    Serial.print(temperature);
    Serial.print("°C, Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    tb.sendTelemetryData("temperature", temperature);
    tb.sendTelemetryData("humidity", humidity);
  }

  tb.loop(); // Keep ThingsBoard connection alive
}
