/*
 * OTA Update Handler
 */

#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <Arduino.h>
#include <Update.h>

// OTA Update status
enum OTAStatus {
  OTA_IDLE,
  OTA_UPDATING,
  OTA_SUCCESS,
  OTA_ERROR
};

// Initialize OTA update system
void setupOTA();

// Get current OTA status
OTAStatus getOTAStatus();

// Get OTA progress (0-100)
int getOTAProgress();

// Get OTA error message
String getOTAError();

// Start OTA update
bool startOTAUpdate(size_t firmwareSize);

// Write firmware data chunk
bool writeOTAChunk(uint8_t* data, size_t len);

// Finish OTA update
bool finishOTAUpdate();

// Abort OTA update
void abortOTAUpdate();

// Get current partition info
String getCurrentPartition();
String getNextPartition();

#endif // OTA_UPDATE_H
