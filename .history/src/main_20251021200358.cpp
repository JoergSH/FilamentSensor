/*
 * ESP32 Elegoo Centauri Carbon Monitor
 * Uses SDCP (Smart Device Control Protocol) over WebSocket
 * 
 * Required Libraries:
 * - WebSocketsClient by Markus Sattler
 * - ArduinoJson by Benoit Blanchon
 * 
 * Install via Arduino Library Manager
 */

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

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

// ========== CONFIGURATION ==========
const char* ssid = "TP-Link_D7D2";
const char* password = "1234567890a";
const char* printerIP = "192.168.1.100";
const int printerPort = 80;
const char* wsPath = "/websocket";

// Filament Sensor Pin Definitions
#define Switch 1  // On / Off
#define Motion 0  // Get Frequenzy

// ========== GLOBALS ==========
WebSocketsClient webSocket;
unsigned long lastStatusRequest = 0;
unsigned long lastPing = 0;
const unsigned long STATUS_INTERVAL = 3000;  // Request status every 3 seconds
const unsigned long PING_INTERVAL = 50000;   // Ping every 50 seconds (timeout is 60s)

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

// Status code meanings
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
  Serial.println("=================================\n");
  
  connectWiFi();
  setupWebSocket();
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

void loop() {
  webSocket.loop();
  
  // Send periodic status requests
  if (millis() - lastStatusRequest > STATUS_INTERVAL) {
    requestStatus();
    lastStatusRequest = millis();
  }
  
  // Send periodic ping to keep connection alive
  if (millis() - lastPing > PING_INTERVAL) {
    sendPing();
    lastPing = millis();
  }
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

  // Add data payload if provided
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
  sendCommand(0);  // Cmd 0 = Request Status Update
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
  
  // ===== OUTPUT COMPLETE JSON =====
  Serial.println("\n========== RAW JSON DATA ==========");
  serializeJsonPretty(doc, Serial);
  Serial.println("\n===================================\n");
  
  // Check if this is a status message
  if (doc["Status"].is<JsonObject>()) {
    JsonObject statusObj = doc["Status"];
    
    // Parse current status
    if (!statusObj["CurrentStatus"].isNull()) {
      JsonArray arr = statusObj["CurrentStatus"].as<JsonArray>();
      if (arr.size() > 0) {
        status.currentStatus = arr[0];
      }
    }
    
    // Parse temperatures
    status.bedTemp = statusObj["TempOfHotbed"] | 0.0f;
    status.nozzleTemp = statusObj["TempOfNozzle"] | 0.0f;
    status.chamberTemp = statusObj["TempOfBox"] | 0.0f;
    status.bedTargetTemp = statusObj["TempTargetHotbed"] | 0.0f;
    status.nozzleTargetTemp = statusObj["TempTargetNozzle"] | 0.0f;
    
    // Parse coordinates
    status.currentCoord = statusObj["CurrenCoord"].as<String>();
    
    // Parse fan speeds
    if (!statusObj["CurrentFanSpeed"].isNull()) {
      JsonObject fanSpeed = statusObj["CurrentFanSpeed"];
      status.modelFan = fanSpeed["ModelFan"] | 0;
      status.auxFan = fanSpeed["AuxiliaryFan"] | 0;
      status.boxFan = fanSpeed["BoxFan"] | 0;
    }
    
    // Parse Z-Offset
    status.zOffset = statusObj["ZOffset"] | 0.0f;
    
    // Parse print info
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
    
    // Parse light status
    if (!statusObj["LightStatus"].isNull()) {
      JsonObject lightStatus = statusObj["LightStatus"];
      // SecondLight: 1 = ON, 0 = OFF
      int lightValue = lightStatus["SecondLight"].as<int>();
      status.lightOn = (lightValue == 1);
    }
    
    displayStatus();
  }
  
  // Check for response/acknowledgment
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
  
  if (status.printStatus == 13 || status.printStatus == 10) {  // Printing or Paused
    Serial.println("\n--- Print Progress ---");
    Serial.printf("Progress: %d%%\n", status.progress);
    Serial.printf("Layer: %d / %d\n", status.currentLayer, status.totalLayers);
    Serial.printf("Time: %d / %d ticks\n", status.currentTicks, status.totalTicks);
    Serial.printf("Print Speed: %d%%\n", status.printSpeed);
    Serial.print("File: ");
    Serial.println(status.filename.length() > 0 ? status.filename : "N/A");
  }
  
  Serial.print("\nLight: ");
  Serial.println(status.lightOn ? "ON" : "OFF");
  
  Serial.println("========================================\n");
}

// ========== CONTROL FUNCTIONS ==========
// Call these to control the printer

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