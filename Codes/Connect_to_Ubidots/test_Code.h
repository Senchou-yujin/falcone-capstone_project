#include "UbidotsEsp32Mqtt.h"
#include <stdlib.h>

const char *UBIDOTS_TOKEN = "BBUS-9GPdqG2AEMYlZh6wCFuJto04xjaRxl";  // Put here your Ubidots TOKEN BBUS-9GPdqG2AEMYlZh6wCFuJto04xjaRxl
const char *WIFI_SSID = "Senchou";      // Wi-Fi SSID
const char *WIFI_PASS = "@5qifyddn";      // Wi-Fi password
const char *DEVICE_LABEL = "falcone1";   // Put here your Device label to which data  will be published

//Topics
const char *VARIABLE_LABEL = "randomNumbers"; /// Put here your Variable label to which data  will be published

const int PUBLISH_FREQUENCY = 5000;
unsigned long timer;

Ubidots ubidots(UBIDOTS_TOKEN);

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);

  if (ubidots.connectToWifi(WIFI_SSID, WIFI_PASS)) {
    Serial.println("Wi-Fi Connected!");
  } else {
    Serial.println("Wi-Fi Connection Failed!");
  }

  ubidots.setup();
  Serial.println("Connecting to Ubidots MQTT...");
  if (ubidots.reconnect()) {
    Serial.println("Connected to Ubidots MQTT!");
  } else {
    Serial.println("Failed to connect to Ubidots MQTT!");
  }
  timer = millis();
}

void loop() {
  if (!ubidots.connected()) ubidots.reconnect();
  
  if (millis() - timer > PUBLISH_FREQUENCY) {
    ubidots.add(VARIABLE_LABEL, rand() % 100); // Send random values between 0-99
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }
  
  ubidots.loop();
}
