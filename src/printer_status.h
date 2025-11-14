/*
 * Printer Status Management
 * Handles printer status structure and status text conversion
 */

#ifndef PRINTER_STATUS_H
#define PRINTER_STATUS_H

#include <Arduino.h>
#include "printer_status_codes.h"

// Printer status structure
struct PrinterStatus {
  int currentStatus = -1;
  float bedTemp = 0;
  float nozzleTemp = 0;
  float chamberTemp = 0;
  float bedTargetTemp = 0;
  float nozzleTargetTemp = 0;
  int printStatus = -1;
  int currentLayer = 0;
  int totalLayers = 0;
  int progress = 0;
  int currentTicks = 0;
  int totalTicks = 0;
  String filename = "";
  String currentCoord = "";
  int modelFan = 0;
  int auxFan = 0;
  int boxFan = 0;
  float zOffset = 0;
  int printSpeed = 100;
  bool lightOn = false;
};

// Global printer status
extern PrinterStatus printerStatus;

// Display current status
void displayPrinterStatus();

// Check for status changes and send notifications
void checkStatusNotifications();

// Note: getStatusText() is defined in printer_status_codes.h

#endif // PRINTER_STATUS_H
