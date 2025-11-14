# CallMeBot WhatsApp-Benachrichtigungen

Diese Dokumentation beschreibt die WhatsApp-Benachrichtigungsfunktion über den CallMeBot-Service.

## Übersicht

Das Centauri Carbon Monitor System kann automatisch WhatsApp-Benachrichtigungen bei wichtigen Events senden:
- Filament-Runout (Filament ausgegangen)
- Filament-Jam (Filament blockiert/verstopft)
- Druck abgeschlossen (mit Druckdauer)

## Voraussetzungen

- WhatsApp auf deinem Smartphone installiert
- CallMeBot API-Key (kostenlos)
- Internetzugang für ESP32

## Einrichtung

### Schritt 1: CallMeBot API-Key erhalten

1. Öffne WhatsApp auf deinem Smartphone
2. Füge die Nummer **+34 644 40 92 48** zu deinen Kontakten hinzu (Name: "CallMeBot")
3. Sende folgende Nachricht an CallMeBot:
   ```
   I allow callmebot to send me messages
   ```
4. Du erhältst eine Antwort wie:
   ```
   CallMeBot API Activated for 491701234567
   Your apikey is: 1234567

   You can now send messages using the API.
   Send Stop to pause the Bot.
   Send Resume to enable it again.
   ```
5. Notiere dir:
   - Deine Telefonnummer (z.B. `491701234567`)
   - Deinen API-Key (z.B. `1234567`)

### Schritt 2: Im ESP32 System konfigurieren

1. Öffne das Dashboard im Browser: `http://<ESP32-IP>`
2. Klicke auf "⚙️ Settings" (oben rechts)
3. Scrolle zum Abschnitt "📱 WhatsApp-Benachrichtigungen (CallMeBot)"
4. Aktiviere die Checkbox "Benachrichtigungen aktivieren"
5. Trage deine Daten ein:
   - **Telefonnummer**: Mit Ländercode, ohne +, Leerzeichen oder Bindestriche
     - ✅ Richtig: `491701234567`
     - ❌ Falsch: `+49 170 123 45 67`
   - **API Key**: Genau wie von CallMeBot erhalten
     - ✅ Richtig: `1234567`
6. Klicke auf "💾 Speichern"

### Schritt 3: Testen

1. Klicke auf "📤 Test-Nachricht senden"
2. Du solltest innerhalb weniger Sekunden eine WhatsApp-Nachricht erhalten:
   ```
   Test Nachricht vom Centauri Carbon Monitor!
   ```
