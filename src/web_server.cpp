/*
 * Web Server Implementation
 */

#include "web_server.h"
#include "config.h"
#include "config_manager.h"
#include "setup_portal.h"
#include "dashboard.h"
#include "settings.h"
#include "printer_status.h"
#include "printer_status_codes.h"
#include "printer_control.h"
#include "filament_sensor.h"
#include "ota_update.h"
#include <ArduinoJson.h>

// Web server instance
static AsyncWebServer webServer(80);

// Use getter functions instead of external variables

void setupWebServer() {
  // Serve setup portal or dashboard based on configuration
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isConfigured()) {
      request->send(200, "text/html", getDashboardHTML());
    } else {
      request->send(200, "text/html", getSetupPortalHTML());
    }
  });

  // Setup portal page (accessible anytime)
  webServer.on("/setup", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", getSetupPortalHTML());
  });

  // Settings page
  webServer.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", getSettingsHTML());
  });

  // API: Handle initial setup
  webServer.on("/api/setup", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);

      if (error) {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
        return;
      }

      String ssid = doc["ssid"].as<String>();
      String password = doc["password"].as<String>();
      String printerIp = doc["printerIp"].as<String>();
      int printerPort = doc["printerPort"] | 80;

      // Save configuration
      updateWiFiConfig(ssid.c_str(), password.c_str());
      updatePrinterConfig(printerIp.c_str(), printerPort);

      JsonDocument response;
      response["success"] = true;
      response["message"] = "Configuration saved. Restarting...";

      String output;
      serializeJson(response, output);
      request->send(200, "application/json", output);

      // Restart ESP32 after 2 seconds
      delay(2000);
      ESP.restart();
    }
  );

  // API: Get current configuration
  webServer.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    SystemConfig& config = getConfig();
    JsonDocument doc;

    doc["configured"] = config.configured;
    doc["wifiSSID"] = config.wifiSSID;
    doc["printerIP"] = config.printerIP;
    doc["printerPort"] = config.printerPort;

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
  });

  // API: Update configuration
  webServer.on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);

      if (error) {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
        return;
      }

      bool needsRestart = false;

      if (doc["wifiSSID"].is<const char*>() && doc["wifiPassword"].is<const char*>()) {
        updateWiFiConfig(doc["wifiSSID"].as<const char*>(), doc["wifiPassword"].as<const char*>());
        needsRestart = true;
      }

      if (doc["printerIP"].is<const char*>()) {
        int port = doc["printerPort"] | 80;
        updatePrinterConfig(doc["printerIP"].as<const char*>(), port);
        needsRestart = true;
      }

      JsonDocument response;
      response["success"] = true;
      response["message"] = needsRestart ? "Config saved. Please restart." : "Config saved.";
      response["needsRestart"] = needsRestart;

      String output;
      serializeJson(response, output);
      request->send(200, "application/json", output);
    }
  );

  // API: Get status
  webServer.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;

    // Status information
    JsonObject status = doc["status"].to<JsonObject>();
    status["state"] = printerStatus.printStatus;
    status["stateText"] = getStatusText(printerStatus.printStatus);
    status["position"] = printerStatus.currentCoord;
    status["zOffset"] = printerStatus.zOffset;
    status["lightOn"] = printerStatus.lightOn;
    status["bedTemp"] = printerStatus.bedTemp;
    status["bedTarget"] = printerStatus.bedTargetTemp;
    status["nozzleTemp"] = printerStatus.nozzleTemp;
    status["nozzleTarget"] = printerStatus.nozzleTargetTemp;
    status["chamberTemp"] = printerStatus.chamberTemp;

    // Print information
    JsonObject print = doc["print"].to<JsonObject>();
    print["progress"] = printerStatus.progress;
    print["filename"] = printerStatus.filename;
    print["layer"] = printerStatus.currentLayer;
    print["totalLayers"] = printerStatus.totalLayers;
    print["speed"] = printerStatus.printSpeed;

    // Fan information
    JsonObject fans = doc["fans"].to<JsonObject>();
    fans["model"] = printerStatus.modelFan;
    fans["aux"] = printerStatus.auxFan;
    fans["box"] = printerStatus.boxFan;

    // Filament sensor information
    JsonObject sensor = doc["sensor"].to<JsonObject>();
    sensor["error"] = isFilamentErrorDetected();
    sensor["lastMotion"] = millis() - getLastMotionPulse();
    sensor["pulseCount"] = getMotionPulseCount();
    sensor["autoPause"] = getAutoPauseEnabled();
    sensor["pauseDelay"] = getMotionTimeout();

    // Check filament present (HIGH = present on this sensor)
    bool filamentPresent = digitalRead(SENSOR_SWITCH) == HIGH;
    sensor["noFilament"] = !filamentPresent;

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
  });

  // API: Control commands
  webServer.on("/api/control", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data, len);

      if (error) {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
        return;
      }

      String action = doc["action"].as<String>();
      JsonDocument response;
      response["success"] = true;

      if (action == "pause") {
        pausePrint();
        response["message"] = "Print paused";
      }
      else if (action == "resume") {
        resumePrint();
        response["message"] = "Print resumed";
      }
      else if (action == "cancel") {
        cancelPrint();
        response["message"] = "Print cancelled";
      }
      else if (action == "toggleLight") {
        toggleLight();
        response["message"] = "Light toggled";
      }
      else if (action == "toggleAutoPause") {
        toggleAutoPause();
        response["message"] = getAutoPauseEnabled() ? "Auto-pause enabled" : "Auto-pause disabled";
      }
      else if (action == "clearError") {
        resetFilamentSensor();
        response["message"] = "Sensor error cleared";
      }
      else if (action == "setPauseDelay") {
        unsigned long delay = doc["delay"] | getMotionTimeout();
        setMotionTimeout(delay);
        response["message"] = "Pause delay updated to " + String(delay) + " ms";
      }
      else if (action == "restart") {
        response["message"] = "Restarting ESP32...";
        String output;
        serializeJson(response, output);
        request->send(200, "application/json", output);
        delay(1000);
        ESP.restart();
        return;
      }
      else {
        response["success"] = false;
        response["message"] = "Unknown action";
      }

      String output;
      serializeJson(response, output);
      request->send(200, "application/json", output);
    }
  );

  // API: Set runout pin output state
  webServer.on("/api/test/runout/set", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;

    if (request->hasParam("state")) {
      String stateStr = request->getParam("state")->value();
      bool state = (stateStr == "1" || stateStr == "true" || stateStr == "HIGH");

      setRunoutPinOutput(state);

      doc["success"] = true;
      doc["message"] = state ? "Pin set to HIGH" : "Pin set to LOW";
    } else {
      doc["success"] = false;
      doc["message"] = "Missing 'state' parameter";
    }

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
  });

  // API: Read runout pin state
  webServer.on("/api/test/runout/read", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;

    String stateResult = getRunoutPinState();
    doc["success"] = true;
    doc["result"] = stateResult;

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
  });

  // API: Get OTA status
  webServer.on("/api/ota/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;

    doc["status"] = getOTAStatus();
    doc["progress"] = getOTAProgress();
    doc["error"] = getOTAError();
    doc["currentPartition"] = getCurrentPartition();
    doc["nextPartition"] = getNextPartition();

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
  });

  // API: Upload firmware for OTA update
  webServer.on("/api/ota/upload", HTTP_POST,
    [](AsyncWebServerRequest *request) {
      // This is called after upload completes
      JsonDocument doc;

      if (getOTAStatus() == OTA_SUCCESS) {
        doc["success"] = true;
        doc["message"] = "Firmware uploaded successfully. Rebooting...";

        String output;
        serializeJson(doc, output);
        request->send(200, "application/json", output);

        // Reboot after sending response
        delay(1000);
        ESP.restart();
      } else {
        doc["success"] = false;
        doc["message"] = getOTAError();

        String output;
        serializeJson(doc, output);
        request->send(500, "application/json", output);
      }
    },
    [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      // This is called for each chunk of uploaded data

      if (index == 0) {
        // First chunk - start OTA update
        Serial.printf("[OTA] Upload started: %s, size: %d bytes\n", filename.c_str(), request->contentLength());

        if (!startOTAUpdate(request->contentLength())) {
          Serial.println("[OTA] Failed to start update");
          return;
        }
      }

      // Write chunk
      if (getOTAStatus() == OTA_UPDATING) {
        if (!writeOTAChunk(data, len)) {
          Serial.println("[OTA] Failed to write chunk");
          return;
        }
      }

      if (final) {
        // Last chunk - finish update
        Serial.printf("[OTA] Upload finished, total size: %d bytes\n", index + len);

        if (!finishOTAUpdate()) {
          Serial.println("[OTA] Failed to finish update");
        }
      }
    }
  );

  // Start server
  webServer.begin();
  Serial.println("[WEB] Web server started on port 80");
}

void processWebServer() {
  // AsyncWebServer handles everything in background
  // Nothing needed here
}

AsyncWebServer& getWebServer() {
  return webServer;
}
