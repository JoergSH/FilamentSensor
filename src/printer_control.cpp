/*
 * Printer Control Functions Implementation
 */

#include "printer_control.h"
#include "websocket_client.h"
#include "printer_status.h"
#include "filament_sensor.h"
#include <ArduinoJson.h>

void startPrint(String filename) {
  JsonDocument doc;
  JsonObject data = doc.to<JsonObject>();
  data["Filename"] = "/local/" + filename;
  data["StartLayer"] = 0;
  data["Calibration_switch"] = 0;
  data["PrintPlatformType"] = 0;
  data["Tlp_Switch"] = 0;

  sendCommand(128, &data);
  Serial.println("Sent: Start Print");

  // Reset sensor state
  resetFilamentSensor();
}

void pausePrint() {
  sendCommand(129);
  Serial.println("Sent: Pause Print");
}

void cancelPrint() {
  sendCommand(130);
  Serial.println("Sent: Cancel Print");
}

void resumePrint() {
  sendCommand(131);
  Serial.println("Sent: Resume Print");

  // Reset error state when resuming
  resetFilamentSensor();
}

void toggleLight() {
  JsonDocument doc;
  JsonObject data = doc.to<JsonObject>();
  JsonObject lightStatus = data["LightStatus"].to<JsonObject>();
  lightStatus["SecondLight"] = !printerStatus.lightOn;
  JsonArray rgb = lightStatus["RgbLight"].to<JsonArray>();
  rgb.add(0);
  rgb.add(0);
  rgb.add(0);

  sendCommand(403, &data);
  Serial.println("Sent: Toggle Light");
}
