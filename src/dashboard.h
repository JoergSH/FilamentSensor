#ifndef DASHBOARD_H
#define DASHBOARD_H

const char* getDashboardHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Centauri Carbon Monitor</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }

    body {
      font-family: 'Segoe UI', system-ui, sans-serif;
      background: #0a0a0a;
      color: #e0e0e0;
      padding: 15px;
      font-size: 14px;
    }

    .container {
      max-width: 800px;
      margin: 0 auto;
    }

    header {
      background: linear-gradient(135deg, #1a1a1a 0%, #2a2a2a 100%);
      border: 1px solid #ff0000;
      border-radius: 8px;
      padding: 15px;
      margin-bottom: 15px;
      text-align: center;
    }

    h1 {
      font-size: 1.8em;
      color: #ff0000;
      margin-bottom: 5px;
    }

    .subtitle {
      color: #888;
      font-size: 0.9em;
    }

    .grid {
      display: grid;
      grid-template-columns: 1fr;
      gap: 15px;
      margin-bottom: 15px;
    }

    @media (min-width: 700px) {
      .grid {
        grid-template-columns: repeat(2, 1fr);
      }
      .card-wide {
        grid-column: span 2;
      }
    }

    .card {
      background: #1a1a1a;
      border: 1px solid #333;
      border-radius: 8px;
      padding: 15px;
    }

    .card h2 {
      font-size: 1.2em;
      color: #ff0000;
      margin-bottom: 12px;
      padding-bottom: 8px;
      border-bottom: 1px solid #333;
      text-shadow: 0 0 8px rgba(255, 0, 0, 0.3);
    }

    .status-badge {
      display: inline-block;
      padding: 8px 16px;
      border-radius: 4px;
      font-weight: bold;
      font-size: 1.1em;
      margin: 8px 0;
      text-shadow: 0 0 5px rgba(0, 0, 0, 0.5);
    }

    .status-idle { background: #444; color: #fff; }
    .status-printing { background: #ff0000; color: #fff; animation: pulse 2s infinite; }
    .status-paused { background: #ff6600; color: #fff; }
    .status-complete { background: #00cc00; color: #fff; }
    .status-error { background: #ff0000; color: #fff; animation: flash 1s infinite; }

    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.7; }
    }

    @keyframes flash {
      0%, 50%, 100% { opacity: 1; }
      25%, 75% { opacity: 0.5; }
    }

    .info-row {
      display: flex;
      justify-content: space-between;
      padding: 6px 0;
      border-bottom: 1px solid #222;
    }

    .info-row:last-child {
      border-bottom: none;
    }

    .label {
      color: #888;
      font-size: 0.9em;
    }

    .value {
      color: #e0e0e0;
      font-weight: 500;
    }

    .temp-display {
      font-size: 1.3em;
      font-weight: bold;
      color: #ff0000;
    }

    .progress-bar {
      width: 100%;
      height: 25px;
      background: #222;
      border-radius: 4px;
      overflow: hidden;
      margin: 10px 0;
      position: relative;
    }

    .progress-fill {
      height: 100%;
      background: linear-gradient(90deg, #ff0000 0%, #ff6600 100%);
      transition: width 0.3s ease;
      display: flex;
      align-items: center;
      justify-content: center;
    }

    .progress-text {
      position: absolute;
      width: 100%;
      text-align: center;
      font-weight: bold;
      color: #fff;
      line-height: 25px;
      text-shadow: 0 0 3px #000;
    }

    button {
      background: #ff0000;
      color: #fff;
      border: none;
      padding: 10px 24px;
      border-radius: 6px;
      cursor: pointer;
      font-size: 1.1em;
      font-weight: 600;
      margin: 5px;
      transition: all 0.2s;
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.4);
    }

    button:hover {
      background: #cc0000;
      transform: translateY(-2px);
      box-shadow: 0 4px 12px rgba(255, 0, 0, 0.4);
    }

    button:active {
      transform: translateY(0);
    }

    button.secondary {
      background: #2a2a2a;
    }

    button.secondary:hover {
      background: #333;
      box-shadow: 0 4px 12px rgba(50, 50, 50, 0.3);
    }

    button.success {
      background: #005500;
    }

    button.success:hover {
      background: #006600;
      box-shadow: 0 4px 12px rgba(0, 100, 0, 0.4);
    }

    button.light-on {
      background: #997700;
      color: #fff;
    }

    button.light-on:hover {
      background: #aa8800;
      box-shadow: 0 4px 12px rgba(170, 136, 0, 0.5);
    }

    button.light-off {
      background: #333;
      color: #fff;
    }

    button.light-off:hover {
      background: #444;
    }

    .sensor-status {
      padding: 10px;
      background: #222;
      border-radius: 4px;
      margin: 8px 0;
    }

    .sensor-ok {
      border-left: 3px solid #00cc00;
    }

    .sensor-error {
      border-left: 3px solid #ff0000;
      animation: flash 1s infinite;
    }

    .camera-container {
      width: 100%;
      aspect-ratio: 16/9;
      background: #000;
      border-radius: 8px;
      overflow: hidden;
      position: relative;
    }

    .camera-container img {
      width: 100%;
      height: 100%;
      object-fit: contain;
    }

    .camera-offline {
      display: flex;
      align-items: center;
      justify-content: center;
      width: 100%;
      height: 100%;
      color: #666;
    }

    input[type="text"],
    input[type="number"],
    input[type="file"] {
      width: 100%;
      padding: 8px;
      background: #222;
      border: 1px solid #444;
      border-radius: 4px;
      color: #e0e0e0;
      font-size: 0.95em;
      margin: 5px 0;
    }

    input[type="checkbox"] {
      width: 18px;
      height: 18px;
      cursor: pointer;
    }

    label {
      display: flex;
      align-items: center;
      gap: 8px;
      margin: 8px 0;
      cursor: pointer;
    }

    .settings-section {
      background: #1a1a1a;
      border: 1px solid #333;
      border-radius: 8px;
      padding: 15px;
      margin-bottom: 15px;
    }

    .settings-section h3 {
      color: #ff0000;
      font-size: 1em;
      margin-bottom: 12px;
      padding-bottom: 8px;
      border-bottom: 1px solid #333;
    }

    .alert {
      padding: 12px;
      border-radius: 4px;
      margin: 10px 0;
      font-size: 0.95em;
    }

    .alert-success {
      background: #1a4d1a;
      border: 1px solid #00cc00;
      color: #00ff00;
    }

    .alert-error {
      background: #4d1a1a;
      border: 1px solid #ff0000;
      color: #ff6666;
    }

    .hidden {
      display: none;
    }

    #settingsBtn {
      position: fixed;
      top: 20px;
      right: 20px;
      z-index: 1000;
      background: #333;
      padding: 12px 20px;
      font-size: 1em;
    }

    #settingsBtn:hover {
      background: #444;
    }

    .modal {
      display: none;
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background: rgba(0,0,0,0.9);
      z-index: 2000;
      overflow-y: auto;
      padding: 20px;
    }

    .modal-content {
      max-width: 800px;
      margin: 0 auto;
    }

    .modal-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 20px;
    }

    .modal-header h2 {
      color: #ff0000;
      font-size: 1.5em;
    }

    .close-btn {
      background: #444;
      padding: 8px 16px;
      font-size: 1.1em;
    }

    @media (max-width: 768px) {
      .grid {
        grid-template-columns: 1fr;
      }

      h1 {
        font-size: 1.4em;
      }

      #settingsBtn {
        top: 10px;
        right: 10px;
        padding: 8px 16px;
        font-size: 0.9em;
      }
    }
  </style>
