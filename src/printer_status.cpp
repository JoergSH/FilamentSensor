/*
 * Printer Status Management Implementation
 */

#include "printer_status.h"
#include "printer_status_codes.h"
#include "filament_sensor.h"
#include "callmebot.h"
#include "config.h"

// Global printer status instance
PrinterStatus printerStatus;

// Track status changes for notifications
static int lastPrintStatus = -1;
static unsigned long printStartTime = 0;

void displayPrinterStatus() {
  Serial.println("\n========================================");
  Serial.println("         PRINTER STATUS");
  Serial.println("========================================");

  Serial.print("State: ");
  Serial.print(getStatusText(printerStatus.printStatus));
  Serial.print(" (");
  Serial.print(printerStatus.printStatus);
  Serial.println(")");

  Serial.println("\n--- Temperatures ---");
  Serial.printf("Bed:     %.1f°C / %.1f°C\n", printerStatus.bedTemp, printerStatus.bedTargetTemp);
  Serial.printf("Nozzle:  %.1f°C / %.1f°C\n", printerStatus.nozzleTemp, printerStatus.nozzleTargetTemp);
  Serial.printf("Chamber: %.1f°C\n", printerStatus.chamberTemp);

  Serial.println("\n--- Position & Movement ---");
  Serial.print("Current Position: ");
  Serial.println(printerStatus.currentCoord);
  Serial.printf("Z-Offset: %.2f mm\n", printerStatus.zOffset);

  Serial.println("\n--- Fan Speeds ---");
  Serial.printf("Model Fan:     %d%%\n", printerStatus.modelFan);
  Serial.printf("Auxiliary Fan: %d%%\n", printerStatus.auxFan);
  Serial.printf("Box Fan:       %d%%\n", printerStatus.boxFan);

  if (printerStatus.printStatus == SDCP_PRINT_STATUS_PRINTING ||
      printerStatus.printStatus == SDCP_PRINT_STATUS_PRINTING_ALT ||
      printerStatus.printStatus == SDCP_PRINT_STATUS_PRINTING_RESUME ||
      printerStatus.printStatus == SDCP_PRINT_STATUS_PAUSED ||
      printerStatus.printStatus == SDCP_PRINT_STATUS_PAUSED_ALT) {
    Serial.println("\n--- Print Progress ---");
    Serial.printf("Progress: %d%%\n", printerStatus.progress);
    Serial.printf("Layer: %d / %d\n", printerStatus.currentLayer, printerStatus.totalLayers);
    Serial.printf("Time: %d / %d ticks\n", printerStatus.currentTicks, printerStatus.totalTicks);
    Serial.printf("Print Speed: %d%%\n", printerStatus.printSpeed);
    Serial.print("File: ");
    Serial.println(printerStatus.filename.length() > 0 ? printerStatus.filename : "N/A");
  }

  Serial.print("\nLight: ");
  Serial.println(printerStatus.lightOn ? "ON" : "OFF");

  // Filament Sensor Status
  Serial.println("\n--- Filament Sensor ---");
  bool filamentPresent = digitalRead(SENSOR_SWITCH) == HIGH;  // HIGH = present
  Serial.printf("Filament Present: %s\n", filamentPresent ? "YES" : "NO");
  Serial.printf("Last Motion: %lu ms ago\n", millis() - getLastMotionPulse());
  Serial.printf("Motion Pulses: %u\n", getMotionPulseCount());
  Serial.printf("Error Detected: %s\n", isFilamentErrorDetected() ? "YES" : "NO");
  Serial.printf("Auto-Pause: %s\n", getAutoPauseEnabled() ? "Enabled" : "Disabled");

  Serial.println("========================================\n");
}

void checkStatusNotifications() {
  // Check if print status changed
  if (printerStatus.printStatus != lastPrintStatus) {
    // Print started or resumed - reset filament sensor timer
    if (lastPrintStatus != SDCP_PRINT_STATUS_PRINTING &&
        lastPrintStatus != SDCP_PRINT_STATUS_PRINTING_ALT &&
        lastPrintStatus != SDCP_PRINT_STATUS_PRINTING_RESUME &&
        (printerStatus.printStatus == SDCP_PRINT_STATUS_PRINTING ||
         printerStatus.printStatus == SDCP_PRINT_STATUS_PRINTING_ALT ||
         printerStatus.printStatus == SDCP_PRINT_STATUS_PRINTING_RESUME)) {

      resetFilamentSensor();  // Reset motion timer to prevent false jam detection
      printStartTime = millis();
      Serial.println("[STATUS] Print started/resumed - filament sensor reset");
    }

    // Print completed (when transitioning to STOPPED or IDLE after printing)
    if ((printerStatus.printStatus == SDCP_PRINT_STATUS_STOPPED ||
         printerStatus.printStatus == SDCP_PRINT_STATUS_IDLE) &&
        (lastPrintStatus == SDCP_PRINT_STATUS_PRINTING ||
         lastPrintStatus == SDCP_PRINT_STATUS_PRINTING_ALT ||
         lastPrintStatus == SDCP_PRINT_STATUS_PRINTING_RESUME)) {

      unsigned long duration = millis() - printStartTime;
      notifyPrintComplete(printerStatus.filename.c_str(), duration);
      Serial.println("[STATUS] Print completed notification sent");
    }

    lastPrintStatus = printerStatus.printStatus;
  }
}