3. Wenn die Nachricht nicht ankommt, siehe [Troubleshooting](#troubleshooting)

## Benachrichtigungstypen

### 1. Filament-Runout

**Wann wird gesendet:**
- Der Filament-Switch-Sensor erkennt, dass kein Filament mehr vorhanden ist
- Während eines aktiven Drucks

**Nachricht:**
```
🚨 Centauri Carbon Alarm!

Filament-Runout erkannt!

Druck wurde pausiert.
```

**Verhalten:**
- Druck wird automatisch pausiert (wenn Auto-Pause aktiviert)
- Benachrichtigung wird gesendet
- Im Dashboard wird der Fehler angezeigt

### 2. Filament-Jam (Stau/Verstopfung)

**Wann wird gesendet:**
- Der Motion-Sensor erkennt keine Filament-Bewegung
- Obwohl der Druckkopf sich bewegt
- Nach Ablauf des konfigurierten Timeouts (Standard: 6,5 Sekunden)

**Nachricht:**
```
🚨 Centauri Carbon Alarm!

Filament-Stau erkannt!

Druck wurde pausiert.
```

**Verhalten:**
- Druck wird automatisch pausiert (wenn Auto-Pause aktiviert)
- Benachrichtigung wird gesendet
- Im Dashboard wird der Fehler angezeigt

### 3. Druck abgeschlossen

**Wann wird gesendet:**
- Der Drucker-Status wechselt von PRINTING zu STOPPED oder IDLE
- Nach erfolgreichem Abschluss des Drucks

**Nachricht:**
```
✅ Druck abgeschlossen!

Datei: test_model.gcode
Dauer: 2h 45min
```

**Informationen:**
- Dateiname des gedruckten Modells
- Gesamtdauer des Drucks (Stunden und Minuten)

## Rate-Limiting und API-Limits

### CallMeBot API-Limits

CallMeBot hat folgende Einschränkungen:
- **Minimum 3 Minuten** zwischen Nachrichten an dieselbe Nummer
- Bei Überschreitung kann dein API-Key temporär blockiert werden

### System-Cooldown

Unser System hat zusätzlich ein internes Rate-Limit:
- **60 Sekunden** zwischen Benachrichtigungen
- Verhindert versehentliches Spamming
- Schützt vor API-Blockierung

### Was bedeutet das in der Praxis?

- Wenn mehrere Fehler schnell hintereinander auftreten, wird nur die erste Benachrichtigung gesendet
- Die nächste Benachrichtigung kann erst nach 60 Sekunden gesendet werden
- Dies ist gewollt und schützt dich vor Nachrichtenflut
- Für normale Druckvorgänge ist dies kein Problem

## Sicherheit und Datenschutz

### Datenspeicherung

- API-Key wird verschlüsselt im ESP32 NVS (Non-Volatile Storage) gespeichert
- Telefonnummer wird ebenfalls im NVS gespeichert
- Daten bleiben auch nach Stromausfall erhalten
- Nur per Flash-Erase komplett löschbar

### API-Key-Schutz

- Der API-Key wird **niemals** über die Web-API übertragen
- Status-API zeigt nur `hasApiKey: true/false`
- Auch im Dashboard wird der Key nicht angezeigt
- Nach dem Speichern zeigt das Eingabefeld nur "****** (gespeichert)"

### Datenübertragung

- Kommunikation mit CallMeBot über HTTPS (verschlüsselt)
- Nachrichten werden URL-encoded übertragen
- Deutsche Umlaute (ä, ö, ü, ß) werden korrekt encoded

## Konfiguration über API

Alternativ zur Web-Oberfläche kannst du die Einstellungen auch per API setzen:

### CallMeBot-Einstellungen setzen

**Endpoint:** `POST /api/control`

**Body:**
```json
{
  "action": "setCallMeBotSettings",
  "enabled": true,
  "phone": "491701234567",
  "apiKey": "1234567"
}
```

**Response:**
```json
{
  "success": true,
  "message": "CallMeBot settings updated"
}
```

### Test-Benachrichtigung senden

**Endpoint:** `POST /api/control`

**Body:**
```json
{
  "action": "testNotification"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Test notification sent"
}
```

### Status abfragen

**Endpoint:** `GET /api/status`

**Response (Auszug):**
```json
{
  "notify": {
    "enabled": true,
    "phone": "491701234567",
    "hasApiKey": true
  }
}
```

## Troubleshooting

### Test-Nachricht kommt nicht an

**Problem:** Nach Klick auf "Test-Nachricht senden" kommt keine WhatsApp-Nachricht an.

**Lösungen:**

1. **Prüfe die Serial Monitor Ausgabe:**
   ```
   [CALLMEBOT] Notifications disabled
   ```
   → Lösung: Checkbox "Benachrichtigungen aktivieren" ist nicht gesetzt

2. **Prüfe die Serial Monitor Ausgabe:**
   ```
   [CALLMEBOT] Phone or API key not configured
   ```
   → Lösung: Telefonnummer oder API-Key fehlt

3. **Prüfe die Serial Monitor Ausgabe:**
   ```
   [CALLMEBOT] Response: 400
   [CALLMEBOT] ❌ Error: Invalid API key
   ```
   → Lösung: API-Key ist falsch oder abgelaufen

4. **Prüfe die Serial Monitor Ausgabe:**
   ```
   [CALLMEBOT] Rate limit: 45000 ms remaining
   ```
   → Lösung: Warte 45 Sekunden und versuche es erneut

5. **ESP32 hat keine Internet-Verbindung:**
   - Prüfe WiFi-Verbindung im Dashboard
   - Prüfe Router-Einstellungen
   - Prüfe ob DNS funktioniert

### Benachrichtigungen kommen verzögert

**Ursache:** CallMeBot API kann bei hoher Last verzögert antworten

**Normal:**
- 1-5 Sekunden Verzögerung ist normal
- Bei sehr hoher Last bis zu 30 Sekunden

**Wenn länger als 1 Minute:**
- Prüfe CallMeBot-Status: [https://api.callmebot.com/status](https://api.callmebot.com/status)
- API könnte temporär überlastet sein

### Keine Benachrichtigung bei Filament-Fehler

**Problem:** Filament-Fehler wird erkannt, aber keine Benachrichtigung gesendet.

**Checkliste:**

1. ✅ Sind Benachrichtigungen aktiviert? (Settings-Seite prüfen)
2. ✅ Ist der letzte Cooldown abgelaufen? (Serial Monitor: "Rate limit" Meldung)
3. ✅ Hat ESP32 Internet-Verbindung? (Dashboard WiFi-Status prüfen)
4. ✅ Ist API-Key noch gültig? (Test-Nachricht senden)

### API-Key funktioniert plötzlich nicht mehr

**Ursachen:**

1. **Bot pausiert:**
   - Du hast "Stop" an CallMeBot gesendet
   - Lösung: Sende "Resume" an CallMeBot (+34 644 40 92 48)

2. **Zu viele Nachrichten gesendet:**
   - API-Limit überschritten
   - Lösung: 24 Stunden warten, dann wieder normal

3. **API-Key abgelaufen:**
   - Sehr selten, aber möglich
   - Lösung: Neuen API-Key anfordern (Schritt 1 wiederholen)

## Fehlercode-Referenz

### HTTP Response Codes

| Code | Bedeutung | Lösung |
|------|-----------|--------|
| 200  | OK - Nachricht gesendet | Alles gut! |
| 400  | Bad Request - Ungültige Parameter | API-Key oder Telefonnummer prüfen |
| 401  | Unauthorized - API-Key ungültig | Neuen API-Key anfordern |
| 429  | Too Many Requests - Rate Limit | Warten und erneut versuchen |
| 500  | Server Error - CallMeBot Problem | Später erneut versuchen |

### Serial Monitor Debug-Codes

```
[CALLMEBOT] Module initialized
[CALLMEBOT]   Enabled: Yes/No
[CALLMEBOT]   Phone: <phone>
[CALLMEBOT]   API Key: ***
```
→ Zeigt Status beim ESP32-Start

```
[CALLMEBOT] Sending notification: <message>
[CALLMEBOT] Response: 200
[CALLMEBOT] ✅ Notification sent successfully
```
→ Erfolgreiche Nachricht

```
[CALLMEBOT] ❌ Request failed: Connection refused
```
→ Netzwerkproblem, prüfe Internet-Verbindung

```
[CALLMEBOT] Rate limit: 45000 ms remaining
```
→ Cooldown aktiv, noch 45 Sekunden warten

## Häufig gestellte Fragen (FAQ)

### Kann ich mehrere Telefonnummern gleichzeitig benachrichtigen?

Nein, aktuell wird nur eine Telefonnummer unterstützt. Du könntest aber eine WhatsApp-Gruppe erstellen und die Nachrichten dort teilen.

### Kostet CallMeBot etwas?

CallMeBot ist komplett kostenlos. Es gibt Limits (siehe Rate-Limiting), aber keine Kosten.

### Kann ich die Nachrichtentexte anpassen?

Ja! Die Texte sind in `src/callmebot.cpp` definiert:
- `notifyFilamentError()` - Filament-Fehler-Nachrichten
- `notifyPrintComplete()` - Druck-abgeschlossen-Nachricht

Du kannst die Texte nach deinen Wünschen anpassen und neu kompilieren.

### Werden meine Daten an Dritte weitergegeben?

- CallMeBot speichert deine Telefonnummer und API-Key
- Nachrichten werden über deren Server geleitet
- Siehe CallMeBot Datenschutzerklärung für Details

### Kann ich einen anderen Benachrichtigungsdienst verwenden?

Das System ist modular aufgebaut. Du könntest das `callmebot` Modul durch einen anderen Service ersetzen (z.B. Telegram, Pushover, etc.). Die Integration ist ähnlich strukturiert.

### Warum werden Emojis verwendet?

Emojis machen die Nachrichten übersichtlicher und schneller erfassbar:
- 🚨 = Alarm/Fehler
- ✅ = Erfolg/Abschluss
- 🖨️ = Druck-bezogen

Sie werden korrekt URL-encoded und funktionieren auf allen Geräten.

## Technische Details

### Implementierung

**Module:**
- `src/callmebot.h` - Header mit Funktionsdeklarationen
- `src/callmebot.cpp` - Implementierung

**Verwendete Libraries:**
- `HTTPClient.h` - HTTPS-Requests
- `Preferences.h` - Persistenter Storage

**API-Aufruf:**
```cpp
String url = "https://api.callmebot.com/whatsapp.php?phone=" + phone +
             "&text=" + encodedMessage +
             "&apikey=" + apiKey;
HTTPClient http;
http.begin(url);
int httpCode = http.GET();
```

### URL-Encoding

Deutsche Umlaute werden korrekt encoded:
- ä → %C3%A4
- ö → %C3%B6
- ü → %C3%BC
- ß → %C3%9F
- Leerzeichen → +
- Newline → %0A

### Thread-Safety

- Alle Funktionen sind thread-safe
- Rate-Limiting nutzt `millis()` für Zeitstempel
- Keine Race-Conditions bei gleichzeitigen Aufrufen

## Support

Bei Problemen:

1. **Serial Monitor prüfen** (115200 baud) für Debug-Ausgaben
2. **CallMeBot Status prüfen**: Sende "Status" an +34 644 40 92 48
3. **GitHub Issues**: [https://github.com/...](https://github.com/)

## Credits

- CallMeBot API: [https://www.callmebot.com](https://www.callmebot.com)
- Entwickelt für Elegoo Centauri Carbon 3D-Drucker
