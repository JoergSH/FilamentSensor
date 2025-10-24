/*
 * WebSocket Client for Printer Communication
 * Handles all WebSocket communication with the Elegoo printer
 */

#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// Initialize WebSocket connection
void setupWebSocket();

// WebSocket event handler
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);

// Send command to printer
void sendCommand(int cmd);
void sendCommand(int cmd, JsonObject *data);

// Request printer status
void requestStatus();

// Send ping to keep connection alive
void sendPing();

// Parse incoming messages
void parseMessage(char* payload);

// Process WebSocket loop
void processWebSocket();

// Get WebSocket instance (for direct access if needed)
WebSocketsClient& getWebSocket();

#endif // WEBSOCKET_CLIENT_H
