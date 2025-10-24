/*
 * Web Server Implementation
 */

#include "web_server.h"
#include "config.h"
#include "config_manager.h"
#include "setup_portal.h"
#include "dashboard.h"
#include "printer_status.h"
#include "printer_status_codes.h"
#include "printer_control.h"
#include "filament_sensor.h"
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

      if (doc.containsKey("wifiSSID") && doc.containsKey("wifiPassword")) {
        updateWiFiConfig(doc["wifiSSID"].as<const char*>(), doc["wifiPassword"].as<const char*>());
        needsRestart = true;
      }

      if (doc.containsKey("printerIP")) {
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
      else {
        response["success"] = false;
        response["message"] = "Unknown action";
      }

      String output;
      serializeJson(response, output);
      request->send(200, "application/json", output);
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