</head>
<body>
  <button id="settingsBtn" onclick="openSettings()">⚙️ Settings</button>

  <div class="container">
    <header>
      <h1>🖨️ Centauri Carbon Monitor</h1>
      <div class="subtitle">Filament-Sensor & Status-Überwachung</div>
    </header>

    <!-- 3-Spalten Grid (2 Spalten: Temps+Status, Sensor+Controls, Kamera über 2 Spalten unten) -->
    <div class="grid">
      <!-- Spalte 1: Temperaturen + Fortschritt + Status -->
      <div class="card">
        <h2>🌡️ Temps & Fortschritt</h2>
        <div id="statusBadge" class="status-badge status-idle" style="margin-bottom: 12px;">IDLE</div>
        <div class="info-row">
          <span class="label">Bett</span>
          <span id="bedTemp" class="temp-display">-°C</span>
        </div>
        <div class="info-row">
          <span class="label">Düse</span>
          <span id="nozzleTemp" class="temp-display">-°C</span>
        </div>
        <div class="info-row">
          <span class="label">Kammer</span>
          <span id="chamberTemp" class="value">-°C</span>
        </div>
        <div class="progress-bar">
          <div id="progressBar" class="progress-fill" style="width: 0%"></div>
          <div id="progressText" class="progress-text">0%</div>
        </div>
        <div class="info-row">
          <span class="label">Datei</span>
          <span id="filename" class="value">-</span>
        </div>
        <div class="info-row">
          <span class="label">Schicht</span>
          <span id="layer" class="value">-</span>
        </div>
        <div class="info-row">
          <span class="label">Speed</span>
          <span id="speed" class="value">-</span>
        </div>
      </div>

      <!-- Spalte 2: Filament-Sensor + Steuerung -->
      <div class="card">
        <h2>🎞️ Sensor & Steuerung</h2>
        <div id="sensorStatus" class="sensor-status sensor-ok">
          <div class="info-row">
            <span class="label">Status</span>
            <span id="sensorError" class="value">OK</span>
          </div>
          <div class="info-row">
            <span class="label">Filament</span>
            <span id="filamentPresent" class="value">-</span>
          </div>
          <div class="info-row">
            <span class="label">Auto-Pause</span>
            <span id="autoPause" class="value">-</span>
          </div>
        </div>
        <div style="border-top: 1px solid #333; margin: 12px 0 8px 0; padding-top: 12px; font-size: 0;">
          <button onclick="clearError()" class="secondary" style="width: 49%; margin: 0; font-size: 1rem; display: inline-block; background: #2a2a2a; padding: 8px 24px;"><span style="font-size: 1.8em;">🔄</span></button><!--
          --><span style="display: inline-block; width: 2%;"></span><!--
          --><button id="lightBtn" onclick="toggleLight()" class="light-off" style="width: 49%; margin: 0; font-size: 1rem; display: inline-block; padding: 8px 24px;"><span style="font-size: 1.8em;">💡</span></button>
          <div style="margin-top: 18px;"></div>
          <button onclick="pausePrint()" style="width: 32%; margin: 0; font-size: 1rem; display: inline-block; background: #660000; padding: 8px 24px;"><span style="font-size: 1.8em;">⏸️</span></button><!--
          --><span style="display: inline-block; width: 2%;"></span><!--
          --><button onclick="resumePrint()" class="success" style="width: 32%; margin: 0; font-size: 1rem; display: inline-block; background: #005500; padding: 8px 24px;"><span style="font-size: 1.8em;">▶️</span></button><!--
          --><span style="display: inline-block; width: 2%;"></span><!--
          --><button onclick="cancelPrint()" style="background: #660000; width: 32%; margin: 0; font-size: 1rem; display: inline-block; padding: 8px 24px;"><span style="font-size: 1.8em;">⏹️</span></button>
        </div>
      </div>

      <!-- Kamera über 2 Spalten -->
      <div class="card card-wide">
        <h2 style="display: flex; justify-content: space-between; align-items: center;">
          <span>📷 Live-Kamera</span>
          <button onclick="refreshCamera()" class="secondary" style="padding: 6px 12px; font-size: 0.9em; margin: 0;">🔄 Refresh</button>
        </h2>
        <div class="camera-container">
          <img id="cameraStream" src="" style="display: none;" onload="this.style.display='block'; document.getElementById('cameraOffline').style.display='none';" onerror="this.style.display='none'; document.getElementById('cameraOffline').style.display='flex';">
          <div id="cameraOffline" class="camera-offline" style="display: flex;">
            Kamera offline oder nicht konfiguriert
          </div>
        </div>
      </div>
    </div>
  </div>

  <!-- Settings Modal -->
  <div id="settingsModal" class="modal">
    <div class="modal-content">
      <div class="modal-header">
        <h2>⚙️ Einstellungen</h2>
        <button class="close-btn" onclick="closeSettings()">✕ Schließen</button>
      </div>

      <!-- Filament Sensor Settings -->
      <div class="settings-section">
        <h3>🎞️ Filament-Sensor</h3>
        <label>
          <input type="checkbox" id="autoPauseToggle" onchange="toggleAutoPause()">
          Auto-Pause aktivieren
        </label>
        <label>
          <input type="checkbox" id="switchModeToggle" onchange="toggleSwitchMode()">
          Direct Mode (Hardware-Pin)
        </label>
        <label>
          <span class="label">Verzögerung vor Pause (ms)</span>
          <input type="number" id="pauseDelay" min="1000" max="10000" step="500">
        </label>
        <button onclick="savePauseDelay()">💾 Verzögerung speichern</button>
      </div>

      <!-- WhatsApp Notifications -->
      <div class="settings-section">
        <h3>📱 WhatsApp-Benachrichtigungen (CallMeBot)</h3>
        <label>
          <input type="checkbox" id="notifyEnabled" onchange="toggleNotifications()">
          Benachrichtigungen aktivieren
        </label>
        <label>
          <span class="label">Telefonnummer (mit Ländercode, z.B. 491701234567)</span>
          <input type="text" id="notifyPhone" placeholder="491701234567">
        </label>
        <label>
          <span class="label">CallMeBot API Key</span>
          <input type="text" id="notifyApiKey" placeholder="API Key">
        </label>
        <button onclick="saveCallMeBotSettings()">💾 Speichern</button>
        <button onclick="testNotification()" class="success">📤 Test-Nachricht senden</button>
        <div id="notifyMessage" class="alert hidden"></div>
      </div>

      <!-- WiFi Configuration -->
      <div class="settings-section">
        <h3>📡 WiFi-Konfiguration</h3>
        <label>
          <span class="label">SSID (Router-Name)</span>
          <input type="text" id="wifiSSIDInput" placeholder="Mein-Router">
        </label>
        <label>
          <span class="label">WiFi-Passwort</span>
          <input type="password" id="wifiPasswordInput" placeholder="********">
        </label>
        <button onclick="saveWiFiConfig()">💾 Speichern & Neu starten</button>
        <div id="wifiConfigMessage" class="alert hidden"></div>
      </div>

      <!-- Printer Configuration -->
      <div class="settings-section">
        <h3>🖨️ Drucker-Konfiguration</h3>
        <label>
          <span class="label">Drucker IP-Adresse</span>
          <input type="text" id="printerIPInput" placeholder="192.168.1.67">
        </label>
        <label>
          <span class="label">Drucker Port</span>
          <input type="number" id="printerPortInput" value="80" min="1" max="65535">
        </label>
        <button onclick="savePrinterConfig()">💾 Speichern & Neu starten</button>
        <div id="printerConfigMessage" class="alert hidden"></div>
      </div>

      <!-- OTA Update - GANZ UNTEN -->
      <div class="settings-section">
        <h3>📦 OTA Firmware Update</h3>
        <p style="color: #888; margin-bottom: 10px; font-size: 0.9em;">
          Wähle eine .bin Datei aus und klicke auf Upload. Der ESP32 startet nach erfolgreichem Update automatisch neu.
        </p>
        <input type="file" id="firmwareFile" accept=".bin">
        <button onclick="uploadFirmware()" style="background: #ff6600;">📤 Firmware hochladen</button>
        <div id="uploadProgress" class="progress-bar hidden">
          <div id="uploadBar" class="progress-fill" style="width: 0%"></div>
          <div id="uploadText" class="progress-text">0%</div>
        </div>
        <div id="uploadMessage" class="alert hidden"></div>
      </div>
    </div>
  </div>

  <script>
    let printerIP = window.location.hostname;

    function updateUI(data) {
      // Status
      const statusText = data.status.stateText;
      const statusCode = data.status.state;
      const badge = document.getElementById('statusBadge');
      badge.textContent = statusText + ' (' + statusCode + ')';
      badge.className = 'status-badge';

      if (statusText.includes('PRINTING')) {
        badge.className += ' status-printing';
      } else if (statusText.includes('PAUSED')) {
        badge.className += ' status-paused';
      } else if (statusText.includes('COMPLETE')) {
        badge.className += ' status-complete';
      } else {
        badge.className += ' status-idle';
      }

      // Licht-Button Farbe ändern
      const lightBtn = document.getElementById('lightBtn');
      if (data.status.lightOn) {
        lightBtn.className = 'light-on';
      } else {
        lightBtn.className = 'light-off';
      }

      // Temperaturen
      document.getElementById('bedTemp').textContent =
        data.status.bedTemp.toFixed(1) + '°C / ' + data.status.bedTarget.toFixed(1) + '°C';
      document.getElementById('nozzleTemp').textContent =
        data.status.nozzleTemp.toFixed(1) + '°C / ' + data.status.nozzleTarget.toFixed(1) + '°C';
      document.getElementById('chamberTemp').textContent = data.status.chamberTemp.toFixed(1) + '°C';

      // Fortschritt
      const progress = data.print.progress || 0;
      document.getElementById('progressBar').style.width = progress + '%';
      document.getElementById('progressText').textContent = progress + '%';
      document.getElementById('filename').textContent = data.print.filename || '-';
      document.getElementById('layer').textContent =
        data.print.layer + ' / ' + data.print.totalLayers;
      document.getElementById('speed').textContent = data.print.speed + '%';

      // Filament Sensor
      const sensorDiv = document.getElementById('sensorStatus');
      if (data.sensor.error) {
        sensorDiv.className = 'sensor-status sensor-error';
        document.getElementById('sensorError').textContent = '⚠️ FEHLER!';
      } else {
        sensorDiv.className = 'sensor-status sensor-ok';
        document.getElementById('sensorError').textContent = '✓ OK';
      }

      document.getElementById('filamentPresent').textContent =
        data.sensor.noFilament ? '❌ FEHLT' : '✓ Vorhanden';
      document.getElementById('autoPause').textContent =
        data.sensor.autoPause ? '✓ Aktiv' : '⚫ Aus';

      // Settings - only update if settings modal is closed (to avoid overwriting user input)
      const settingsModal = document.getElementById('settingsModal');
      if (settingsModal.style.display !== 'block') {
        document.getElementById('autoPauseToggle').checked = data.sensor.autoPause;
        document.getElementById('switchModeToggle').checked = data.sensor.switchDirectMode;
        document.getElementById('pauseDelay').value = data.sensor.pauseDelay;
        document.getElementById('notifyEnabled').checked = data.notify.enabled;
        document.getElementById('notifyPhone').value = data.notify.phone;
        document.getElementById('wifiSSIDInput').value = data.wifiSSID || '';
        document.getElementById('wifiPasswordInput').value = '';  // Password bleibt leer aus Sicherheitsgründen
        document.getElementById('printerIPInput').value = data.printerIP || '';
        document.getElementById('printerPortInput').value = data.printerPort || 80;
      }

      // Note: Camera is loaded separately via loadCameraURL() function
    }

    function fetchStatus() {
      fetch('/api/status')
        .then(r => r.json())
        .then(updateUI)
        .catch(e => console.error('Status fetch error:', e));
    }

    function sendCommand(action, data = {}) {
      fetch('/api/control', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({action, ...data})
      }).then(() => fetchStatus());
    }

    function pausePrint() { sendCommand('pause'); }
    function resumePrint() { sendCommand('resume'); }
    function cancelPrint() { if(confirm('Druck wirklich abbrechen?')) sendCommand('cancel'); }
    function toggleLight() { sendCommand('toggleLight'); }
    function clearError() { sendCommand('clearError'); }
    function toggleAutoPause() { sendCommand('toggleAutoPause'); }
    function toggleSwitchMode() { sendCommand('toggleSwitchMode'); }

    // Camera functions
    async function loadCameraURL() {
      try {
        const response = await fetch('/api/config');
        const data = await response.json();
        if (data.printerIP) {
          const cameraURL = 'http://' + data.printerIP + ':3031/video';
          document.getElementById('cameraStream').src = cameraURL;
        }
      } catch (error) {
        console.error('Fehler beim Laden der Kamera-URL:', error);
      }
    }

    function reloadCamera() {
      const img = document.getElementById('cameraStream');
      const currentSrc = img.src;
      img.src = '';
      setTimeout(() => {
        img.src = currentSrc + '?t=' + new Date().getTime();
      }, 100);
    }

    function savePauseDelay() {
      const delay = document.getElementById('pauseDelay').value;
      sendCommand('setPauseDelay', {delay: parseInt(delay)});
    }

    function toggleNotifications() {
      const enabled = document.getElementById('notifyEnabled').checked;
      sendCommand('setCallMeBotSettings', {
        enabled,
        phone: document.getElementById('notifyPhone').value,
        apiKey: document.getElementById('notifyApiKey').value
      });
    }

    function saveCallMeBotSettings() {
      const msg = document.getElementById('notifyMessage');
      sendCommand('setCallMeBotSettings', {
        enabled: document.getElementById('notifyEnabled').checked,
        phone: document.getElementById('notifyPhone').value,
        apiKey: document.getElementById('notifyApiKey').value
      });
      msg.className = 'alert alert-success';
      msg.textContent = '✓ Einstellungen gespeichert';
      msg.classList.remove('hidden');
      setTimeout(() => msg.classList.add('hidden'), 3000);
    }

    function saveWiFiConfig() {
      const msg = document.getElementById('wifiConfigMessage');
      const wifiSSID = document.getElementById('wifiSSIDInput').value;
      const wifiPassword = document.getElementById('wifiPasswordInput').value;

      if (!wifiSSID) {
        msg.className = 'alert alert-error';
        msg.textContent = '❌ Bitte SSID eingeben';
        msg.classList.remove('hidden');
        setTimeout(() => msg.classList.add('hidden'), 3000);
        return;
      }

      if (!wifiPassword) {
        msg.className = 'alert alert-error';
        msg.textContent = '❌ Bitte WiFi-Passwort eingeben';
        msg.classList.remove('hidden');
        setTimeout(() => msg.classList.add('hidden'), 3000);
        return;
      }

      fetch('/api/settings', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({wifiSSID, wifiPassword})
      })
      .then(r => r.json())
      .then(data => {
        msg.className = 'alert alert-success';
        msg.textContent = '✓ Gespeichert. ESP32 startet neu...';
        msg.classList.remove('hidden');
        setTimeout(() => {
          location.reload();
        }, 3000);
      })
      .catch(e => {
        msg.className = 'alert alert-error';
        msg.textContent = '❌ Fehler beim Speichern';
        msg.classList.remove('hidden');
        setTimeout(() => msg.classList.add('hidden'), 3000);
      });
    }

    function savePrinterConfig() {
      const msg = document.getElementById('printerConfigMessage');
      const printerIP = document.getElementById('printerIPInput').value;
      const printerPort = parseInt(document.getElementById('printerPortInput').value);

      if (!printerIP) {
        msg.className = 'alert alert-error';
        msg.textContent = '❌ Bitte Drucker-IP eingeben';
        msg.classList.remove('hidden');
        setTimeout(() => msg.classList.add('hidden'), 3000);
        return;
      }

      fetch('/api/settings', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({printerIP, printerPort})
      })
      .then(r => r.json())
      .then(data => {
        msg.className = 'alert alert-success';
        msg.textContent = '✓ Gespeichert. ESP32 startet neu...';
        msg.classList.remove('hidden');
        setTimeout(() => {
          location.reload();
        }, 3000);
      })
      .catch(e => {
        msg.className = 'alert alert-error';
        msg.textContent = '❌ Fehler beim Speichern';
        msg.classList.remove('hidden');
        setTimeout(() => msg.classList.add('hidden'), 3000);
      });
    }

    function testNotification() {
      const msg = document.getElementById('notifyMessage');
      sendCommand('testNotification');
      msg.className = 'alert alert-success';
      msg.textContent = '📤 Test-Nachricht wird gesendet...';
      msg.classList.remove('hidden');
      setTimeout(() => msg.classList.add('hidden'), 5000);
    }

    function uploadFirmware() {
      const file = document.getElementById('firmwareFile').files[0];
      if (!file) {
        alert('Bitte wähle eine Firmware-Datei aus');
        return;
      }

      const formData = new FormData();
      formData.append('firmware', file);

      const progressDiv = document.getElementById('uploadProgress');
      const progressBar = document.getElementById('uploadBar');
      const progressText = document.getElementById('uploadText');
      const msg = document.getElementById('uploadMessage');

      progressDiv.classList.remove('hidden');
      msg.classList.add('hidden');

      const xhr = new XMLHttpRequest();

      xhr.upload.addEventListener('progress', (e) => {
        if (e.lengthComputable) {
          const percent = Math.round((e.loaded / e.total) * 100);
          progressBar.style.width = percent + '%';
          progressText.textContent = percent + '%';
        }
      });

      xhr.addEventListener('load', () => {
        if (xhr.status === 200) {
          msg.className = 'alert alert-success';
          msg.textContent = '✓ Upload erfolgreich! ESP32 startet neu...';
          msg.classList.remove('hidden');
          setTimeout(() => location.reload(), 10000);
        } else {
          msg.className = 'alert alert-error';
          msg.textContent = '❌ Upload fehlgeschlagen: ' + xhr.statusText;
          msg.classList.remove('hidden');
        }
        progressDiv.classList.add('hidden');
      });

      xhr.addEventListener('error', () => {
        msg.className = 'alert alert-error';
        msg.textContent = '❌ Upload fehlgeschlagen';
        msg.classList.remove('hidden');
        progressDiv.classList.add('hidden');
      });

      xhr.open('POST', '/api/ota/upload');
      xhr.send(formData);
    }

    function openSettings() {
      document.getElementById('settingsModal').style.display = 'block';
    }

    function closeSettings() {
      document.getElementById('settingsModal').style.display = 'none';
    }

    function refreshCamera() {
      reloadCamera();
    }

    // Initial fetch und Update alle 500ms
    fetchStatus();
    setInterval(fetchStatus, 500);

    // Load camera stream
    loadCameraURL();
  </script>
</body>
</html>
)rawliteral";
}

#endif
