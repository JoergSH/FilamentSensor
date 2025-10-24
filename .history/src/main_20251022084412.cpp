/*
 * ESP32 Elegoo Centauri Carbon Monitor
 * Mit Filament Sensor und Web Dashboard
 * 
 * Required Libraries:
 * - WebSocketsClient by Markus Sattler
 * - ArduinoJson by Benoit Blanchon
 * - ESPAsyncWebServer by me-no-dev
 * - AsyncTCP by me-no-dev
 * 
 * Install via Arduino Library Manager
 */

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>

// ========== FUNCTION PROTOTYPES ==========
void connectWiFi();
void setupWebSocket();
void setupWebServer();
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
void sendCommand(int cmd);
void sendCommand(int cmd, JsonObject *data);
void requestStatus();
void sendPing();
void parseMessage(char* payload);
void displayStatus();
const char* getStatusText(int code);
void startPrint(String filename);
void pausePrint();
void cancelPrint();
void resumePrint();
void toggleLight();
void setupFilamentSensor();
void checkFilamentSensor();
void IRAM_ATTR filamentMotionISR();
bool isPrintHeadMoving();
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                      AwsEventType type, void *arg, uint8_t *data, size_t len);
void sendWebSocketUpdate();
void sendFilamentAlert(String type, String message);

// ========== CONFIGURATION ==========
const char* ssid = "TP-Link_D7D2";
const char* password = "1234567890a";
const char* printerIP = "192.168.1.100";
const int printerPort = 80;
const char* wsPath = "/websocket";

// Filament Sensor Pin Definitions
#define SENSOR_SWITCH 1  // On/Off - Filament present detection
#define SENSOR_MOTION 0  // Motion detection - Filament movement

// Filament Sensor Configuration
#define MOTION_TIMEOUT 3000        // 3 seconds without motion = jam/runout
#define POSITION_CHECK_INTERVAL 500 // Check position change every 500ms
#define MIN_MOVEMENT_THRESHOLD 0.1  // Minimum coordinate change in mm

// ========== GLOBALS ==========
WebSocketsClient webSocket;
AsyncWebServer server(81);  // WebSocket Server auf Port 81
AsyncWebSocket ws("/ws");   // WebSocket Endpoint

unsigned long lastStatusRequest = 0;
unsigned long lastPing = 0;
unsigned long lastWebUpdate = 0;
const unsigned long STATUS_INTERVAL = 3000;
const unsigned long PING_INTERVAL = 50000;
const unsigned long WEB_UPDATE_INTERVAL = 500;  // Sende alle 500ms Update ans Dashboard

// Filament Sensor Variables
volatile unsigned long lastMotionPulse = 0;
volatile unsigned int motionPulseCount = 0;
unsigned long lastFilamentCheck = 0;
unsigned long lastPositionCheck = 0;
String lastPosition = "";
bool filamentErrorDetected = false;
bool autoPauseEnabled = true;

// Printer status
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
} status;

// ========== STATUS CODES ==========
const char* getStatusText(int code) {
  switch(code) {
    case 0: return "IDLE";
    case 8: return "PREPARING";
    case 9: return "STARTING";
    case 10: return "PAUSED";
    case 13: return "PRINTING";
    default: return "UNKNOWN";
  }
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=================================");
  Serial.println("Centauri Carbon Monitor - ESP32");
  Serial.println("With Filament Runout Detection");
  Serial.println("& Web Dashboard");
  Serial.println("=================================\n");
  
  setupFilamentSensor();
  connectWiFi();
  setupWebSocket();
  setupWebServer();
}

// ========== FILAMENT SENSOR SETUP ==========
void setupFilamentSensor() {
  pinMode(SENSOR_SWITCH, INPUT_PULLUP);
  pinMode(SENSOR_MOTION, INPUT_PULLUP);
  
  // Attach interrupt for motion detection
  attachInterrupt(digitalPinToInterrupt(SENSOR_MOTION), filamentMotionISR, FALLING);
  
  Serial.println("[SENSOR] Filament sensor initialized");
}

// Interrupt service routine for filament motion
void IRAM_ATTR filamentMotionISR() {
  lastMotionPulse = millis();
  motionPulseCount++;
}

// ========== WIFI SETUP ==========
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed!");
  }
}

