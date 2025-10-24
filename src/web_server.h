/*
 * Web Server Module
 * Provides HTTP server with dashboard and REST API endpoints
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// Initialize web server
void setupWebServer();

// Process web server (if needed)
void processWebServer();

// Get server instance (for direct access if needed)
AsyncWebServer& getWebServer();

#endif // WEB_SERVER_H
