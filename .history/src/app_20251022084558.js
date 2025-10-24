// app.js - Main Application

class PrinterDashboard {
    constructor() {
        this.printerState = {
            printStatus: 0,
            bedTemp: 0,
            nozzleTemp: 0,
            chamberTemp: 0,
            bedTargetTemp: 0,
            nozzleTargetTemp: 0,
            currentCoord: '',
            zOffset: 0,
            modelFan: 0,
            auxFan: 0,
            boxFan: 0,
            filename: '',
            currentLayer: 0,
            totalLayers: 0,
            progress: 0,
            printSpeed: 100,
            sensor: {
                present: false,
                lastMotion: 0,
                pulses: 0,
                switchRaw: 0
            }
        };
    }

    init() {
        console.log('🚀 Dashboard wird initialisiert...');
        
        // Setup WebSocket Events
        this.setupWebSocketEvents();
        
        // Setup Button Events
        this.setupButtonEvents();
        
        // Connect to ESP32
        printerWS.connect();
        
        console.log('✓ Dashboard bereit');
    }

    setupWebSocketEvents() {
        // Connection Events
        printerWS.on('connected', () => {
            console.log('WebSocket verbunden');
            UI.updateConnectionStatus(true);
        });

        printerWS.on('disconnected', () => {
            console.log('WebSocket getrennt');
            UI.updateConnectionStatus(false);
        });

        printerWS.on('error', (error) => {
            console.error('WebSocket Error:', error);
        });

        // Data Events
        printerWS.on('status', (data) => {
            this.handleStatusUpdate(data);
        });

        printerWS.on('sensor', (data) => {
            this.handleSensorUpdate(data);
        });

        printerWS.on('position', (data) => {
            this.handlePositionUpdate(data);
        });

        printerWS.on('alert', (data) => {
            this.handleAlert(data);
        });

        printerWS.on('fullStatus', (status) => {
            this.handleFullStatus(status);
        });
    }

    setupButtonEvents() {
        document.getElementById('pauseBtn').addEventListener('click', () => {
            console.log('Pause gedrückt');
            printerWS.pausePrint();
        });

        document.getElementById('resumeBtn').addEventListener('click', () => {
            console.log('Resume gedrückt');
            printerWS.resumePrint();
        });

        document.getElementById('cancelBtn').addEventListener('click', () => {
            if (confirm('Druck wirklich abbrechen?')) {
                console.log('Cancel gedrückt');
                printerWS.cancelPrint();
            }
        });

        document.getElementById('lightBtn').addEventListener('click', () => {
            console.log('Light Toggle gedrückt');
            printerWS.toggleLight();
        });
    }

    handleStatusUpdate(data) {
        // Update internal state
        Object.assign(this.printerState, data);
        
        // Update UI
        UI.updateFullStatus(this.printerState);
    }

    handleSensorUpdate(data) {
        this.printerState.sensor = data;
        UI.updateFilamentSensor(data);
    }

    handlePositionUpdate(data) {
        this.printerState.currentCoord = data.coord;
        UI.updatePosition(data.coord);
    }

    handleAlert(data) {
        console.warn('⚠️ Alert:', data.message);
        
        if (data.type === 'filament_runout') {
            UI.showFilamentAlert('Filament aufgebraucht!');
        } else if (data.type === 'filament_jam') {
            UI.showFilamentAlert('Filament Stau erkannt!');
        } else {
            UI.showFilamentAlert(data.message);
        }
    }

    handleFullStatus(status) {
        this.printerState = { ...this.printerState, ...status };
        UI.updateFullStatus(this.printerState);
    }
}

// Start Application when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    const dashboard = new PrinterDashboard();
    dashboard.init();
});