// ========== WEBSOCKET TO PRINTER SETUP ==========
void setupWebSocket() {
  Serial.printf("Connecting to printer at ws://%s%s\n", printerIP, wsPath);
  webSocket.begin(printerIP, printerPort, wsPath);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

// ========== WEB SERVER SETUP ==========
void setupWebServer() {
  // WebSocket Event Handler
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  
  // Start Server
  server.begin();
  Serial.println("[WEB] WebSocket Server gestartet auf Port 81");
  Serial.printf("[WEB] Dashboard URL: http://%s:81\n", WiFi.localIP().toString().c_str());
}

// ========== MAIN LOOP ==========
void loop() {
  webSocket.loop();
  
  // Send periodic status requests to printer
  if (millis() - lastStatusRequest > STATUS_INTERVAL) {
    requestStatus();
    lastStatusRequest = millis();
  }
  
  // Send periodic ping to printer
  if (millis() - lastPing > PING_INTERVAL) {
    sendPing();
    lastPing = millis();
  }
  
  // Check filament sensor
  checkFilamentSensor();
  
  // Send updates to web dashboard
  if (millis() - lastWebUpdate > WEB_UPDATE_INTERVAL) {
    sendWebSocketUpdate();
    lastWebUpdate = millis();
  }
  
  // Cleanup WebSocket Clients
  ws.cleanupClients();
}

// ========== FILAMENT SENSOR CHECK ==========
void checkFilamentSensor() {
  unsigned long now = millis();
  
  // Only check when actively printing
  if (status.printStatus != 13) {  // 13 = PRINTING
    filamentErrorDetected = false;
    return;
  }
  
  // Check if filament switch detects no filament
  // HIGH = Filament present, LOW = No filament
  int switchValue = digitalRead(SENSOR_SWITCH);
  bool filamentPresent = (switchValue == HIGH);
  
  if (!filamentPresent && !filamentErrorDetected) {
    Serial.println("\n[SENSOR] ⚠️  FILAMENT RUNOUT DETECTED!");
    filamentErrorDetected = true;
    sendFilamentAlert("filament_runout", "Filament ist leer!");
    if (autoPauseEnabled) {
      pausePrint();
      Serial.println("[SENSOR] Print paused automatically");
    }
    return;
  }
  
  // Check filament motion only if printhead should be moving
  if (now - lastFilamentCheck < MOTION_TIMEOUT) {
    return;
  }
  lastFilamentCheck = now;
  
  // Determine if printhead is moving
  bool headMoving = isPrintHeadMoving();
  
  if (headMoving) {
    // Printhead is moving, filament should be moving too
    unsigned long timeSinceLastPulse = now - lastMotionPulse;
    
    if (timeSinceLastPulse > MOTION_TIMEOUT && !filamentErrorDetected) {
      Serial.println("\n[SENSOR] ⚠️  FILAMENT JAM DETECTED!");
      Serial.printf("[SENSOR] No motion for %lu ms while printing\n", timeSinceLastPulse);
      Serial.printf("[SENSOR] Position: %s\n", status.currentCoord.c_str());
      Serial.printf("[SENSOR] Motion pulses: %u\n", motionPulseCount);
      
      filamentErrorDetected = true;
      sendFilamentAlert("filament_jam", "Filament-Stau erkannt!");
      if (autoPauseEnabled) {
        pausePrint();
        Serial.println("[SENSOR] Print paused automatically");
      }
    } else if (timeSinceLastPulse < MOTION_TIMEOUT && motionPulseCount > 0) {
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
  
  String currentPos = status.currentCoord;
  
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

// ========== WEBSOCKET TO PRINTER EVENTS ==========
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Disconnected from printer!");
      break;
      
    case WStype_CONNECTED:
      Serial.println("[WS] Connected to printer!");
      requestStatus();
      break;
      
    case WStype_TEXT:
      parseMessage((char*)payload);
      break;
      
    case WStype_ERROR:
      Serial.println("[WS] Error!");
      break;
  }
}

void sendCommand(int cmd) {
  sendCommand(cmd, nullptr);
}

void sendCommand(int cmd, JsonObject *data) {
  JsonDocument doc;

  doc["Id"] = "";
  JsonObject dataObj = doc["Data"].to<JsonObject>();
  dataObj["Cmd"] = cmd;

  if (data != nullptr) {
    JsonObject cmdData = dataObj["Data"].to<JsonObject>();
    for (JsonPair kv : *data) {
      cmdData[kv.key()] = kv.value();
    }
  } else {
    dataObj["Data"].to<JsonObject>();
  }

  dataObj["RequestID"] = String(random(0xFFFFFF), HEX);
  dataObj["MainboardID"] = "";
  dataObj["TimeStamp"] = millis();
  dataObj["From"] = 1;

  String output;
  serializeJson(doc, output);
  webSocket.sendTXT(output);
}

void requestStatus() {
  sendCommand(0);
}

void sendPing() {
  webSocket.sendTXT("ping");
}

void parseMessage(char* payload) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);
  
  if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.c_str());
    return;
  }
  
  // Parse status silently (no JSON output)
  if (doc["Status"].is<JsonObject>()) {
    JsonObject statusObj = doc["Status"];
    
    if (!statusObj["CurrentStatus"].isNull()) {
      JsonArray arr = statusObj["CurrentStatus"].as<JsonArray>();
      if (arr.size() > 0) {
        status.currentStatus = arr[0];
      }
    }
    
    status.bedTemp = statusObj["TempOfHotbed"] | 0.0f;
    status.nozzleTemp = statusObj["TempOfNozzle"] | 0.0f;
    status.chamberTemp = statusObj["TempOfBox"] | 0.0f;
    status.bedTargetTemp = statusObj["TempTargetHotbed"] | 0.0f;
    status.nozzleTargetTemp = statusObj["TempTargetNozzle"] | 0.0f;
    status.currentCoord = statusObj["CurrenCoord"].as<String>();
    
    if (!statusObj["CurrentFanSpeed"].isNull()) {
      JsonObject fanSpeed = statusObj["CurrentFanSpeed"];
      status.modelFan = fanSpeed["ModelFan"] | 0;
      status.auxFan = fanSpeed["AuxiliaryFan"] | 0;
      status.boxFan = fanSpeed["BoxFan"] | 0;
    }
    
    status.zOffset = statusObj["ZOffset"] | 0.0f;
    
    if (!statusObj["PrintInfo"].isNull()) {
      JsonObject printInfo = statusObj["PrintInfo"];
      status.printStatus = printInfo["Status"] | -1;
      status.currentLayer = printInfo["CurrentLayer"] | 0;
      status.totalLayers = printInfo["TotalLayer"] | 0;
      status.currentTicks = printInfo["CurrentTicks"] | 0;
      status.totalTicks = printInfo["TotalTicks"] | 0;
      status.progress = printInfo["Progress"] | 0;
      status.printSpeed = printInfo["PrintSpeedPct"] | 100;
      status.filename = printInfo["Filename"].as<String>();
    }
    
    if (!statusObj["LightStatus"].isNull()) {
      JsonObject lightStatus = statusObj["LightStatus"];
      int lightValue = lightStatus["SecondLight"].as<int>();
      status.lightOn = (lightValue == 1);
    }
    
    displayStatus();
  }
  else if (!doc["Data"].isNull()) {
    JsonObject data = doc["Data"];
    if (!data["Cmd"].isNull()) {
      int cmd = data["Cmd"];
      // Only log command failures
      if (!data["Data"].isNull() && !data["Data"]["Ack"].isNull()) {
        int ack = data["Data"]["Ack"];
        if (ack != 0) {
          Serial.printf("[ERROR] Command %d failed (Ack: %d)\n", cmd, ack);
        }
      }
    }
  }
}

