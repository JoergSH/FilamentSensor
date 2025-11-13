# Centauri Carbon Monitor - Firmware

Dieses Verzeichnis enthält vorkompilierte Firmware-Dateien für den ESP32-C3 Super Mini.

## Dateien

- **bootloader.bin** (19 KB) - ESP32-C3 Bootloader
- **partitions.bin** (3 KB) - Partition Table (mit OTA-Unterstützung)
- **firmware.bin** (1.3 MB) - Haupt-Firmware

## Flash-Anleitung

**➡️ Siehe [FLASH_INSTRUCTIONS.md](FLASH_INSTRUCTIONS.md) für detaillierte Schritt-für-Schritt Anleitung**

## Kurzanleitung

1. ESP32-C3 via USB verbinden
2. [ESP Web Flasher](https://espressif.github.io/esptool-js/) öffnen
3. "Connect" klicken und Port auswählen
4. "Erase Flash" klicken
5. Dateien mit folgenden Adressen flashen:
   - `bootloader.bin` → **0x0**
   - `partitions.bin` → **0x8000**
   - `firmware.bin` → **0x10000**
6. "Program" klicken und warten
7. ESP32 neu starten

## Nach dem Flash

1. ESP32 startet im Access Point Modus
2. WiFi-Netzwerk "Centauri-Monitor-Setup" suchen
3. Verbinden und `http://192.168.4.1` öffnen
4. Router-Daten und Drucker-IP eingeben
5. Dashboard ist verfügbar unter der zugewiesenen IP

## Updates

Zukünftige Updates können über OTA (Over-The-Air) durchgeführt werden:
- Dashboard → Einstellungen → "firmware.bin" hochladen

## Hardware

Diese Firmware ist gebaut für:
- **Board**: ESP32-C3 Super Mini (oder kompatibel)
- **Flash**: 4 MB
- **Framework**: Arduino ESP32 v3.2.0

## Features

- ✅ Echtzeit-Druckerüberwachung
- ✅ Filament-Runout & Jam Detection
- ✅ Zwei Betriebsmodi (Direct/Pause)
- ✅ Web-Dashboard mit Live-Updates
- ✅ OTA-Update-Unterstützung
- ✅ Persistente Einstellungen
- ✅ Drucker-Kamera Integration

## Support

- **Dokumentation**: [../README.md](../README.md)
- **Issues**: [GitHub Issues](https://github.com/JoergSH/FilamentSensor/issues)
- **Drucker**: Elegoo Centauri Carbon (Firmware 1.1.46 getestet)
