/*
 * Serial Configuration Helper Implementation
 */

#include "serial_config.h"
#include "config_manager.h"
#include <Arduino.h>

void printConfigMenu() {
  Serial.println("\n");
  Serial.println("╔═══════════════════════════════════════════╗");
  Serial.println("║   SERIAL CONFIGURATION MENU               ║");
  Serial.println("╠═══════════════════════════════════════════╣");
  Serial.println("║  Enter commands in Serial Monitor:        ║");
  Serial.println("║                                           ║");
  Serial.println("║  wifi:<ssid>:<password>                   ║");
  Serial.println("║  printer:<ip>:<port>                      ║");
  Serial.println("║  show                                     ║");
  Serial.println("║  restart                                  ║");
  Serial.println("║                                           ║");
  Serial.println("║  Example:                                 ║");
  Serial.println("║  wifi:MeinWiFi:geheim123                  ║");
  Serial.println("║  printer:192.168.1.100:80                 ║");
  Serial.println("╚═══════════════════════════════════════════╝");
  Serial.println();
}

void checkSerialConfig() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.length() == 0) {
      return;
    }

    Serial.printf("[SERIAL] Received: %s\n", command.c_str());

    if (command.startsWith("wifi:")) {
      // Format: wifi:ssid:password
      int firstColon = command.indexOf(':', 5);
      if (firstColon > 5) {
        String ssid = command.substring(5, firstColon);
        String password = command.substring(firstColon + 1);

        Serial.printf("[SERIAL] Setting WiFi: %s\n", ssid.c_str());
        updateWiFiConfig(ssid.c_str(), password.c_str());
        Serial.println("[SERIAL] ✓ WiFi configuration saved!");
        Serial.println("[SERIAL] Type 'restart' to apply changes.");
      } else {
        Serial.println("[SERIAL] ✗ Invalid format! Use: wifi:ssid:password");
      }
    }
    else if (command.startsWith("printer:")) {
      // Format: printer:ip:port
      int firstColon = command.indexOf(':', 8);
      if (firstColon > 8) {
        String ip = command.substring(8, firstColon);
        int port = command.substring(firstColon + 1).toInt();

        if (port == 0) port = 80;

        Serial.printf("[SERIAL] Setting Printer: %s:%d\n", ip.c_str(), port);
        updatePrinterConfig(ip.c_str(), port);
        Serial.println("[SERIAL] ✓ Printer configuration saved!");
        Serial.println("[SERIAL] Type 'restart' to apply changes.");
      } else {
        Serial.println("[SERIAL] ✗ Invalid format! Use: printer:192.168.1.100:80");
      }
    }
    else if (command == "show") {
      SystemConfig& config = getConfig();
      Serial.println("\n[SERIAL] Current Configuration:");
      Serial.printf("[SERIAL]   Configured: %s\n", config.configured ? "YES" : "NO");
      Serial.printf("[SERIAL]   WiFi SSID: %s\n", config.wifiSSID);
      Serial.printf("[SERIAL]   WiFi Password: %s\n", strlen(config.wifiPassword) > 0 ? "***" : "(empty)");
      Serial.printf("[SERIAL]   Printer IP: %s\n", config.printerIP);
      Serial.printf("[SERIAL]   Printer Port: %d\n", config.printerPort);
      Serial.println();
    }
    else if (command == "restart") {
      Serial.println("[SERIAL] Restarting ESP32...");
      delay(1000);
      ESP.restart();
    }
    else if (command == "help" || command == "?") {
      printConfigMenu();
    }
    else {
      Serial.println("[SERIAL] ✗ Unknown command. Type 'help' for menu.");
    }
  }
}
