/*
 * Configuration File
 * All configuration constants for the Filament Sensor project
 */

#ifndef CONFIG_H
#define CONFIG_H

// ========== WiFi Configuration ==========
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

// ========== Printer Configuration ==========
extern const char* PRINTER_IP;
extern const int PRINTER_PORT;
extern const char* PRINTER_WS_PATH;

// ========== Filament Sensor Pin Definitions ==========
#define SENSOR_SWITCH 1  // On/Off - Filament present detection
#define SENSOR_MOTION 0  // Motion detection - Filament movement

// ========== Filament Sensor Configuration ==========
#define MOTION_TIMEOUT 3000        // 3 seconds without motion = jam/runout
#define POSITION_CHECK_INTERVAL 500 // Check position change every 500ms
#define MIN_MOVEMENT_THRESHOLD 0.1  // Minimum coordinate change in mm

// ========== WebSocket Configuration ==========
extern const unsigned long STATUS_INTERVAL;  // Request status every 3 seconds
extern const unsigned long PING_INTERVAL;   // Send ping every 50 seconds

#endif // CONFIG_H
