// ui.js - UI Update Functions

const UI = {
    // Connection Status
    updateConnectionStatus(connected) {
        const statusEl = document.getElementById('connectionStatus');
        const dot = statusEl.querySelector('.status-dot');
        const text = statusEl.querySelector('.status-text');
        
        if (connected) {
            dot.classList.add('connected');
            dot.classList.remove('disconnected');
            text.textContent = 'Verbunden';
        } else {
            dot.classList.remove('connected');
            dot.classList.add('disconnected');
            text.textContent = 'Getrennt';
        }
    },

    // Print Status
    updatePrintStatus(status) {
        const badge = document.getElementById('printStatus');
        const statusText = CONFIG.STATUS_CODES[status] || 'UNBEKANNT';
        
        badge.textContent = statusText;
        badge.className = 'status-badge';
        
        if (status === 13) badge.classList.add('printing');
        else if (status === 10) badge.classList.add('paused');
        else if (status === 0) badge.classList.add('idle');
        else badge.classList.add('error');
        
        // Update buttons
        this.updateButtons(status);
    },

    // Button States
    updateButtons(printStatus) {
        const pauseBtn = document.getElementById('pauseBtn');
        const resumeBtn = document.getElementById('resumeBtn');
        const cancelBtn = document.getElementById('cancelBtn');
        
        const isPrinting = printStatus === 13;
        const isPaused = printStatus === 10;
        
        pauseBtn.disabled = !isPrinting;
        resumeBtn.disabled = !isPaused;
        cancelBtn.disabled = !(isPrinting || isPaused);
    },

    // Print Info
    updatePrintInfo(data) {
        document.getElementById('filename').textContent = data.filename || '-';
        document.getElementById('layerInfo').textContent = `${data.currentLayer} / ${data.totalLayers}`;
        document.getElementById('printSpeed').textContent = `${data.printSpeed}%`;
        
        // Progress bar
        const progress = data.progress || 0;
        document.getElementById('progressBar').style.width = `${progress}%`;
        document.getElementById('progressText').textContent = `${progress}%`;
    },

    // Temperatures
    updateTemperatures(data) {
        document.getElementById('bedTemp').textContent = data.bedTemp.toFixed(1);
        document.getElementById('bedTarget').textContent = data.bedTargetTemp.toFixed(1);
        document.getElementById('nozzleTemp').textContent = data.nozzleTemp.toFixed(1);
        document.getElementById('nozzleTarget').textContent = data.nozzleTargetTemp.toFixed(1);
        document.getElementById('chamberTemp').textContent = data.chamberTemp.toFixed(1);
    },

    // Position
    updatePosition(coord) {
        if (!coord) return;
        
        // Parse "X,Y,Z" format
        const parts = coord.split(',');
        if (parts.length === 3) {
            document.getElementById('posX').textContent = parseFloat(parts[0]).toFixed(2);
            document.getElementById('posY').textContent = parseFloat(parts[1]).toFixed(2);
            document.getElementById('posZ').textContent = parseFloat(parts[2]).toFixed(2);
        }
    },

    updateZOffset(offset) {
        document.getElementById('zOffset').textContent = `${offset.toFixed(2)} mm`;
    },

    // Fan Speeds
    updateFans(data) {
        this.updateFan('modelFan', data.modelFan);
        this.updateFan('auxFan', data.auxFan);
        this.updateFan('boxFan', data.boxFan);
    },