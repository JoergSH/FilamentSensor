/*
 * WiFi Connection Manager Implementation
 */

#include "wifi_manager.h"
#include "config_manager.h"

void connectWiFi() {
  SystemConfig& config = getConfig();

  Serial.printf("Connecting to WiFi: %s\n", config.wifiSSID);
  WiFi.begin(config.wifiSSID, config.wifiPassword);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed!");
  }
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}
