# Centauri Carbon Monitor

ESP32-basierte Überwachungslösung für den Elegoo Centauri Carbon 3D-Drucker mit Filament-Sensor und Web-Dashboard.

Getestet mit der Aktuellen Firmware 1.1.46

![1763177370786](image/README/1763177370786.png)Features

- ✅ **Echtzeit-Druckerüberwachung** via WebSocket
- ✅ **Filament-Runout-Detektion** (Sensor erkennt fehlendes Filament)
- ✅ **Filament-Jam-Detektion** (Motion-Sensor erkennt Verstopfung)
- ✅ **Automatische Pause** bei Filament-Problemen
- ✅ **Zwei Betriebsmodi**:
  - **Direct Mode**: Sensor-Signal direkt an Drucker (Hardware-Erkennung)
  - **Pause Mode**: Software-basierte Pause-Befehle via WebSocket
- ✅ **Intelligente Fehlererkennungs**:
  - Keine False-Positives beim Druckstart (wartet auf erste Filament-Bewegung)
  - Keine Fehler auf letzter Schicht (Parking-Phase)
  - Optimierte Motion-Sensor-Performance (vermeidet pinMode-Blocking)
- ✅ **Persistente Einstellungen** (gespeichert im Flash)
- ✅ **Web-Dashboard** für Überwachung und Steuerung
- ✅ **OTA-Updates** über Web-Interface
- ✅ **Drucker-Kamera Integration** (Live-Stream im Dashboard)
- ✅ **WhatsApp-Benachrichtigungen** via CallMeBot (Filament-Fehler, Druck abgeschlossen)
- ✅ **REST API** für externe Integrationen

## Hardware

- **ESP32-C3 Super Mini** (oder kompatibel)
- BIGTREETECH Smart Filament Sensor V2.0 oder kompatibel
- **Filament Switch Sensor** (Pin 1) - Erkennt Filament-Präsenz
- **Filament Motion Sensor** (Pin 0) - Erkennt Filament-Bewegung
- **Filament Switch** **Output** zum Drucker

## Modulare Architektur

Das Projekt ist in logische Module aufgeteilt:

### Core Module

- **[config.h](src/config.h)** / **[config.cpp](src/config.cpp)**

  - Pin-Definitionen
  - Timeout-Konfigurationen
- **[main.cpp](src/main.cpp)**

  - Hauptprogramm
  - Initialisierung aller Module
  - Haupt-Loop-Koordination

### Netzwerk-Module

- **[wifi_manager.h](src/wifi_manager.h)** / **[wifi_manager.cpp](src/wifi_manager.cpp)**

  - WiFi-Verbindungsmanagement
  - Automatische Reconnect-Logik
- **[websocket_client.h](src/websocket_client.h)** / **[websocket_client.cpp](src/websocket_client.cpp)**

  - WebSocket-Verbindung zum Drucker
  - Senden von Befehlen
  - Empfangen und Parsen von Status-Updates
- **[web_server.h](src/web_server.h)** / **[web_server.cpp](src/web_server.cpp)**

  - HTTP-Webserver (Port 80)
  - REST API-Endpunkte
  - Dashboard-Bereitstellung

### Drucker-Module

- **[printer_status.h](src/printer_status.h)** / **[printer_status.cpp](src/printer_status.cpp)**

  - PrinterStatus-Struktur
  - Status-Anzeige-Funktionen
  - Verwendet printer_status_codes.h
- **[printer_status_codes.h](src/printer_status_codes.h)**

  - Status-Code-Definitionen (IDLE, PRINTING, PAUSED, etc.)
  - Status-Text-Konvertierung
- **[printer_control.h](src/printer_control.h)** / **[printer_control.cpp](src/printer_control.cpp)**

  - Druckersteuerungsfunktionen
  - startPrint(), pausePrint(), resumePrint(), cancelPrint()
  - toggleLight()

### Sensor-Modul

