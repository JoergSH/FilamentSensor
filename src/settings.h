/*
 * Settings Page HTML
 * Configuration, OTA Updates, and Testing
 */

#ifndef SETTINGS_H
#define SETTINGS_H

const char* getSettingsHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Einstellungen - Centauri Monitor</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }

    body {
      font-family: 'Segoe UI', system-ui, sans-serif;
      background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
      color: #fff;
      min-height: 100vh;
      padding: 20px;
    }

    .container {
      max-width: 800px;
      margin: 0 auto;
    }

    header {
      text-align: center;
      margin-bottom: 30px;
      padding: 20px;
      background: rgba(255,255,255,0.1);
      border-radius: 15px;
      backdrop-filter: blur(10px);
    }

    h1 {
      font-size: 2.5em;
      margin-bottom: 10px;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
    }

    .nav-buttons {
      display: flex;
      gap: 10px;
      margin-bottom: 20px;
    }

    .btn {
      padding: 12px 24px;
      border: none;
      border-radius: 8px;
      font-size: 1em;
      cursor: pointer;
      transition: all 0.3s ease;
      font-weight: 600;
      flex: 1;
    }

    .btn-primary {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
    }

    .btn-primary:hover {
      transform: translateY(-2px);
      box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
    }

    .settings-section {
      background: rgba(255, 255, 255, 0.1);
      backdrop-filter: blur(10px);
      border-radius: 15px;
      padding: 25px;
      margin-bottom: 20px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
    }

    .settings-section h2 {
      margin-bottom: 20px;
      font-size: 1.5em;
      border-bottom: 2px solid rgba(255,255,255,0.2);
      padding-bottom: 10px;
    }

    .form-group {
      margin-bottom: 15px;
    }

    .form-group label {
      display: block;
      margin-bottom: 5px;
      font-weight: 600;
    }

    .form-group input {
      width: 100%;
      padding: 10px;
      border-radius: 5px;
      border: 1px solid rgba(255,255,255,0.3);
      background: rgba(255,255,255,0.1);
      color: #fff;
      font-size: 1em;
    }

    .form-group input::placeholder {
      color: rgba(255,255,255,0.5);
    }

    .controls {
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
      margin-top: 15px;
    }

    .btn-success {
      background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%);
      color: white;
    }

    .btn-success:hover {
      transform: translateY(-2px);
      box-shadow: 0 5px 15px rgba(56, 239, 125, 0.4);
    }

    .btn-secondary {
      background: rgba(255,255,255,0.2);
      color: white;
    }

    .btn-secondary:hover {
      background: rgba(255,255,255,0.3);
    }

    .btn-warning {
      background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
      color: white;
    }

    .btn-danger {
      background: linear-gradient(135deg, #eb3349 0%, #f45c43 100%);
      color: white;
    }

    .status-message {
      margin-top: 10px;
      padding: 10px;
      border-radius: 5px;
      text-align: center;
      font-weight: bold;
    }

    .info-row {
      display: flex;
      justify-content: space-between;
      padding: 10px 0;
      border-bottom: 1px solid rgba(255,255,255,0.1);
    }

    .info-label {
      opacity: 0.8;
    }

    .info-value {
      font-weight: 600;
    }

    #otaProgress, #runoutTestResult {
      margin-top: 15px;
    }

    .progress-bar-container {
      background: rgba(255,255,255,0.2);
      border-radius: 10px;
      overflow: hidden;
      height: 30px;
    }

    .progress-bar {
      background: #4CAF50;
      height: 100%;
      width: 0%;
      transition: width 0.3s;
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: bold;
    }

    .test-result {
      padding: 10px;
      background: rgba(0,0,0,0.3);
      border-radius: 5px;
      font-family: monospace;
      font-size: 0.9em;
      text-align: center;
      font-weight: bold;
    }

    @media (max-width: 768px) {
      h1 { font-size: 1.8em; }
      .btn { width: 100%; margin: 5px 0; }
      .controls { flex-direction: column; }
    }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <h1>⚙️ Einstellungen</h1>
      <p>Konfiguration & Verwaltung</p>
    </header>

    <div class="nav-buttons">
      <button class="btn btn-primary" onclick="window.location.href='/'">
        🏠 Zurück zum Dashboard
      </button>
    </div>

    <!-- WiFi & Printer Settings -->
    <div class="settings-section">
      <h2>🌐 Netzwerk & Drucker</h2>

      <div class="form-group">
        <label>WiFi SSID:</label>
        <input type="text" id="wifiSSID" placeholder="SSID">
      </div>

      <div class="form-group">
        <label>WiFi Passwort:</label>
        <input type="password" id="wifiPassword" placeholder="Passwort">
      </div>

      <div class="form-group">
        <label>Drucker IP-Adresse:</label>
        <input type="text" id="printerIP" placeholder="192.168.1.100">
      </div>

      <div class="form-group">
        <label>Drucker Port:</label>
        <input type="number" id="printerPort" placeholder="80" value="80">
      </div>

      <div class="controls">
        <button class="btn btn-success" onclick="saveSettings()">
          💾 Speichern
        </button>
        <button class="btn btn-secondary" onclick="loadSettings()">
          🔄 Neu laden
        </button>
      </div>

      <div id="settingsStatus" class="status-message" style="display:none;"></div>
    </div>

    <!-- OTA Update Section -->
    <div class="settings-section">
      <h2>🔄 Firmware Update (OTA)</h2>

      <div class="info-row">
        <span class="info-label">Aktuelle Partition:</span>
        <span class="info-value" id="currentPartition">-</span>
      </div>
      <div class="info-row">
        <span class="info-label">Nächste Partition:</span>
        <span class="info-value" id="nextPartition">-</span>
      </div>

      <div class="controls" style="margin-top: 20px;">
        <input type="file" id="firmwareFile" accept=".bin" style="display: none;" onchange="uploadFirmware()">
        <button class="btn btn-success" onclick="document.getElementById('firmwareFile').click()">
          📁 Firmware auswählen
        </button>
        <button class="btn btn-secondary" onclick="checkOTAStatus()">
          📊 Status prüfen
        </button>
      </div>

      <div id="otaProgress" style="display: none;">
        <div class="progress-bar-container">
          <div id="otaProgressBar" class="progress-bar">0%</div>
        </div>
        <div id="otaStatus" style="margin-top: 10px; text-align: center;">Warte auf Upload...</div>
      </div>
    </div>

    <!-- Runout Pin Test Section -->
    <div class="settings-section">
      <h2>🔧 Runout Pin Test (IO2)</h2>
      <p style="opacity: 0.8; margin-bottom: 15px;">
        Manuelles Testen des Runout-Pins. Im Normalbetrieb wird SENSOR_SWITCH automatisch durchgeschleift.
      </p>

      <div class="controls">
        <button class="btn btn-success" onclick="setRunoutOutput(true)">
          ⬆️ HIGH setzen
        </button>
        <button class="btn btn-danger" onclick="setRunoutOutput(false)">
          ⬇️ LOW setzen
        </button>
      </div>

      <div style="margin-top: 10px;">
        <button class="btn btn-secondary" onclick="readRunoutState()" style="width: 100%;">
          📖 Aktuellen Zustand lesen
        </button>
      </div>

      <div id="runoutTestResult" class="test-result">
        Klicke einen Button um IO2 zu steuern
      </div>
    </div>

  </div>

  <script>
    // Load settings on page load
    window.onload = function() {
      loadSettings();
      checkOTAStatus();
    };

    async function loadSettings() {
      try {
        const response = await fetch('/api/config');
        const data = await response.json();

        document.getElementById('wifiSSID').value = data.wifiSSID || '';
        document.getElementById('printerIP').value = data.printerIP || '';
        document.getElementById('printerPort').value = data.printerPort || 80;

        showStatus('settingsStatus', '✅ Einstellungen geladen', 'success');
      } catch (error) {
        console.error('Fehler beim Laden:', error);
        showStatus('settingsStatus', '❌ Fehler beim Laden', 'error');
      }
    }

    async function saveSettings() {
      const wifiSSID = document.getElementById('wifiSSID').value;
      const wifiPassword = document.getElementById('wifiPassword').value;
      const printerIP = document.getElementById('printerIP').value;
      const printerPort = parseInt(document.getElementById('printerPort').value) || 80;

      if (!printerIP) {
        alert('Bitte mindestens eine Drucker-IP eingeben!');
        return;
      }

      const ipPattern = /^(\d{1,3}\.){3}\d{1,3}$/;
      if (!ipPattern.test(printerIP)) {
        alert('Ungültige IP-Adresse!');
        return;
      }

      const config = {
        printerIP: printerIP,
        printerPort: printerPort
      };

      if (wifiSSID && wifiPassword) {
        config.wifiSSID = wifiSSID;
        config.wifiPassword = wifiPassword;
      }

      try {
        const response = await fetch('/api/config', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(config)
        });

        const result = await response.json();

        if (result.success) {
          showStatus('settingsStatus', '✅ ' + result.message, 'success');

          if (result.needsRestart) {
            if (confirm('Einstellungen gespeichert!\n\nFür WiFi-Änderungen ist ein Neustart erforderlich.\nJetzt neu starten?')) {
              await fetch('/api/control', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ action: 'restart' })
              });
              alert('ESP32 wird neu gestartet...\nBitte warten Sie 10 Sekunden.');
            }
          }
        } else {
          showStatus('settingsStatus', '❌ ' + result.message, 'error');
        }
      } catch (error) {
        console.error('Fehler beim Speichern:', error);
        showStatus('settingsStatus', '❌ Fehler beim Speichern', 'error');
      }
    }

    async function checkOTAStatus() {
      try {
        const response = await fetch('/api/ota/status');
        const data = await response.json();

        document.getElementById('currentPartition').textContent = data.currentPartition;
        document.getElementById('nextPartition').textContent = data.nextPartition;
      } catch (error) {
        console.error('OTA Status Fehler:', error);
      }
    }

    async function uploadFirmware() {
      const fileInput = document.getElementById('firmwareFile');
      const file = fileInput.files[0];

      if (!file) return;

      if (!file.name.endsWith('.bin')) {
        alert('Bitte nur .bin Dateien auswählen!');
        return;
      }

      if (!confirm('Firmware "' + file.name + '" hochladen?\n\nDer ESP32 wird nach erfolgreichem Upload neu gestartet.')) {
        fileInput.value = '';
        return;
      }

      const formData = new FormData();
      formData.append('firmware', file);

      const progressDiv = document.getElementById('otaProgress');
      const progressBar = document.getElementById('otaProgressBar');
      const statusDiv = document.getElementById('otaStatus');

      progressDiv.style.display = 'block';
      progressBar.style.width = '0%';
      progressBar.textContent = '0%';
      statusDiv.textContent = 'Upload läuft...';

      try {
        const xhr = new XMLHttpRequest();

        xhr.upload.addEventListener('progress', (e) => {
          if (e.lengthComputable) {
            const percentComplete = Math.round((e.loaded / e.total) * 100);
            progressBar.style.width = percentComplete + '%';
            progressBar.textContent = percentComplete + '%';
          }
        });

        xhr.addEventListener('load', () => {
          if (xhr.status === 200) {
            const response = JSON.parse(xhr.responseText);
            progressBar.style.width = '100%';
            progressBar.textContent = '100%';
            statusDiv.textContent = response.message || 'Upload erfolgreich! Neustart...';
            progressBar.style.background = '#4CAF50';

            setTimeout(() => {
              alert('Firmware erfolgreich aktualisiert!\n\nDer ESP32 startet jetzt neu.\nBitte warten Sie ca. 10 Sekunden und laden Sie die Seite neu.');
            }, 1000);
          } else {
            const response = JSON.parse(xhr.responseText);
            statusDiv.textContent = 'Fehler: ' + (response.message || 'Upload fehlgeschlagen');
            progressBar.style.background = '#dc3545';
            alert('Upload fehlgeschlagen: ' + (response.message || 'Unbekannter Fehler'));
          }
          fileInput.value = '';
        });

        xhr.addEventListener('error', () => {
          statusDiv.textContent = 'Upload fehlgeschlagen';
          progressBar.style.background = '#dc3545';
          alert('Upload fehlgeschlagen: Netzwerkfehler');
          fileInput.value = '';
        });

        xhr.open('POST', '/api/ota/upload');
        xhr.send(formData);

      } catch (error) {
        console.error('Upload Fehler:', error);
        statusDiv.textContent = 'Fehler: ' + error.message;
        progressBar.style.background = '#dc3545';
        alert('Upload fehlgeschlagen: ' + error.message);
        fileInput.value = '';
      }
    }

    async function setRunoutOutput(state) {
      try {
        const resultDiv = document.getElementById('runoutTestResult');
        resultDiv.textContent = state ? 'Setze IO2 auf HIGH...' : 'Setze IO2 auf LOW...';
        resultDiv.style.color = '#ffc107';

        const response = await fetch('/api/test/runout/set?state=' + (state ? '1' : '0'));
        const data = await response.json();

        if (data.success) {
          resultDiv.textContent = state ? '✅ IO2 = HIGH gesetzt' : '✅ IO2 = LOW gesetzt';
          resultDiv.style.color = state ? '#4CAF50' : '#dc3545';
          resultDiv.style.fontSize = '1.2em';
        } else {
          resultDiv.textContent = 'Fehler: ' + data.message;
          resultDiv.style.color = '#dc3545';
        }
      } catch (error) {
        console.error('Runout Set Fehler:', error);
        const resultDiv = document.getElementById('runoutTestResult');
        resultDiv.textContent = 'Fehler: ' + error.message;
        resultDiv.style.color = '#dc3545';
      }
    }

    async function readRunoutState() {
      try {
        const resultDiv = document.getElementById('runoutTestResult');
        resultDiv.textContent = 'Lese Pin IO2...';
        resultDiv.style.color = '#ffc107';

        const response = await fetch('/api/test/runout/read');
        const data = await response.json();

        if (data.success) {
          resultDiv.textContent = data.result;
          const isHigh = data.result.includes('HIGH');
          resultDiv.style.color = isHigh ? '#4CAF50' : '#ffc107';
          resultDiv.style.fontSize = '1.2em';
        } else {
          resultDiv.textContent = 'Fehler: ' + data.message;
          resultDiv.style.color = '#dc3545';
        }
      } catch (error) {
        console.error('Runout Read Fehler:', error);
        const resultDiv = document.getElementById('runoutTestResult');
        resultDiv.textContent = 'Fehler: ' + error.message;
        resultDiv.style.color = '#dc3545';
      }
    }

    function showStatus(elementId, message, type) {
      const statusDiv = document.getElementById(elementId);
      statusDiv.style.display = 'block';
      statusDiv.textContent = message;
      statusDiv.style.background = type === 'success' ? 'rgba(76, 175, 80, 0.3)' : 'rgba(244, 67, 54, 0.3)';
      statusDiv.style.color = type === 'success' ? '#4CAF50' : '#f44336';

      setTimeout(() => {
        statusDiv.style.display = 'none';
      }, 5000);
    }
  </script>
</body>
</html>
  )rawliteral";
}

#endif // SETTINGS_H
