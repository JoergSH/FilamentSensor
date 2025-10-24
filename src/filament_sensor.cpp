/*
 * Filament Sensor Implementation
 */

#include "filament_sensor.h"
#include "config.h"
#include "printer_status.h"
#include "printer_status_codes.h"
#include "printer_control.h"
#include <Preferences.h>

// Preferences namespace
static Preferences preferences;

// Filament Sensor Variables
static volatile unsigned long lastMotionPulse = 0;
static std::atomic<unsigned int> motionPulseCount(0);
static unsigned long lastFilamentCheck = 0;
static unsigned long lastPositionCheck = 0;
static String lastPosition = "";
static bool filamentErrorDetected = false;
static bool autoPauseEnabled = true;
static unsigned long motionTimeout = MOTION_TIMEOUT;  // Default from config.h, but changeable

// Load settings from persistent storage
void loadSensorSettings() {
  preferences.begin("filament", false);  // false = read-write mode

  // Load motion timeout (default: MOTION_TIMEOUT from config.h)
  motionTimeout = preferences.getULong("motionTimeout", MOTION_TIMEOUT);

  // Load auto-pause enabled (default: true)
  autoPauseEnabled = preferences.getBool("autoPause", true);

  preferences.end();

  Serial.println("[SENSOR] Settings loaded from flash:");
  Serial.printf("[SENSOR]   Motion Timeout: %lu ms\n", motionTimeout);
  Serial.printf("[SENSOR]   Auto-Pause: %s\n", autoPauseEnabled ? "enabled" : "disabled");
}

// Save settings to persistent storage
void saveSensorSettings() {
  preferences.begin("filament", false);  // false = read-write mode

  preferences.putULong("motionTimeout", motionTimeout);
  preferences.putBool("autoPause", autoPauseEnabled);

  preferences.end();

  Serial.println("[SENSOR] Settings saved to flash");
}

void setupFilamentSensor() {
  // Load saved settings from flash
  loadSensorSettings();

  pinMode(SENSOR_SWITCH, INPUT_PULLDOWN);
  pinMode(SENSOR_MOTION, INPUT_PULLUP);

  // Attach interrupt for motion detection
  attachInterrupt(digitalPinToInterrupt(SENSOR_MOTION), filamentMotionISR, FALLING);

  Serial.println("[SENSOR] Filament sensor initialized");
  Serial.printf("[SENSOR] Switch Pin: %d, Motion Pin: %d\n", SENSOR_SWITCH, SENSOR_MOTION);
}

void IRAM_ATTR filamentMotionISR() {
  lastMotionPulse = millis();
  motionPulseCount++;
}

void checkFilamentSensor() {
  unsigned long now = millis();

  // Only check when actively printing
  if (printerStatus.printStatus != SDCP_PRINT_STATUS_PRINTING &&
      printerStatus.printStatus != SDCP_PRINT_STATUS_PRINTING_ALT &&
      printerStatus.printStatus != SDCP_PRINT_STATUS_PRINTING_RESUME) {
    filamentErrorDetected = false;
    lastFilamentCheck = 0;  // Reset check timer
    return;
  }

  // PRIORITY 1: Check if filament switch detects no filament (IMMEDIATE)
  bool filamentPresent = digitalRead(SENSOR_SWITCH) == HIGH;  // HIGH = present

  if (!filamentPresent && !filamentErrorDetected) {
    Serial.println("\n[SENSOR] ⚠️  FILAMENT RUNOUT DETECTED!");
    filamentErrorDetected = true;
    if (autoPauseEnabled) {
      pausePrint();
      Serial.println("[SENSOR] Print paused automatically (RUNOUT)");
    }
    return;
  }

  // PRIORITY 2: Check filament motion (with timeout)
  // Only check periodically to reduce CPU load
  if (now - lastFilamentCheck < 100) {  // Check every 100ms
    return;
  }
  lastFilamentCheck = now;

  // Determine if printhead is moving
  bool headMoving = isPrintHeadMoving();

  if (headMoving) {
    // Printhead is moving, filament should be moving too
    unsigned long timeSinceLastPulse = now - lastMotionPulse;

    if (timeSinceLastPulse > motionTimeout && !filamentErrorDetected) {
      Serial.println("\n[SENSOR] ⚠️  FILAMENT JAM DETECTED!");
      Serial.printf("[SENSOR] No motion for %lu ms while printing\n", timeSinceLastPulse);
      Serial.printf("[SENSOR] Position: %s\n", printerStatus.currentCoord.c_str());
      Serial.printf("[SENSOR] Motion pulses: %u\n", motionPulseCount.load());

      filamentErrorDetected = true;
      if (autoPauseEnabled) {
        pausePrint();
        Serial.println("[SENSOR] Print paused automatically (JAM)");
      }
    } else if (timeSinceLastPulse < motionTimeout && motionPulseCount > 0) {
      // Motion detected, all good
      if (filamentErrorDetected) {
        Serial.println("[SENSOR] ✓ Filament motion resumed");
        filamentErrorDetected = false;
      }
    }
  }
}

bool isPrintHeadMoving() {
  unsigned long now = millis();

  if (now - lastPositionCheck < POSITION_CHECK_INTERVAL) {
    return false;  // Too soon to check
  }
  lastPositionCheck = now;

  String currentPos = printerStatus.currentCoord;

  // First reading
  if (lastPosition.length() == 0) {
    lastPosition = currentPos;
    return false;
  }

  // Compare positions
  bool moving = (currentPos != lastPosition);

  if (moving) {
    Serial.printf("[SENSOR] Movement detected: %s -> %s\n",
                  lastPosition.c_str(), currentPos.c_str());
  }

  lastPosition = currentPos;
  return moving;
}

void setAutoPauseEnabled(bool enabled) {
  autoPauseEnabled = enabled;
  saveSensorSettings();  // Save to flash
  Serial.printf("[SENSOR] Auto-pause %s\n", enabled ? "enabled" : "disabled");
}

void toggleAutoPause() {
  autoPauseEnabled = !autoPauseEnabled;
  saveSensorSettings();  // Save to flash
  Serial.printf("[SENSOR] Auto-pause toggled: %s\n", autoPauseEnabled ? "enabled" : "disabled");
}

bool isFilamentErrorDetected() {
  return filamentErrorDetected;
}

void displayFilamentSensorStatus() {
  Serial.println("\n--- Filament Sensor ---");
  Serial.printf("Filament Present: %s\n",
                digitalRead(SENSOR_SWITCH) != LOW ? "YES" : "NO");
  Serial.printf("Last Motion: %lu ms ago\n", millis() - lastMotionPulse);
  Serial.printf("Motion Pulses: %u\n", motionPulseCount.load());
  Serial.printf("Error Detected: %s\n", filamentErrorDetected ? "YES" : "NO");
  Serial.printf("Auto-Pause: %s\n", autoPauseEnabled ? "Enabled" : "Disabled");
}

void resetFilamentSensor() {
  filamentErrorDetected = false;
  motionPulseCount = 0;
  lastPosition = "";
  Serial.println("[SENSOR] Sensor state reset");
}

unsigned long getLastMotionPulse() {
  return lastMotionPulse;
}

unsigned int getMotionPulseCount() {
  return motionPulseCount.load();
}

bool getAutoPauseEnabled() {
  return autoPauseEnabled;
}

void setMotionTimeout(unsigned long timeout) {
  motionTimeout = timeout;
  saveSensorSettings();  // Save to flash
  Serial.printf("[SENSOR] Motion timeout set to %lu ms\n", timeout);
}

unsigned long getMotionTimeout() {
  return motionTimeout;
}
