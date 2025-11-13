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
static bool switchDirectMode = true;  // true = direct to RUNOUT_PIN, false = send pause command
static unsigned long motionTimeout = MOTION_TIMEOUT;  // Default from config.h, but changeable
static bool motionDetectedThisPrint = false;  // Track if we've seen motion during current print

// Load settings from persistent storage
void loadSensorSettings() {
  preferences.begin("filament", false);  // false = read-write mode

  // Load motion timeout (default: MOTION_TIMEOUT from config.h)
  motionTimeout = preferences.getULong("motionTimeout", MOTION_TIMEOUT);

  // Load auto-pause enabled (default: true)
  autoPauseEnabled = preferences.getBool("autoPause", true);

  // Load switch mode (default: true = direct mode)
  switchDirectMode = preferences.getBool("switchDirect", true);

  preferences.end();

  Serial.println("[SENSOR] Settings loaded from flash:");
  Serial.printf("[SENSOR]   Motion Timeout: %lu ms\n", motionTimeout);
  Serial.printf("[SENSOR]   Auto-Pause: %s\n", autoPauseEnabled ? "enabled" : "disabled");
  Serial.printf("[SENSOR]   Switch Mode: %s\n", switchDirectMode ? "Direct" : "Pause Command");
}

// Save settings to persistent storage
void saveSensorSettings() {
  preferences.begin("filament", false);  // false = read-write mode

  preferences.putULong("motionTimeout", motionTimeout);
  preferences.putBool("autoPause", autoPauseEnabled);
  preferences.putBool("switchDirect", switchDirectMode);

  preferences.end();

  Serial.println("[SENSOR] Settings saved to flash");
}

void setupFilamentSensor() {
  // Load saved settings from flash
  loadSensorSettings();

  pinMode(SENSOR_SWITCH, INPUT_PULLDOWN);
  pinMode(SENSOR_MOTION, INPUT_PULLUP);

  // Initialize runout output pin (INPUT = floating/HIGH via printer pull-up)
  pinMode(RUNOUT_PIN, INPUT);  // Default to HIGH (no error) via printer's pull-up

  // Attach interrupt for motion detection
  attachInterrupt(digitalPinToInterrupt(SENSOR_MOTION), filamentMotionISR, FALLING);

  Serial.println("[SENSOR] Filament sensor initialized");
  Serial.printf("[SENSOR] Switch Pin: %d, Motion Pin: %d, Runout Output: %d\n",
                SENSOR_SWITCH, SENSOR_MOTION, RUNOUT_PIN);
}

void IRAM_ATTR filamentMotionISR() {
  lastMotionPulse = millis();
  motionPulseCount++;
}