void displayStatus() {
  // Only show minimal status
  Serial.println("\n--- Position & Movement ---");
  Serial.print("Current Position: ");
  Serial.println(status.currentCoord);
  
  Serial.println("\n--- Filament Sensor ---");
  int switchRaw = digitalRead(SENSOR_SWITCH);
  bool present = (switchRaw == HIGH);  // HIGH = Filament present
  Serial.printf("Switch Pin %d: %s (Raw: %d)\n", 
                SENSOR_SWITCH,
                switchRaw == HIGH ? "HIGH" : "LOW",
                switchRaw);
  Serial.printf("Filament Present: %s\n", present ? "YES" : "NO");
  Serial.printf("Last Motion: %lu ms ago\n", millis() - lastMotionPulse);
  Serial.printf("Motion Pulses: %u\n", motionPulseCount);
  Serial.printf("Error Detected: %s\n", filamentErrorDetected ? "YES" : "NO");
  Serial.println();
}

// ========== WEB DASHBOARD WEBSOCKET EVENTS ==========
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("[WEB] Client #%u verbunden von %s\n", 
                    client->id(), client->remoteIP().toString().c_str());
      // Sende sofort vollständigen Status
      sendWebSocketUpdate();
      break;
      
    case WS_EVT_DISCONNECT:
      Serial.printf("[WEB] Client #%u getrennt\n", client->id());
      break;
      
    case WS_EVT_DATA: {
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        String message = String((char*)data);
        
        // Parse JSON Command
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, message);
        
        if (!error && doc["type"] == "command") {
          String cmd = doc["command"].as<String>();
          
          if (cmd == "pause") {
            pausePrint();
            Serial.println("[WEB] Command: Pause");
          }
          else if (cmd == "resume") {
            resumePrint();
            Serial.println("[WEB] Command: Resume");
          }
          else if (cmd == "cancel") {
            cancelPrint();
            Serial.println("[WEB] Command: Cancel");
          }
          else if (cmd == "light") {
            toggleLight();
            Serial.println("[WEB] Command: Toggle Light");
          }
        }
      }
      break;
    }
    
    case WS_EVT_ERROR:
      Serial.println("[WEB] WebSocket Error");
      break;
  }
}

