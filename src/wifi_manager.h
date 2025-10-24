/*
 * WiFi Connection Manager
 * Handles WiFi connectivity
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

// Initialize and connect to WiFi
void connectWiFi();

// Check if WiFi is connected
bool isWiFiConnected();

#endif // WIFI_MANAGER_H
