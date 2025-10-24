/*
 * OTA Update Implementation
 */

#include "ota_update.h"
#include <esp_ota_ops.h>

// OTA state
static OTAStatus otaStatus = OTA_IDLE;
static int otaProgress = 0;
static String otaError = "";
static size_t totalSize = 0;
static size_t writtenSize = 0;

void setupOTA() {
  // Nothing needed for initialization
  Serial.println("[OTA] OTA update system initialized");

  // Print current partition info
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (running) {
    Serial.printf("[OTA] Running from partition: %s (type %d, subtype %d, address 0x%x, size %d)\n",
                  running->label, running->type, running->subtype, running->address, running->size);
  }

  const esp_partition_t* next = esp_ota_get_next_update_partition(NULL);
  if (next) {
    Serial.printf("[OTA] Next update partition: %s (type %d, subtype %d, address 0x%x, size %d)\n",
                  next->label, next->type, next->subtype, next->address, next->size);
  }
}

OTAStatus getOTAStatus() {
  return otaStatus;
}

int getOTAProgress() {
  return otaProgress;
}

String getOTAError() {
  return otaError;
}

bool startOTAUpdate(size_t firmwareSize) {
  if (otaStatus == OTA_UPDATING) {
    otaError = "Update already in progress";
    return false;
  }

  Serial.printf("[OTA] Starting update, firmware size: %d bytes\n", firmwareSize);

  // Check partition size
  const esp_partition_t* partition = esp_ota_get_next_update_partition(NULL);
  if (!partition) {
    otaError = "No OTA partition available";
    Serial.println("[OTA] ERROR: No OTA partition found");
    return false;
  }

  if (firmwareSize > partition->size) {
    otaError = "Firmware too large for partition";
    Serial.printf("[OTA] ERROR: Firmware size %d exceeds partition size %d\n", firmwareSize, partition->size);
    return false;
  }

  // Start update
  if (!Update.begin(firmwareSize, U_FLASH)) {
    otaError = "Update.begin failed: " + String(Update.errorString());
    Serial.printf("[OTA] ERROR: Update.begin failed: %s\n", Update.errorString());
    return false;
  }

  totalSize = firmwareSize;
  writtenSize = 0;
  otaProgress = 0;
  otaStatus = OTA_UPDATING;
  otaError = "";

  Serial.println("[OTA] Update started successfully");
  return true;
}

bool writeOTAChunk(uint8_t* data, size_t len) {
  if (otaStatus != OTA_UPDATING) {
    otaError = "Update not started";
    return false;
  }

  size_t written = Update.write(data, len);
  if (written != len) {
    otaError = "Write failed: " + String(Update.errorString());
    Serial.printf("[OTA] ERROR: Write failed, expected %d, wrote %d: %s\n",
                  len, written, Update.errorString());
    otaStatus = OTA_ERROR;
    Update.abort();
    return false;
  }

  writtenSize += written;
  otaProgress = (writtenSize * 100) / totalSize;

  // Print progress every 10%
  static int lastProgress = 0;
  if (otaProgress >= lastProgress + 10) {
    Serial.printf("[OTA] Progress: %d%% (%d / %d bytes)\n", otaProgress, writtenSize, totalSize);
    lastProgress = otaProgress;
  }

  return true;
}

bool finishOTAUpdate() {
  if (otaStatus != OTA_UPDATING) {
    otaError = "Update not in progress";
    return false;
  }

  if (!Update.end(true)) {
    otaError = "Update.end failed: " + String(Update.errorString());
    Serial.printf("[OTA] ERROR: Update.end failed: %s\n", Update.errorString());
    otaStatus = OTA_ERROR;
    return false;
  }

  otaProgress = 100;
  otaStatus = OTA_SUCCESS;
  Serial.println("[OTA] Update completed successfully!");
  Serial.println("[OTA] Rebooting in 2 seconds...");

  return true;
}

void abortOTAUpdate() {
  if (otaStatus == OTA_UPDATING) {
    Update.abort();
    Serial.println("[OTA] Update aborted");
  }
  otaStatus = OTA_ERROR;
  otaError = "Update aborted by user";
  otaProgress = 0;
}

String getCurrentPartition() {
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (running) {
    return String(running->label);
  }
  return "unknown";
}

String getNextPartition() {
  const esp_partition_t* next = esp_ota_get_next_update_partition(NULL);
  if (next) {
    return String(next->label);
  }
  return "unknown";
}