- **[filament_sensor.h](src/filament_sensor.h)** / **[filament_sensor.cpp](src/filament_sensor.cpp)**
  - Filament-Sensor-Initialisierung
  - Runout-Detection (Switch-basiert)
  - Jam-Detection (Motion-Sensor mit Timeout)
  - Zwei Betriebsmodi (Direct/Pause)
  - Automatische Pause-Logik mit intelligenter Fehlererkennungs
  - Interrupt-basierte Motion-Erkennung (IRAM_ATTR)
  - Persistente Einstellungen (ESP32 NVS)
  - Optimiertes Pin-Handling (vermeidet pinMode-Blocking)

### Benachrichtigungs-Modul

- **[callmebot.h](src/callmebot.h)** / **[callmebot.cpp](src/callmebot.cpp)**
  - WhatsApp-Benachrichtigungen via CallMeBot API
  - Benachrichtigungen bei Filament-Runout und Filament-Jam
  - Benachrichtigung bei Druck abgeschlossen (mit Druckdauer)
  - Rate-Limiting (60 Sekunden Cooldown)
  - URL-Encoding für deutsche Umlaute (ä, ö, ü, ß)
  - Persistente Einstellungen (ESP32 NVS)
  - Konfiguration über Web-Interface

### Web-Interface

- **[dashboard.h](src/dashboard.h)**
  - Vollständiges HTML/CSS/JavaScript Dashboard
  - Responsive Design
  - Echtzeit-Updates (500ms)
  - Touch-optimiert

## API-Endpunkte

### GET /

Web-Dashboard (HTML-Oberfläche)

### GET /api/status

Gibt aktuellen Status als JSON zurück:

```json
{
  "status": {
    "state": 11,
    "stateText": "PRINTING",
    "position": "X:120.5 Y:85.3 Z:15.2",
    "zOffset": 0.0,
    "lightOn": true,
    "bedTemp": 60.0,
    "bedTarget": 60.0,
    "nozzleTemp": 220.0,
    "nozzleTarget": 220.0,
    "chamberTemp": 35.0
  },
  "print": {
    "progress": 45,
    "filename": "model.gcode",
    "layer": 150,
    "totalLayers": 300,
    "speed": 100
  },
  "fans": {
    "model": 100,
    "aux": 50,
    "box": 30
  },
  "sensor": {
    "error": false,
    "noFilament": false,
    "lastMotion": 250,
    "pulseCount": 1234,
    "autoPause": true,
    "pauseDelay": 3000,
    "switchDirectMode": true
  },
  "notify": {
    "enabled": true,
    "phone": "491701234567",
    "hasApiKey": true
  }
}
```

### POST /api/control

Sendet Steuerungsbefehle:

**Actions:**

- `pause` - Druck pausieren
- `resume` - Druck fortsetzen
- `cancel` - Druck abbrechen
- `toggleLight` - Licht umschalten
- `toggleAutoPause` - Auto-Pause aktivieren/deaktivieren
- `toggleSwitchMode` - Zwischen Direct und Pause Mode umschalten
- `clearError` - Sensor-Fehler zurücksetzen
- `setPauseDelay` - Motion-Timeout setzen (ms)
- `setCallMeBotSettings` - CallMeBot-Einstellungen setzen (enabled, phone, apiKey)
- `testNotification` - Test-Benachrichtigung senden
- `restart` - ESP32 neu starten

**Beispiel:**

```json
{
  "action": "pause"
}
```

## WhatsApp-Benachrichtigungen (CallMeBot)

Das System kann automatisch WhatsApp-Benachrichtigungen über den CallMeBot-Service senden.

### Einrichtung

1. **CallMeBot API-Key erhalten**:

   - Sende "I allow callmebot to send me messages" an WhatsApp-Nummer: **+34 644 40 92 48**
   - Du erhältst eine Antwort mit deinem API-Key
   - Beispiel: `Your apikey is: 1234567`
2. **In den Settings konfigurieren**:

   - Öffne die Settings-Seite im Dashboard
   - Scrolle zu "📱 WhatsApp-Benachrichtigungen (CallMeBot)"
   - Aktiviere "Benachrichtigungen aktivieren"
   - Trage deine Telefonnummer ein (mit Ländercode, z.B. `491701234567`)
   - Trage deinen API-Key ein (z.B. `1234567`)
   - Klicke auf "💾 Speichern"