void sendWebSocketUpdate() {
  if (ws.count() == 0) return;  // Keine Clients verbunden
  
  JsonDocument doc;
  JsonObject statusObj = doc["status"].to<JsonObject>();
  
  // Printer Status
  statusObj["printStatus"] = status.printStatus;
  statusObj["filename"] = status.filename;
  statusObj["progress"] = status.progress;
  statusObj["currentLayer"] = status.currentLayer;
  statusObj["totalLayers"] = status.totalLayers;
  statusObj["printSpeed"] = status.printSpeed;
  
  // Temperaturen
  statusObj["bedTemp"] = status.bedTemp;
  statusObj["bedTargetTemp"] = status.bedTargetTemp;
  statusObj["nozzleTemp"] = status.nozzleTemp;
  statusObj["nozzleTargetTemp"] = status.nozzleTargetTemp;
  statusObj["chamberTemp"] = status.chamberTemp;
  
  // Position
  statusObj["currentCoord"] = status.currentCoord;
  statusObj["zOffset"] = status.zOffset;
  
  // Lüfter
  statusObj["modelFan"] = status.modelFan;
  statusObj["auxFan"] = status.auxFan;
  statusObj["boxFan"] = status.boxFan;
  
  // Filament Sensor
  JsonObject sensorObj = doc["sensor"].to<JsonObject>();
  sensorObj["present"] = (digitalRead(SENSOR_SWITCH) == HIGH);
  sensorObj["lastMotion"] = millis() - lastMotionPulse;
  sensorObj["pulses"] = motionPulseCount;
  sensorObj["switchRaw"] = digitalRead(SENSOR_SWITCH);
  sensorObj["error"] = filamentErrorDetected;
  
  String output;
  serializeJson(doc, output);
  ws.textAll(output);
}

void sendFilamentAlert(String type, String message) {
  if (ws.count() == 0) return;
  
  JsonDocument doc;
  doc["type"] = "alert";
  doc["alertType"] = type;
  doc["message"] = message;
  doc["timestamp"] = millis();
  
  String output;
  serializeJson(doc, output);
  ws.textAll(output);
}

// ========== CONTROL FUNCTIONS ==========
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
  filamentErrorDetected = false;
  motionPulseCount = 0;
  lastPosition = "";
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
  filamentErrorDetected = false;
  motionPulseCount = 0;
}

void toggleLight() {
  JsonDocument doc;
  JsonObject data = doc.to<JsonObject>();
  JsonObject lightStatus = data["LightStatus"].to<JsonObject>();
  lightStatus["SecondLight"] = !status.lightOn;
  JsonArray rgb = lightStatus["RgbLight"].to<JsonArray>();
  rgb.add(0);
  rgb.add(0);
  rgb.add(0);
  
  sendCommand(403, &data);
  Serial.println("Sent: Toggle Light");
}