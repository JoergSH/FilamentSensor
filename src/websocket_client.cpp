/*
 * WebSocket Client Implementation
 */

#include "websocket_client.h"
#include "config.h"
#include "config_manager.h"
#include "printer_status.h"

// WebSocket instance
static WebSocketsClient webSocket;

void setupWebSocket() {
  SystemConfig& config = getConfig();

  Serial.printf("Connecting to printer at ws://%s:%d%s\n", config.printerIP, config.printerPort, PRINTER_WS_PATH);
  webSocket.begin(config.printerIP, config.printerPort, PRINTER_WS_PATH);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Disconnected!");
      break;

    case WStype_CONNECTED:
      Serial.println("[WS] Connected to printer!");
      Serial.printf("[WS] URL: ws://%s%s\n", PRINTER_IP, PRINTER_WS_PATH);
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
        printerStatus.currentStatus = arr[0];
      }
    }

    printerStatus.bedTemp = statusObj["TempOfHotbed"] | 0.0f;
    printerStatus.nozzleTemp = statusObj["TempOfNozzle"] | 0.0f;
    printerStatus.chamberTemp = statusObj["TempOfBox"] | 0.0f;
    printerStatus.bedTargetTemp = statusObj["TempTargetHotbed"] | 0.0f;
    printerStatus.nozzleTargetTemp = statusObj["TempTargetNozzle"] | 0.0f;
    printerStatus.currentCoord = statusObj["CurrenCoord"].as<String>();

    if (!statusObj["CurrentFanSpeed"].isNull()) {
      JsonObject fanSpeed = statusObj["CurrentFanSpeed"];
      printerStatus.modelFan = fanSpeed["ModelFan"] | 0;
      printerStatus.auxFan = fanSpeed["AuxiliaryFan"] | 0;
      printerStatus.boxFan = fanSpeed["BoxFan"] | 0;
    }

    printerStatus.zOffset = statusObj["ZOffset"] | 0.0f;

    if (!statusObj["PrintInfo"].isNull()) {
      JsonObject printInfo = statusObj["PrintInfo"];
      printerStatus.printStatus = printInfo["Status"] | -1;
      printerStatus.currentLayer = printInfo["CurrentLayer"] | 0;
      printerStatus.totalLayers = printInfo["TotalLayer"] | 0;
      printerStatus.currentTicks = printInfo["CurrentTicks"] | 0;
      printerStatus.totalTicks = printInfo["TotalTicks"] | 0;
      printerStatus.progress = printInfo["Progress"] | 0;
      printerStatus.printSpeed = printInfo["PrintSpeedPct"] | 100;
      printerStatus.filename = printInfo["Filename"].as<String>();
    }

    if (!statusObj["LightStatus"].isNull()) {
      JsonObject lightStatus = statusObj["LightStatus"];
      int lightValue = lightStatus["SecondLight"].as<int>();
      printerStatus.lightOn = (lightValue == 1);
    }

    displayPrinterStatus();
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

void processWebSocket() {
  webSocket.loop();
}

WebSocketsClient& getWebSocket() {
  return webSocket;
}
