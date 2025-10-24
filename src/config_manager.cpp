/*
 * Configuration Manager Implementation
 */

#include "config_manager.h"
#include "config.h"
#include <Preferences.h>
#include <WiFi.h>
#include <esp_wifi.h>

static Preferences configPrefs;
static SystemConfig currentConfig;

void initConfigManager() {
  // Initialize with defaults
  strncpy(currentConfig.wifiSSID, "", sizeof(currentConfig.wifiSSID));
  strncpy(currentConfig.wifiPassword, "", sizeof(currentConfig.wifiPassword));
  strncpy(currentConfig.printerIP, "192.168.1.100", sizeof(currentConfig.printerIP));
  currentConfig.printerPort = 80;
  currentConfig.configured = false;

  // Try to load from flash
  if (loadConfig()) {
    Serial.println("[CONFIG] Configuration loaded from flash");
  } else {
    Serial.println("[CONFIG] No configuration found, using defaults");
  }
}

bool loadConfig() {
  configPrefs.begin("system", true);  // true = read-only

  bool configured = configPrefs.getBool("configured", false);

  if (configured) {
    configPrefs.getString("wifiSSID", currentConfig.wifiSSID, sizeof(currentConfig.wifiSSID));
    configPrefs.getString("wifiPass", currentConfig.wifiPassword, sizeof(currentConfig.wifiPassword));
    configPrefs.getString("printerIP", currentConfig.printerIP, sizeof(currentConfig.printerIP));
    currentConfig.printerPort = configPrefs.getInt("printerPort", 80);
    currentConfig.configured = true;

    Serial.println("[CONFIG] Loaded settings:");
    Serial.printf("[CONFIG]   WiFi SSID: %s\n", currentConfig.wifiSSID);
    Serial.printf("[CONFIG]   Printer IP: %s:%d\n", currentConfig.printerIP, currentConfig.printerPort);
  }

  configPrefs.end();
  return configured;
}

void saveConfig() {
  configPrefs.begin("system", false);  // false = read-write

  configPrefs.putBool("configured", true);
  configPrefs.putString("wifiSSID", currentConfig.wifiSSID);
  configPrefs.putString("wifiPass", currentConfig.wifiPassword);
  configPrefs.putString("printerIP", currentConfig.printerIP);
  configPrefs.putInt("printerPort", currentConfig.printerPort);

  configPrefs.end();

  currentConfig.configured = true;
  Serial.println("[CONFIG] Configuration saved to flash");
}

SystemConfig& getConfig() {
  return currentConfig;
}

bool isConfigured() {
  return currentConfig.configured;
}

