#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  
  // Get the MAC address
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  
  // Print the MAC address
  Serial.print("ESP32 MAC Address: ");
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X", 
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println();
}

void loop() {
  // Nothing to do here
}