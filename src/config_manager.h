/*
 * Configuration Manager
 * Handles persistent configuration storage and WiFi setup
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>

// Configuration structure
struct SystemConfig {
  char wifiSSID[64];
  char wifiPassword[64];
  char printerIP[16];
  int printerPort;
  bool configured;
};

// Initialize configuration system
void initConfigManager();

// Load configuration from flash
bool loadConfig();

// Save configuration to flash
void saveConfig();

// Get current configuration
SystemConfig& getConfig();

// Check if system is configured
bool isConfigured();

// Start WiFi Access Point for setup
void startConfigPortal();

// Stop config portal and connect to WiFi
void stopConfigPortal();

// Update WiFi credentials
void updateWiFiConfig(const char* ssid, const char* password);

// Update printer configuration
void updatePrinterConfig(const char* ip, int port);

#endif // CONFIG_MANAGER_H
