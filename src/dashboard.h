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
      background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
      color: #fff;
      min-height: 100vh;
      padding: 20px;
    }
    
    .container {
      max-width: 1200px;
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
    
    .subtitle {
      opacity: 0.9;
      font-size: 1.1em;
    }
    
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px;
      margin-bottom: 20px;
    }
    
    .card {
      background: rgba(255,255,255,0.15);
      backdrop-filter: blur(10px);
      border-radius: 15px;
      padding: 20px;
      box-shadow: 0 8px 32px rgba(0,0,0,0.1);
      border: 1px solid rgba(255,255,255,0.2);
    }
    
    .card h2 {
      font-size: 1.3em;
      margin-bottom: 15px;
      padding-bottom: 10px;
      border-bottom: 2px solid rgba(255,255,255,0.3);
    }
    
    .status-badge {
      display: inline-block;
      padding: 8px 16px;
      border-radius: 20px;
      font-weight: bold;
      font-size: 1.1em;
      margin: 10px 0;
    }
    
    .status-idle { background: #6c757d; }
    .status-printing { background: #28a745; animation: pulse 2s infinite; }
    .status-paused { background: #ffc107; color: #000; }
    .status-error { background: #dc3545; animation: flash 1s infinite; }
    
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.7; }
    }
    
    @keyframes flash {
      0%, 50%, 100% { opacity: 1; }
      25%, 75% { opacity: 0.5; }
    }
    
    .temp-display {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 12px;
      margin: 10px 0;
      background: rgba(0,0,0,0.2);
      border-radius: 10px;
    }
    
    .temp-label {
      font-weight: bold;
      font-size: 0.9em;
      opacity: 0.8;
    }
    
    .temp-value {
      font-size: 1.8em;
      font-weight: bold;
    }
    
    .temp-target {
      font-size: 1em;
      opacity: 0.7;
    }
    
    .progress-bar {
      width: 100%;
      height: 30px;
      background: rgba(0,0,0,0.3);
      border-radius: 15px;
      overflow: hidden;
      margin: 15px 0;
      position: relative;
    }
    
    .progress-fill {
      height: 100%;
      background: linear-gradient(90deg, #4CAF50, #8BC34A);
      transition: width 0.3s ease;
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: bold;
    }
    
    .info-row {
      display: flex;
      justify-content: space-between;
      padding: 8px 0;
      border-bottom: 1px solid rgba(255,255,255,0.1);
    }
    
    .info-label {
      opacity: 0.8;
    }
    
    .info-value {
      font-weight: bold;
    }
    
    .btn {
      padding: 12px 24px;
      border: none;
      border-radius: 8px;
      font-size: 1em;
      font-weight: bold;
      cursor: pointer;
      transition: all 0.3s;
      margin: 5px;
      color: #fff;
    }
    
    .btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 5px 15px rgba(0,0,0,0.3);
    }
    
    .btn:active {
      transform: translateY(0);
    }
    
    .btn-pause { background: #ffc107; color: #000; }
    .btn-resume { background: #28a745; }
    .btn-cancel { background: #dc3545; }
    .btn-light { background: #17a2b8; }
    .btn-toggle { background: #6610f2; }
    
    .btn:disabled {
      opacity: 0.5;
      cursor: not-allowed;
    }
    
    .controls {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 10px;
      margin-top: 15px;
    }
    
    .sensor-settings {
      background: rgba(255,255,255,0.1);
      padding: 15px;
      margin: 15px 0;
      border-radius: 10px;
    }
    
    .delay-slider {
      width: 100%;
      margin: 10px 0;
    }
    
    .delay-value {
      font-size: 1.2em;
      font-weight: bold;
      color: #4CAF50;
    }
    
    .sensor-status {
      padding: 15px;
      margin: 10px 0;
      border-radius: 10px;
      border-left: 4px solid;
      font-size: 1.1em;
      font-weight: bold;
    }
    
    .sensor-ok {
      background: rgba(40, 167, 69, 0.2);
      border-color: #28a745;
    }
    
    .sensor-warning {
      background: rgba(255, 193, 7, 0.2);
      border-color: #ffc107;
      color: #ffc107;
    }
    
    .sensor-error {
      background: rgba(220, 53, 69, 0.2);
      border-color: #dc3545;
      animation: flash 1s infinite;
    }
    
    .update-time {
      text-align: center;
      opacity: 0.6;
      font-size: 0.9em;
      margin-top: 20px;
    }
    
    @media (max-width: 768px) {
      h1 { font-size: 1.8em; }
      .grid { grid-template-columns: 1fr; }
      .btn { width: 100%; margin: 5px 0; }
    }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <h1>🖨️ Centauri Carbon Monitor</h1>
      <p class="subtitle">Echtzeit-Überwachung & Steuerung</p>
    </header>
    
    <div class="grid">
      <!-- Status Card -->
      <div class="card">
        <h2>📊 Status</h2>
        <div class="status-badge" id="statusBadge">Lädt...</div>
        <div class="info-row">
          <span class="info-label">Position:</span>
          <span class="info-value" id="position">-</span>
        </div>
        <div class="info-row">
          <span class="info-label">Z-Offset:</span>
          <span class="info-value" id="zOffset">-</span>
        </div>
        <div class="info-row">
          <span class="info-label">Licht:</span>
          <span class="info-value" id="light">-</span>
        </div>
      </div>
      
      <!-- Temperature Card -->
      <div class="card">
        <h2>🌡️ Temperaturen</h2>
        <div class="temp-display">
          <div>
            <div class="temp-label">🛏️ BETT</div>
            <div class="temp-value" id="bedTemp">-</div>
            <div class="temp-target" id="bedTarget">Ziel: -</div>
          </div>
        </div>
        <div class="temp-display">
          <div>
            <div class="temp-label">🔥 DÜSE</div>
            <div class="temp-value" id="nozzleTemp">-</div>
            <div class="temp-target" id="nozzleTarget">Ziel: -</div>
          </div>
        </div>
        <div class="temp-display">
          <div>
            <div class="temp-label">📦 KAMMER</div>
            <div class="temp-value" id="chamberTemp">-</div>
          </div>
        </div>
      </div>
      
      <!-- Print Progress Card -->
      <div class="card">
        <h2>📈 Druckfortschritt</h2>
        <div class="progress-bar">
          <div class="progress-fill" id="progressBar" style="width: 0%">0%</div>
        </div>
        <div class="info-row">
          <span class="info-label">Datei:</span>
          <span class="info-value" id="filename">-</span>
        </div>
        <div class="info-row">
          <span class="info-label">Layer:</span>
          <span class="info-value" id="layers">-</span>
        </div>
        <div class="info-row">
          <span class="info-label">Geschwindigkeit:</span>
          <span class="info-value" id="speed">-</span>
        </div>
      </div>
      
      <!-- Filament Sensor Card -->
      <div class="card">
        <h2>🎞️ Filament Sensor</h2>
        <div id="sensorStatus" class="sensor-status sensor-ok">
          ✅ Filament OK
        </div>
        <div class="info-row">
          <span class="info-label">Letzte Bewegung:</span>
          <span class="info-value" id="lastMotion">-</span>
        </div>
        <div class="info-row">
          <span class="info-label">Impulse:</span>
          <span class="info-value" id="pulseCount">-</span>
        </div>
        <div class="info-row">
          <span class="info-label">Auto-Pause:</span>
          <span class="info-value" id="autoPause">-</span>
        </div>
      </div>
      
      <!-- Fans Card -->
      <div class="card">
        <h2>💨 Lüfter</h2>
        <div class="info-row">
          <span class="info-label">Modell:</span>
          <span class="info-value" id="fanModel">-</span>
        </div>
        <div class="info-row">
          <span class="info-label">Auxiliary:</span>
          <span class="info-value" id="fanAux">-</span>
        </div>
        <div class="info-row">
          <span class="info-label">Box:</span>
          <span class="info-value" id="fanBox">-</span>
        </div>
      </div>
      
      <!-- Control Card -->
      <div class="card">
        <h2>🎮 Steuerung</h2>
        <div class="controls">
          <button class="btn btn-pause" onclick="sendControl('pause')">⏸️ Pause</button>
          <button class="btn btn-resume" onclick="sendControl('resume')">▶️ Resume</button>
          <button class="btn btn-cancel" onclick="confirmCancel()">⏹️ Abbrechen</button>
          <button class="btn btn-light" onclick="sendControl('toggleLight')">💡 Licht</button>
        </div>
      </div>
    </div>
    
    <!-- Filament Sensor Settings -->
    <div class="sensor-settings">
      <h3>⚙️ Filament-Sensor Einstellungen</h3>
      <div class="controls">
        <button class="btn btn-toggle" onclick="toggleAutoPause()" id="autoPauseBtn">
          Auto-Pause: EIN
        </button>
        <button class="btn btn-resume" onclick="clearError()" id="clearErrorBtn">
          Fehler zurücksetzen
        </button>
      </div>
      
      <div style="margin-top: 15px;">
        <label>Verzögerung vor Pause: <span id="delayValue" class="delay-value">2000ms</span></label>
        <input type="range" min="500" max="10000" value="2000" step="100" 
               class="delay-slider" id="pauseDelay" 
               oninput="updateDelayValue(this.value)"
               onchange="setPauseDelay(this.value)">
      </div>
    </div>
    
    <div class="update-time" id="updateTime">Warte auf Daten...</div>
  </div>
  
  <script>
    let updateInterval;
    
    async function fetchStatus() {
      try {
        const response = await fetch('/api/status');
        const data = await response.json();
        updateUI(data);
        document.getElementById('updateTime').textContent = 
          'Letzte Aktualisierung: ' + new Date().toLocaleTimeString('de-DE');
      } catch (error) {
        console.error('Fehler beim Abrufen:', error);
      }
    }
    
    // Filament sensor controls
    async function toggleAutoPause() {
      await sendControl('toggleAutoPause');
    }
    
    async function clearError() {
      await sendControl('clearError');
    }
    
    function updateDelayValue(value) {
      document.getElementById('delayValue').textContent = value + 'ms';
    }
    
    let delayTimeout;
    function setPauseDelay(value) {
      clearTimeout(delayTimeout);
      delayTimeout = setTimeout(() => {
        sendControl('setPauseDelay', { delay: parseInt(value) });
      }, 500);
    }
    
    function updateUI(data) {
      // Status
      const statusText = data.status.stateText;
      const badge = document.getElementById('statusBadge');
      badge.textContent = statusText;
      badge.className = 'status-badge';
      
      if (data.status.state === 11) {  // PRINTING
        badge.className += ' status-printing';
      } else if (data.status.state === 6) {  // PAUSED
        badge.className += ' status-paused';
      } else {
        badge.className += ' status-idle';
      }
      
      // Position und Z-Offset
      document.getElementById('position').textContent = data.status.position || '-';
      document.getElementById('zOffset').textContent = 
        data.status.zOffset ? data.status.zOffset.toFixed(2) + ' mm' : '-';
      
      document.getElementById('light').textContent = data.status.lightOn ? '💡 AN' : '⚫ AUS';
      
      // Filament Sensor Status - VERBESSERT
      const sensorStatusDiv = document.getElementById('sensorStatus');
      const autoPauseBtn = document.getElementById('autoPauseBtn');
      const clearErrorBtn = document.getElementById('clearErrorBtn');
      
      // Auto-Pause Button Update
      autoPauseBtn.textContent = 'Auto-Pause: ' + (data.sensor.autoPause ? 'EIN' : 'AUS');
      
      // Pause delay slider update
      const delaySlider = document.getElementById('pauseDelay');
      if (delaySlider.value != data.sensor.pauseDelay) {
        delaySlider.value = data.sensor.pauseDelay;
        updateDelayValue(data.sensor.pauseDelay);
      }
      
      // Sensor Status Display - KORRIGIERT
      if (data.sensor.noFilament) {
        sensorStatusDiv.textContent = '⚠️ Kein Filament erkannt';
        sensorStatusDiv.className = 'sensor-status sensor-error';
        clearErrorBtn.disabled = false;
      } else if (data.sensor.error) {
        sensorStatusDiv.textContent = '⚠️ Filament-Stau erkannt';
        sensorStatusDiv.className = 'sensor-status sensor-error';
        clearErrorBtn.disabled = false;
      } else {
        sensorStatusDiv.textContent = '✅ Filament OK';
        sensorStatusDiv.className = 'sensor-status sensor-ok';
        clearErrorBtn.disabled = true;
      }
      
      // Sensor Info Rows
      document.getElementById('lastMotion').textContent = 
        Math.floor(data.sensor.lastMotion / 1000) + ' s';
      document.getElementById('pulseCount').textContent = data.sensor.pulseCount;
      document.getElementById('autoPause').textContent = 
        data.sensor.autoPause ? '✅ Aktiv' : '❌ Inaktiv';
      
      // Temperatures
      document.getElementById('bedTemp').textContent = data.status.bedTemp.toFixed(1) + '°C';
      document.getElementById('bedTarget').textContent = 'Ziel: ' + data.status.bedTarget.toFixed(1) + '°C';
      document.getElementById('nozzleTemp').textContent = data.status.nozzleTemp.toFixed(1) + '°C';
      document.getElementById('nozzleTarget').textContent = 'Ziel: ' + data.status.nozzleTarget.toFixed(1) + '°C';
      document.getElementById('chamberTemp').textContent = data.status.chamberTemp.toFixed(1) + '°C';
      
      // Print Progress
      const progress = data.print.progress;
      document.getElementById('progressBar').style.width = progress + '%';
      document.getElementById('progressBar').textContent = progress + '%';
      document.getElementById('filename').textContent = data.print.filename || '-';
      document.getElementById('layers').textContent = 
        data.print.layer + ' / ' + data.print.totalLayers;
      document.getElementById('speed').textContent = data.print.speed + '%';
      
      // Fans
      document.getElementById('fanModel').textContent = data.fans.model + '%';
      document.getElementById('fanAux').textContent = data.fans.aux + '%';
      document.getElementById('fanBox').textContent = data.fans.box + '%';
    }
    
    async function sendControl(action, data = {}) {
      try {
        const response = await fetch('/api/control', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ action: action, ...data })
        });
        const result = await response.json();
        if (result.success) {
          console.log(result.message);
          setTimeout(fetchStatus, 500);
        }
      } catch (error) {
        console.error('Fehler:', error);
      }
    }
    
    function confirmCancel() {
      if (confirm('Druck wirklich abbrechen?')) {
        sendControl('cancel');
      }
    }
    
    // Start updates
    fetchStatus();
    updateInterval = setInterval(fetchStatus, 500);
  </script>
</body>
</html>
  )rawliteral";
}

#endif