// config.js - Konfiguration

const CONFIG = {
    // ESP32 WebSocket Server
    ESP32_IP: '192.168.1.150',  // ← HIER DEINE ESP32 IP EINTRAGEN
    ESP32_PORT: 81,              // WebSocket Port (muss im ESP32 Code angepasst werden)
    
    // Update Intervals
    UI_UPDATE_INTERVAL: 100,     // UI Update alle 100ms
    
    // Filament Sensor Thresholds
    MOTION_TIMEOUT: 3000,        // 3 Sekunden ohne Bewegung = Warnung
    
    // Status Codes
    STATUS_CODES: {
        0: 'IDLE',
        8: 'VORBEREITUNG',
        9: 'START',
        10: 'PAUSIERT',
        13: 'DRUCKT'
    }
};

// Export für andere Module
if (typeof module !== 'undefined' && module.exports) {
    module.exports = CONFIG;
}