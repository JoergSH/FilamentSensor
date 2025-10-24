/*
 * ESP32 Elegoo Centauri Carbon Monitor with Filament Sensor
 * Detects filament runout/jam and auto-pauses print
 */

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <atomic>
//#include "printer_status_codes.h"


// ========== FUNCTION PROTOTYPES ==========
void connectWiFi();
void setupWebSocket();
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
unsigned long lastStatusRequest = 0;
unsigned long lastPing = 0;
const unsigned long STATUS_INTERVAL = 3000;
const unsigned long PING_INTERVAL = 50000;
// Filament Sensor Variables
volatile unsigned long lastMotionPulse = 0;
std::atomic<unsigned int> motionPulseCount(0);
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

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=================================");
  Serial.println("Centauri Carbon Monitor - ESP32");
  Serial.println("With Filament Runout Detection");
  Serial.println("=================================\n");
  
  setupFilamentSensor();
  connectWiFi();
  setupWebSocket();
}



void loop() {
  webSocket.loop();
  
  // Send periodic status requests
  if (millis() - lastStatusRequest > STATUS_INTERVAL) {
    requestStatus();
    lastStatusRequest = millis();
  }
  
  // Send periodic ping
  if (millis() - lastPing > PING_INTERVAL) {
    sendPing();
    lastPing = millis();
  }
  
  // Check filament sensor
  checkFilamentSensor();
}

void setupFilamentSensor() {
  pinMode(SENSOR_SWITCH, INPUT_PULLDOWN);
  pinMode(SENSOR_MOTION, INPUT_PULLUP);
  
  // Attach interrupt for motion detection
  attachInterrupt(digitalPinToInterrupt(SENSOR_MOTION), filamentMotionISR, FALLING);
  
  Serial.println("[SENSOR] Filament sensor initialized");
  Serial.printf("[SENSOR] Switch Pin: %d, Motion Pin: %d\n", SENSOR_SWITCH, SENSOR_MOTION);
}

// Interrupt service routine for filament motion
void filamentMotionISR() {
  lastMotionPulse = millis();
  motionPulseCount++;
}

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

void setupWebSocket() {
  Serial.printf("Connecting to printer at ws://%s%s\n", printerIP, wsPath);
  webSocket.begin(printerIP, printerPort, wsPath);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void checkFilamentSensor() {
  unsigned long now = millis();
  
  // Only check when actively printing
  if (status.printStatus != 13) {  // 13 = PRINTING
    filamentErrorDetected = false;
    return;
  }
  
  // Check if filament switch detects no filament
  bool filamentPresent = digitalRead(SENSOR_SWITCH) == LOW;  // Assume LOW = present
  
  if (!filamentPresent && !filamentErrorDetected) {
    Serial.println("\n[SENSOR] ⚠️  FILAMENT RUNOUT DETECTED!");
    filamentErrorDetected = true;
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
      Serial.printf("[SENSOR] Motion pulses: %u\n", motionPulseCount.load());
      
      filamentErrorDetected = true;
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

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Disconnected!");
      break;
      
    case WStype_CONNECTED:
      Serial.println("[WS] Connected to printer!");
      Serial.printf("[WS] URL: ws://%s%s\n", printerIP, wsPath);
      requestStatus();
      break;
      
    case WStype_TEXT:
      parseMessage((char*)payload);
      break;
      
    case WStype_ERROR:
      Serial.println("[WS] Error!");
      break;
      
    case WStype_PING:
      Serial.println("[WS] Ping received");
      break;
      
    case WStype_PONG:
      Serial.println("[WS] Pong received");
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
  
  Serial.println("\n========== RAW JSON DATA ==========");
  serializeJsonPretty(doc, Serial);
  Serial.println("\n===================================\n");
  
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
      Serial.printf("[ACK] Command %d acknowledged\n", cmd);
      
      if (!data["Data"].isNull() && !data["Data"]["Ack"].isNull()) {
        int ack = data["Data"]["Ack"];
        switch(ack) {
          case 0: Serial.println("  Result: Success"); break;
          case 1: Serial.println("  Result: Failure/Error"); break;
          case 2: Serial.println("  Result: File Not Found"); break;
          default: Serial.printf("  Result: Unknown (%d)\n", ack); break;
        }
      }
    }
  }
}

void displayStatus() {
  Serial.println("\n========================================");
  Serial.println("         PRINTER STATUS");
  Serial.println("========================================");
  
  Serial.print("State: ");
  Serial.print(getStatusText(status.printStatus));
  Serial.print(" (");
  Serial.print(status.printStatus);
  Serial.println(")");
  
  Serial.println("\n--- Temperatures ---");
  Serial.printf("Bed:     %.1f°C / %.1f°C\n", status.bedTemp, status.bedTargetTemp);
  Serial.printf("Nozzle:  %.1f°C / %.1f°C\n", status.nozzleTemp, status.nozzleTargetTemp);
  Serial.printf("Chamber: %.1f°C\n", status.chamberTemp);
  
  Serial.println("\n--- Position & Movement ---");
  Serial.print("Current Position: ");
  Serial.println(status.currentCoord);
  Serial.printf("Z-Offset: %.2f mm\n", status.zOffset);
  
  Serial.println("\n--- Fan Speeds ---");
  Serial.printf("Model Fan:     %d%%\n", status.modelFan);
  Serial.printf("Auxiliary Fan: %d%%\n", status.auxFan);
  Serial.printf("Box Fan:       %d%%\n", status.boxFan);
  
  if (status.printStatus == 13 || status.printStatus == 10) {
    Serial.println("\n--- Print Progress ---");
    Serial.printf("Progress: %d%%\n", status.progress);
    Serial.printf("Layer: %d / %d\n", status.currentLayer, status.totalLayers);
    Serial.printf("Time: %d / %d ticks\n", status.currentTicks, status.totalTicks);
    Serial.printf("Print Speed: %d%%\n", status.printSpeed);
    Serial.print("File: ");
    Serial.println(status.filename.length() > 0 ? status.filename : "N/A");
  }
  
  Serial.println("\n--- Filament Sensor ---");
  Serial.printf("Filament Present: %s\n", 
                digitalRead(SENSOR_SWITCH) != LOW ? "YES" : "NO");
  Serial.printf("Last Motion: %lu ms ago\n", millis() - lastMotionPulse);
  Serial.printf("Motion Pulses: %u\n", motionPulseCount.load());
  Serial.printf("Error Detected: %s\n", filamentErrorDetected ? "YES" : "NO");
  
  Serial.print("\nLight: ");
  Serial.println(status.lightOn ? "ON" : "OFF");
  
  Serial.println("========================================\n");
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