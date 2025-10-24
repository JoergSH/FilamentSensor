/*
 * Setup Portal HTML
 * Initial configuration interface
 */

#ifndef SETUP_PORTAL_H
#define SETUP_PORTAL_H

const char* getSetupPortalHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Centauri Monitor Setup</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }

    body {
      font-family: 'Segoe UI', system-ui, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: #fff;
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }

    .container {
      background: rgba(255,255,255,0.15);
      backdrop-filter: blur(10px);
      border-radius: 20px;
      padding: 40px;
      max-width: 500px;
      width: 100%;
      box-shadow: 0 8px 32px rgba(0,0,0,0.2);
      border: 1px solid rgba(255,255,255,0.2);
    }

    h1 {
      font-size: 2em;
      margin-bottom: 10px;
      text-align: center;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
    }

    .subtitle {
      text-align: center;
      opacity: 0.9;
      margin-bottom: 30px;
      font-size: 1.1em;
    }

    .form-group {
      margin-bottom: 20px;
    }

    label {
      display: block;
      margin-bottom: 8px;
      font-weight: bold;
      font-size: 0.95em;
    }

    input {
      width: 100%;
      padding: 12px;
      border: none;
      border-radius: 10px;
      font-size: 1em;
      background: rgba(255,255,255,0.9);
      color: #333;
    }

    input:focus {
      outline: 3px solid rgba(255,255,255,0.5);
    }

    .btn {
      width: 100%;
      padding: 15px;
      border: none;
      border-radius: 10px;
      font-size: 1.1em;
      font-weight: bold;
      cursor: pointer;
      transition: all 0.3s;
      margin-top: 10px;
    }

    .btn-primary {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: #fff;
    }

    .btn-primary:hover {
      transform: translateY(-2px);
      box-shadow: 0 5px 20px rgba(0,0,0,0.3);
    }

    .btn-secondary {
      background: rgba(255,255,255,0.2);
      color: #fff;
    }

    .alert {
      padding: 15px;
      border-radius: 10px;
      margin-bottom: 20px;
      display: none;
    }

    .alert-success {
      background: rgba(40, 167, 69, 0.3);
      border: 1px solid rgba(40, 167, 69, 0.5);
    }

    .alert-error {
      background: rgba(220, 53, 69, 0.3);
      border: 1px solid rgba(220, 53, 69, 0.5);
    }

    .info-box {
      background: rgba(255,255,255,0.1);
      padding: 15px;
      border-radius: 10px;
      margin-bottom: 20px;
      font-size: 0.9em;
    }

    .section {
      margin-bottom: 30px;
    }

    .section-title {
      font-size: 1.3em;
      margin-bottom: 15px;
      padding-bottom: 10px;
      border-bottom: 2px solid rgba(255,255,255,0.3);
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🖨️ Centauri Monitor</h1>
    <p class="subtitle">Initial Setup</p>

    <div class="alert alert-success" id="successAlert">
      ✅ Konfiguration gespeichert! System startet neu...
    </div>

    <div class="alert alert-error" id="errorAlert">
      ❌ Fehler beim Speichern!
    </div>

    <div class="info-box">
      ℹ️ Bitte geben Sie Ihre WiFi-Zugangsdaten und die IP-Adresse Ihres Druckers ein.
    </div>

    <form id="setupForm">
      <div class="section">
        <div class="section-title">📶 WiFi Einstellungen</div>

        <div class="form-group">
          <label for="ssid">Netzwerkname (SSID)</label>
          <input type="text" id="ssid" name="ssid" required placeholder="Ihr WiFi Netzwerk">
        </div>

        <div class="form-group">
          <label for="password">Passwort</label>
          <input type="password" id="password" name="password" required placeholder="WiFi Passwort">
        </div>
      </div>

      <div class="section">
        <div class="section-title">🖨️ Drucker Einstellungen</div>

        <div class="form-group">
          <label for="printerIp">Drucker IP-Adresse</label>
          <input type="text" id="printerIp" name="printerIp" required
                 placeholder="192.168.1.100" pattern="^(\d{1,3}\.){3}\d{1,3}$">
        </div>

        <div class="form-group">
          <label for="printerPort">Drucker Port</label>
          <input type="number" id="printerPort" name="printerPort" required
                 value="80" min="1" max="65535">
        </div>
      </div>

      <button type="submit" class="btn btn-primary">💾 Speichern & Verbinden</button>
    </form>
  </div>

  <script>
    document.getElementById('setupForm').addEventListener('submit', async (e) => {
      e.preventDefault();

      const data = {
        ssid: document.getElementById('ssid').value,
        password: document.getElementById('password').value,
        printerIp: document.getElementById('printerIp').value,
        printerPort: parseInt(document.getElementById('printerPort').value)
      };

      try {
        const response = await fetch('/api/setup', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(data)
        });

        const result = await response.json();

        if (result.success) {
          document.getElementById('successAlert').style.display = 'block';
          document.getElementById('setupForm').style.display = 'none';

          setTimeout(() => {
            window.location.href = '/';
          }, 3000);
        } else {
          document.getElementById('errorAlert').textContent = '❌ ' + result.message;
          document.getElementById('errorAlert').style.display = 'block';
        }
      } catch (error) {
        document.getElementById('errorAlert').textContent = '❌ Verbindungsfehler!';
        document.getElementById('errorAlert').style.display = 'block';
      }
    });
  </script>
</body>
</html>
  )rawliteral";
}

#endif // SETUP_PORTAL_H
