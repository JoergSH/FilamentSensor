/*
 * Configuration Implementation
 * Actual variable definitions
 */

#include "config.h"

// ========== WiFi Configuration ==========
const char* WIFI_SSID = "TP-Link_D7D2";
const char* WIFI_PASSWORD = "1234567890a";

// ========== Printer Configuration ==========
const char* PRINTER_IP = "192.168.1.100";
const int PRINTER_PORT = 80;
const char* PRINTER_WS_PATH = "/websocket";

// ========== WebSocket Configuration ==========
const unsigned long STATUS_INTERVAL = 3000;  // Request status every 3 seconds
const unsigned long PING_INTERVAL = 50000;   // Send ping every 50 seconds