void checkFilamentSensor() {
  unsigned long now = millis();

  // Read filament switch state
  bool filamentPresent = digitalRead(SENSOR_SWITCH) == HIGH;  // HIGH = present

  // Handle switch output pin based on mode (ALWAYS, even when not printing)
  if (switchDirectMode) {
    // DIRECT MODE: Forward SENSOR_SWITCH state INVERTED to RUNOUT_PIN
    // Printer expects: HIGH = OK, LOW = error
    // We have: filamentPresent true = OK, false = error
    // So send: filamentPresent as-is (true = HIGH = OK, false = LOW = error)
    setRunoutPinOutput(filamentPresent);  // HIGH if present (OK), LOW if absent (error)
  } else {
    // PAUSE COMMAND MODE: Keep RUNOUT_PIN always HIGH (no error to printer)
    setRunoutPinOutput(true);
  }

  // Only check for auto-pause when actively printing
  if (printerStatus.printStatus != SDCP_PRINT_STATUS_PRINTING &&
      printerStatus.printStatus != SDCP_PRINT_STATUS_PRINTING_ALT &&
      printerStatus.printStatus != SDCP_PRINT_STATUS_PRINTING_RESUME) {
    filamentErrorDetected = false;
    lastFilamentCheck = 0;  // Reset check timer
    motionDetectedThisPrint = false;  // Reset motion tracking for next print
    return;
  }

  // ===== FROM HERE ON: ONLY WHEN ACTIVELY PRINTING =====

  // PRIORITY 1: Check if filament switch detects no filament (IMMEDIATE)
  if (!filamentPresent && !filamentErrorDetected) {
    Serial.println("\n[SENSOR] ⚠️  FILAMENT RUNOUT DETECTED!");
    filamentErrorDetected = true;

    // In Pause Mode: send pause command (Direct Mode handles via pin)
    if (!switchDirectMode && autoPauseEnabled) {
      pausePrint();
      Serial.println("[SENSOR] Print paused automatically (Pause Mode - RUNOUT)");
    }
    return;
  } else if (filamentPresent && filamentErrorDetected) {
    // Filament restored
    Serial.println("[SENSOR] ✓ Filament restored");
    filamentErrorDetected = false;
  }

  // PRIORITY 2: Check filament motion (with timeout)
  // Only check periodically to reduce CPU load
  if (now - lastFilamentCheck < 100) {  // Check every 100ms
    return;
  }
  lastFilamentCheck = now;

  // Check if motion pulses received (regardless of printhead movement)
  unsigned long timeSinceLastPulse = now - lastMotionPulse;

  if (motionPulseCount > 0) {
    // Motion pulses received - reset counter
    motionPulseCount = 0;
    motionDetectedThisPrint = true;  // Mark that we've seen motion during this print

    if (filamentErrorDetected) {
      Serial.println("[SENSOR] ✓ Filament motion resumed");
      filamentErrorDetected = false;
    }
  }

  // Check if on last layer (parking, no more filament movement expected)
  bool onLastLayer = (printerStatus.currentLayer >= printerStatus.totalLayers && printerStatus.totalLayers > 0);

  // Determine if printhead is moving
  bool headMoving = isPrintHeadMoving();

  if (headMoving && !onLastLayer) {
    // Printhead is moving and not on last layer, filament should be moving too
    // Only check for jam if we've already seen motion during this print (prevents false positives at start)
    if (motionDetectedThisPrint && timeSinceLastPulse > motionTimeout && !filamentErrorDetected) {
      Serial.println("\n[SENSOR] ⚠️  FILAMENT JAM DETECTED!");
      Serial.printf("[SENSOR] No motion for %lu ms while printing\n", timeSinceLastPulse);
      Serial.printf("[SENSOR] Position: %s\n", printerStatus.currentCoord.c_str());
      Serial.printf("[SENSOR] Layer: %d/%d\n", printerStatus.currentLayer, printerStatus.totalLayers);
      Serial.printf("[SENSOR] Motion pulses: %u\n", motionPulseCount.load());

      filamentErrorDetected = true;
      if (autoPauseEnabled) {
        pausePrint();
        Serial.println("[SENSOR] Print paused automatically (JAM)");
      }
    }
  } else if (onLastLayer && filamentErrorDetected) {
    // On last layer, clear any previous errors
    Serial.println("[SENSOR] Last layer - clearing filament errors");
    filamentErrorDetected = false;
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
                digitalRead(SENSOR_SWITCH) == HIGH ? "YES" : "NO");
  Serial.printf("Last Motion: %lu ms ago\n", millis() - lastMotionPulse);
  Serial.printf("Motion Pulses: %u\n", motionPulseCount.load());
  Serial.printf("Error Detected: %s\n", filamentErrorDetected ? "YES" : "NO");
  Serial.printf("Auto-Pause: %s\n", autoPauseEnabled ? "Enabled" : "Disabled");
}

void resetFilamentSensor() {
  filamentErrorDetected = false;
  motionPulseCount = 0;
  lastPosition = "";
  motionDetectedThisPrint = false;  // Reset motion tracking
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

void setRunoutPinOutput(bool state) {
  static bool lastState = true;  // Track last state to avoid unnecessary pinMode changes

  if (state != lastState) {
    if (state) {
      // HIGH = Release pin (floating/high-impedance, pull-up on printer pulls to HIGH)
      pinMode(RUNOUT_PIN, INPUT);
      Serial.println("[RUNOUT OUTPUT] Pin IO2 released (floating -> HIGH via pull-up)");
    } else {
      // LOW = Pull to ground (open-drain style)
      pinMode(RUNOUT_PIN, OUTPUT);
      digitalWrite(RUNOUT_PIN, LOW);
      Serial.println("[RUNOUT OUTPUT] Pin IO2 pulled to GND (LOW)");
    }
    lastState = state;
  }
}

String getRunoutPinState() {
  // Read current output state
  int currentState = digitalRead(RUNOUT_PIN);

  String result = "Pin IO2: ";
  result += (currentState == HIGH) ? "HIGH (1)" : "LOW (0)";

  Serial.printf("[RUNOUT STATE] %s\n", result.c_str());

  return result;
}

bool getSwitchDirectMode() {
  return switchDirectMode;
}

void setSwitchDirectMode(bool directMode) {
  switchDirectMode = directMode;
  saveSensorSettings();  // Save to flash
  Serial.printf("[SENSOR] Switch mode set to: %s\n", directMode ? "Direct" : "Pause Command");
}

void toggleSwitchMode() {
  setSwitchDirectMode(!switchDirectMode);
  Serial.printf("[SENSOR] Switch mode toggled to: %s\n", switchDirectMode ? "Direct" : "Pause Command");
}
