# Firmware Flash-Anleitung

## ESP Web Flasher (Empfohlen für Anfänger)

Der einfachste Weg die Firmware zu flashen ist über das ESP Web Flasher Tool von Espressif.

### Voraussetzungen

- **Google Chrome** oder **Microsoft Edge** Browser (Web Serial API erforderlich)
- USB-Kabel zum Verbinden des ESP32-C3

### Schritt-für-Schritt Anleitung

1. **ESP32-C3 mit dem Computer verbinden**
   - USB-Kabel an ESP32-C3 anschließen
   - Warten bis der Treiber installiert ist (Windows installiert automatisch)

2. **ESP Web Flasher öffnen**
   - Im Browser öffnen: [https://espressif.github.io/esptool-js/](https://espressif.github.io/esptool-js/)

3. **Firmware-Dateien vorbereiten**
   - Alle 3 Dateien aus diesem Ordner bereithalten:
     - `bootloader.bin`
     - `partitions.bin`
     - `firmware.bin`

4. **ESP32 verbinden**
   - Auf "Connect" klicken
   - Seriellen Port auswählen (z.B. "USB-SERIAL CH340 (COM3)")
   - Auf "Connect" klicken

5. **Firmware flashen**
   - **Wichtig**: Zuerst "Erase Flash" klicken und warten bis abgeschlossen
   - Dann die Dateien mit folgenden Adressen hinzufügen:

   | Datei | Adresse |
   |-------|---------|
   | `bootloader.bin` | 0x0 |
   | `partitions.bin` | 0x8000 |
   | `firmware.bin` | 0x10000 |

   - Auf "Program" klicken
   - Warten bis "Firmware successfully burned" angezeigt wird

6. **ESP32 neu starten**
   - Auf "Disconnect" klicken
   - ESP32 vom USB trennen und wieder anschließen
   - Oder Reset-Button drücken (falls vorhanden)

7. **WiFi konfigurieren**
   - ESP32 startet im Access Point Modus
   - Mit WiFi "Centauri-Monitor-Setup" verbinden
   - Browser öffnen: `http://192.168.4.1`
   - Router-Daten und Drucker-IP eingeben
   - Speichern - ESP startet neu

## Alternative: esptool.py (für Fortgeschrittene)

Falls Sie esptool.py installiert haben:

```bash
esptool.py --chip esp32c3 --port COM3 --baud 921600 \
  --before default_reset --after hard_reset write_flash \
  -z --flash_mode dio --flash_freq 80m --flash_size 4MB \
  0x0 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 firmware.bin
```

**Port anpassen**: Ersetzen Sie `COM3` mit Ihrem COM-Port (Windows) oder `/dev/ttyUSB0` (Linux)

## Troubleshooting

### Port nicht gefunden

- **Windows**: Überprüfen Sie im Geräte-Manager ob der CH340 Treiber installiert ist
- **Linux**: Fügen Sie sich zur `dialout` Gruppe hinzu: `sudo usermod -a -G dialout $USER`
- **Mac**: Installieren Sie den CH340 Treiber von [http://www.wch.cn/downloads/CH341SER_MAC_ZIP.html](http://www.wch.cn/downloads/CH341SER_MAC_ZIP.html)

### Flash fehlgeschlagen

1. Trennen Sie den ESP32 vom USB
2. Halten Sie die BOOT-Taste gedrückt (falls vorhanden)
3. Schließen Sie USB wieder an
4. Lassen Sie BOOT-Taste los
5. Versuchen Sie erneut zu flashen

### Nach Flash startet ESP32 nicht

- Führen Sie "Erase Flash" aus
- Flashen Sie alle 3 Dateien erneut mit korrekten Adressen
- Stellen Sie sicher, dass die Dateien nicht beschädigt sind

## Versions-Info

Diese Firmware-Dateien wurden gebaut für:
- **Board**: ESP32-C3 Super Mini (nologo_esp32c3_super_mini)
- **Framework**: Arduino ESP32 v3.2.0
- **Flash Size**: 4MB
- **Partition**: Default (OTA support)

## Nach dem Flash

Nach erfolgreichem Flash und WiFi-Konfiguration:

1. Finden Sie die IP-Adresse des ESP32 (wird im Setup angezeigt oder im Router)
2. Öffnen Sie `http://<ESP32-IP-Adresse>` im Browser
3. Das Dashboard wird angezeigt
4. Konfigurieren Sie die Filament-Sensor Einstellungen nach Bedarf

## Updates

Für zukünftige Updates können Sie OTA (Over-The-Air) verwenden:

1. Dashboard öffnen
2. Auf "⚙️ Einstellungen & OTA Update" klicken
3. "firmware.bin" Datei auswählen
4. Upload starten
5. Warten bis Update abgeschlossen und ESP neu gestartet ist

**Vorteil**: Kein USB-Kabel oder Flasher-Tool erforderlich!
