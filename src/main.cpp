/*
 * ESP32 Elegoo Centauri Carbon Monitor with Filament Sensor
 * Detects filament runout/jam and auto-pauses print
 *
 * Refactored into modular components for better maintainability
 */

#include <Arduino.h>
#include "config.h"
#include "config_manager.h"
#include "wifi_manager.h"
#include "websocket_client.h"
#include "web_server.h"
#include "printer_status.h"
#include "printer_control.h"
#include "filament_sensor.h"
#include "ota_update.h"

// Timing variables
unsigned long lastStatusRequest = 0;
unsigned long lastPing = 0;
bool inSetupMode = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=================================");
  Serial.println("Centauri Carbon Monitor - ESP32");
  Serial.println("With Filament Runout Detection");
  Serial.println("      & Web Dashboard");
  Serial.println("=================================\n");

  // Initialize configuration manager
  initConfigManager();

  // Initialize OTA update system
  setupOTA();

  // Initialize filament sensor
  setupFilamentSensor();

  // Check if system is configured
  if (!isConfigured()) {
    Serial.println("[MAIN] System not configured!");
    Serial.println("[MAIN] Starting setup portal...");

    // Start Access Point for initial setup
    startConfigPortal();
    inSetupMode = true;

    // Start web server for setup portal
    setupWebServer();

    Serial.println("[MAIN] Setup portal ready!");
    Serial.println("[MAIN] Connect to 'Centauri-Monitor-Setup' and visit http://192.168.4.1");
  } else {
    Serial.println("[MAIN] System configured, starting normal operation...");

    // Connect to WiFi
    connectWiFi();

    // Only setup WebSocket and normal web server if WiFi connected
    if (isWiFiConnected()) {
      setupWebSocket();
      setupWebServer();
      Serial.println("[MAIN] System ready!");
    } else {
      Serial.println("[MAIN] WiFi connection failed! Starting setup portal...");
      startConfigPortal();
      inSetupMode = true;
      setupWebServer();
    }
  }
}

void loop() {
  // If in setup mode, just wait for configuration
  if (inSetupMode) {
    delay(100);
    return;
  }

  // Normal operation
  // Process WebSocket communication
  processWebSocket();

  // Send periodic status requests
  if (millis() - lastStatusRequest > STATUS_INTERVAL) {
    requestStatus();
    lastStatusRequest = millis();
  }

  // Send periodic ping
  if (millis() - lastPing > PING_INTERVAL) {
    sendPing();
    lastPing = millis();
  }

  // Check filament sensor
  checkFilamentSensor();
}