3. **Test-Nachricht senden**:

   - Klicke auf "📤 Test-Nachricht senden"
   - Du solltest eine WhatsApp-Nachricht erhalten

### Benachrichtigungstypen

Das System sendet automatisch Benachrichtigungen bei folgenden Events:

1. **Filament-Runout erkannt**

   ```
   🚨 Centauri Carbon Alarm!

   Filament-Runout erkannt!

   Druck wurde pausiert.
   ```
2. **Filament-Stau erkannt**

   ```
   🚨 Centauri Carbon Alarm!

   Filament-Stau erkannt!

   Druck wurde pausiert.
   ```
3. **Druck abgeschlossen**

   ```
   ✅ Druck abgeschlossen!

   Datei: model.gcode
   Dauer: 2h 45min
   ```

### Rate-Limiting

- **CallMeBot API**: Minimum 3 Minuten zwischen Nachrichten (API-Limit)
- **System-Cooldown**: 60 Sekunden zwischen Benachrichtigungen
- Dies verhindert Spam und hält die API-Limits ein

### Sicherheit

- Der API-Key wird verschlüsselt im ESP32 NVS (Flash) gespeichert
- Der API-Key wird niemals in der Status-API angezeigt
- Nur die Information "hasApiKey: true/false" ist sichtbar

## Filament-Sensor Betriebsmodi

Der Filament-Sensor unterstützt zwei Betriebsmodi, die über das Dashboard umgeschaltet werden können:

### Direct Mode (Standard)

- **SENSOR_SWITCH** wird direkt an **RUNOUT_PIN** weitergeleitet
- Der Drucker erkennt Filament-Probleme über den Hardware-Pin
- Vorteile:
  - Funktioniert auch bei WebSocket-Verbindungsabbruch
  - Schnellste Reaktionszeit (Hardware-basiert)
  - Drucker-Firmware übernimmt Fehlerbehandlung
- Nachteile:
  - Drucker muss Runout-Sensor in Firmware aktiviert haben
  - Keine Software-basierte Filterung möglich

### Pause Mode

- **RUNOUT_PIN** bleibt immer HIGH (kein Fehler-Signal an Drucker)
- ESP32 sendet Pause-Befehl über WebSocket bei Filament-Problem
- Vorteile:
  - Funktioniert auch ohne Drucker-seitige Runout-Unterstützung
  - Intelligente Software-basierte Fehlererkennungs
  - Logs und Debugging über Serial Monitor
- Nachteile:
  - Benötigt aktive WebSocket-Verbindung
  - Minimal verzögerte Reaktion (Software-basiert)

### Intelligente Fehlererkennung

Beide Modi nutzen die intelligente Fehlererkennungs:

1. **Runout-Detection**: Sofortige Erkennung wenn SENSOR_SWITCH LOW wird
2. **Jam-Detection**: Motion-Timeout nur wenn:
   - Druck läuft (Status = PRINTING)
   - Druckkopf bewegt sich (Position-Check)
   - Bereits Filament-Bewegung während Druck erkannt wurde (verhindert False-Positives beim Start)
   - Nicht auf letzter Schicht (verhindert Fehler beim Beenden)

## Konfiguration

### Ersteinrichtung

Beim ersten Start verbinden Sie sich mit dem ESP:

1. WiFi-Netzwerk "Centauri-Monitor-Setup" suchen und verbinden
2. Im Browser die Setup-Seite aufrufen: `http://192.168.4.1`
3. Router-Zugangsdaten eingeben (SSID & Passwort)
4. IP-Adresse des Druckers im Netzwerk eingeben
5. Speichern - ESP startet neu und verbindet sich mit dem Router

### Sensor-Einstellungen (Dashboard)

Im Dashboard unter "Filament-Sensor":

- **Auto-Pause**: Automatisches Pausieren bei Filament-Problemen ein/aus
- **Mode**: Umschalten zwischen Direct und Pause Mode
- **Verzögerung vor Pause**: Motion-Timeout in Millisekunden (Standard: 3000ms)
  - Zu kurz: False-Positives bei Retractions
  - Zu lang: Verzögerte Jam-Erkennung