void startConfigPortal() {
  Serial.println("[CONFIG] Starting configuration portal...");
  Serial.println("[CONFIG] ESP32 Chip Info:");
  Serial.printf("[CONFIG]   Model: ESP32-C3\n");
  Serial.printf("[CONFIG]   Cores: %d\n", ESP.getChipCores());
  Serial.printf("[CONFIG]   Chip Revision: %d\n", ESP.getChipRevision());

  // Stop any existing WiFi connection
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(1000);

  Serial.println("[CONFIG] Starting Access Point...");

  // Start Access Point - TRY WITHOUT PASSWORD FIRST FOR TESTING
  const char* apSSID = "ESP32-Setup";  // Shorter, simpler name
  const char* apPassword = "";  // NO PASSWORD for maximum compatibility

  // Set WiFi mode BEFORE softAP
  WiFi.mode(WIFI_AP);
  delay(100);

  // Reduce TX power to avoid brown-out issues (especially when powered via USB)
  Serial.println("[CONFIG] Setting WiFi TX power to LOW for stability...");
  WiFi.setTxPower(WIFI_POWER_8_5dBm);  // Lowest stable power (was WIFI_POWER_19_5dBm default)

  // Try to start AP with minimal parameters
  Serial.println("[CONFIG] Attempting to start AP (open network)...");
  bool apStarted = WiFi.softAP(apSSID);  // Simplest possible call

  if (!apStarted) {
    Serial.println("[CONFIG] ✗ First attempt failed!");
    Serial.println("[CONFIG] Trying with channel 6...");
    apStarted = WiFi.softAP(apSSID, "", 6);
  }

  if (!apStarted) {
    Serial.println("[CONFIG] ✗ Second attempt failed!");
    Serial.println("[CONFIG] Trying with channel 11...");
    apStarted = WiFi.softAP(apSSID, "", 11);
  }

  delay(1000);

  // Check status
  Serial.println("[CONFIG] ");
  Serial.println("[CONFIG] WiFi AP Status:");
  Serial.printf("[CONFIG]   Mode: %d (should be 2 for AP)\n", WiFi.getMode());
  Serial.printf("[CONFIG]   softAP started: %s\n", apStarted ? "YES" : "NO");

  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
    // Configure IP address explicitly
    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_IP, gateway, subnet);

    // Get AP info
    IPAddress IP = WiFi.softAPIP();
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_AP, &conf);

    Serial.println("[CONFIG] ✓ Access Point is running!");
    Serial.printf("[CONFIG]   SSID: %s\n", (char*)conf.ap.ssid);
    Serial.printf("[CONFIG]   Channel: %d\n", conf.ap.channel);
    Serial.printf("[CONFIG]   Max Connections: %d\n", conf.ap.max_connection);
    Serial.printf("[CONFIG]   IP address: %s\n", IP.toString().c_str());
    Serial.printf("[CONFIG]   MAC Address: %s\n", WiFi.softAPmacAddress().c_str());
    Serial.printf("[CONFIG]   TX Power: LOW (for USB power stability)\n");

    // Get number of connected stations
    Serial.printf("[CONFIG]   Connected stations: %d\n", WiFi.softAPgetStationNum());

    Serial.println("[CONFIG] ");
    Serial.println("[CONFIG] ══════════════════════════════════════");
    Serial.println("[CONFIG] 📱 CONNECT TO THIS WIFI:");
    Serial.printf("[CONFIG]    Network: %s\n", apSSID);
    Serial.println("[CONFIG]    Password: (none - open network)");
    Serial.println("[CONFIG] ");
    Serial.println("[CONFIG] 🌐 THEN OPEN IN BROWSER:");
    Serial.println("[CONFIG]    http://192.168.4.1");
    Serial.println("[CONFIG] ══════════════════════════════════════");
    Serial.println("[CONFIG] ");
    Serial.println("[CONFIG] If you still don't see the network:");
    Serial.println("[CONFIG]   1. Make sure your device supports 2.4 GHz WiFi");
    Serial.println("[CONFIG]   2. Try turning WiFi off/on on your device");
    Serial.println("[CONFIG]   3. Move closer to the ESP32");
    Serial.println("[CONFIG]   4. Check with a WiFi scanner app");
  } else {
    Serial.println("[CONFIG] ✗✗✗ FATAL ERROR ✗✗✗");
    Serial.println("[CONFIG] Access Point failed to start!");
    Serial.println("[CONFIG] This could indicate a hardware problem.");
    Serial.printf("[CONFIG] Current WiFi Mode: %d\n", WiFi.getMode());
  }
}

void stopConfigPortal() {
  Serial.println("[CONFIG] Stopping configuration portal...");
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
}

void updateWiFiConfig(const char* ssid, const char* password) {
  strncpy(currentConfig.wifiSSID, ssid, sizeof(currentConfig.wifiSSID) - 1);
  strncpy(currentConfig.wifiPassword, password, sizeof(currentConfig.wifiPassword) - 1);
  currentConfig.wifiSSID[sizeof(currentConfig.wifiSSID) - 1] = '\0';
  currentConfig.wifiPassword[sizeof(currentConfig.wifiPassword) - 1] = '\0';

  Serial.printf("[CONFIG] WiFi updated: %s\n", ssid);
  saveConfig();
}

void updatePrinterConfig(const char* ip, int port) {
  strncpy(currentConfig.printerIP, ip, sizeof(currentConfig.printerIP) - 1);
  currentConfig.printerIP[sizeof(currentConfig.printerIP) - 1] = '\0';
  currentConfig.printerPort = port;

  Serial.printf("[CONFIG] Printer updated: %s:%d\n", ip, port);
  saveConfig();
}