## Installation

### Option 1: Fertige Firmware flashen (Empfohlen für Anfänger)

Die einfachste Methode ist das Flashen der vorkompilierten Firmware:

1. **Firmware herunterladen**

   - Alle Dateien aus dem `firmware/` Ordner herunterladen
   - Oder als ZIP: [firmware.zip](firmware/)
2. **ESP Web Flasher verwenden**

   - Detaillierte Anleitung: [firmware/FLASH_INSTRUCTIONS.md](firmware/FLASH_INSTRUCTIONS.md)
   - Online-Tool: [https://espressif.github.io/esptool-js/](https://espressif.github.io/esptool-js/)
3. **Dateien flashen** (mit angegebenen Adressen):

   - `bootloader.bin` → 0x0
   - `partitions.bin` → 0x8000
   - `firmware.bin` → 0x10000

### Option 2: Selbst kompilieren (für Entwickler)

1. VS Code und PlatformIO installieren
2. Repository klonen
3. Projekt in VS Code öffnen
4. Kompilieren und hochladen:
   ```bash
   platformio run --target upload
   ```

## Zugriff auf das Dashboard

Nach dem Start und erfolgreicher WiFi-Verbindung:

1. IP-Adresse im Serial Monitor ablesen
2. Browser öffnen: `http://<ESP32-IP-Adresse>`
3. Dashboard wird angezeigt mit Live-Updates

## Status-Codes

| Code | Status   | Beschreibung               |
| ---- | -------- | -------------------------- |
| 0    | IDLE     | Drucker bereit             |
| 1    | HOMING   | Achsen werden referenziert |
| 6    | PAUSED   | Druck pausiert             |
| 11   | PRINTING | Druck läuft               |
| 9    | COMPLETE | Druck abgeschlossen        |

Vollständige Liste in [printer_status_codes.h](src/printer_status_codes.h)

## Dependencies

- **ESP32 Arduino Framework** (^3.2.0)
- **WebSockets** (^2.7.1) - Drucker-Kommunikation
- **ArduinoJson** (^7.4.2) - JSON-Parsing
- **ESPAsyncWebServer** (^3.6.0) - Web-Dashboard & OTA
- **HTTPClient** (^3.2.0) - CallMeBot API-Kommunikation
- **Preferences** (^3.2.0) - Persistente Einstellungen (ESP32 NVS)

## Troubleshooting

### WebSocket-Verbindung bricht ab

**Mögliche Ursachen**:

1. **WiFi-Signal schwach**: ESP32 zu weit vom Router
2. **Drucker-IP geändert**: IP im Settings-Interface aktualisieren
3. **Drucker neu gestartet**: WebSocket reconnect dauert ~5 Sekunden

**Lösung**:

- Prüfen Sie WiFi-Signal-Stärke
- Drucker statische IP im Router zuweisen
- Warten Sie auf automatischen Reconnect

## Entwicklung

### Debug-Ausgaben

Alle Module nutzen den Serial Monitor (115200 baud):

- `[WS]` - WebSocket-Events
- `[SENSOR]` - Filament-Sensor-Events
- `[SENSOR DEBUG]` - Pin-Status und Motion-Daten
- `[RUNOUT OUTPUT]` - Pin-Änderungen am RUNOUT_PIN
- `[WEB]` - Webserver-Events
- `[CALLMEBOT]` - WhatsApp-Benachrichtigungen
- `[STATUS]` - Printer-Status-Änderungen

### Performance-Optimierungen

1. **pinMode-Blocking vermeiden**: `setRunoutPinOutput()` nutzt static state tracking
2. **Interrupt-Safe**: Motion-ISR nutzt IRAM_ATTR und atomic operations
3. **Effiziente Checks**: Motion-Check nur alle 100ms, Position-Check alle 500ms

## Lizenz

MIT License

## Credits

Entwickelt für Elegoo Centauri Carbon 3D-Drucker
