/**
 * Strider32 HUD Control Center Application JavaScript
 * Native Vanilla ES6+ Web Interface
 * Author: Sayuru Gunathilaka
 */

document.addEventListener('DOMContentLoaded', () => {
    App.init();
});

const App = {
    pollInterval: null,
    isTabVisible: true,
    currentState: {
        safetyState: 0, // 0=READY, 1=RUNNING, 2=ESTOPPED, 3=FAULT
        estopActive: false,
        servos: [],
        currentAnim: {
            id: "wave_hello",
            frames: [
                { angles: [90, 90, 45, 150, 90, 90, 90, 90], duration: 400 },
                { angles: [90, 90, 60, 120, 90, 90, 90, 90], duration: 400 }
            ]
        }
    },

    init() {
        this.bindEvents();
        this.initTabs();
        this.startStatusPolling();
        this.fetchStatus();
        this.fetchServos();
        this.fetchAnimations();
    },

    bindEvents() {
        // E-STOP Button
        document.getElementById('estopBtn').addEventListener('click', () => this.triggerEstop());
        document.getElementById('bannerResetBtn').addEventListener('click', () => this.resetEstop());

        // Visibility Change Listener (Pause polling when tab is inactive)
        document.addEventListener('visibilitychange', () => {
            this.isTabVisible = !document.hidden;
            if (this.isTabVisible) {
                this.fetchStatus();
            }
        });
    },

    initTabs() {
        const navBtns = document.querySelectorAll('.nav-btn');
        navBtns.forEach(btn => {
            btn.addEventListener('click', () => {
                const targetTab = btn.getAttribute('data-tab');
                
                navBtns.forEach(b => b.classList.remove('active'));
                btn.classList.add('active');

                document.querySelectorAll('.tab-pane').forEach(pane => {
                    pane.classList.remove('active');
                });
                document.getElementById(`tab-${targetTab}`).classList.add('active');

                // Tab Specific Data Loads
                if (targetTab === 'diagnostics') this.fetchDiagnostics();
                if (targetTab === 'calibration') this.fetchServos();
                if (targetTab === 'animation') this.fetchAnimations();
                if (targetTab === 'network') this.fetchNetwork();
            });
        });
    },

    startStatusPolling() {
        this.pollInterval = setInterval(() => {
            if (this.isTabVisible) {
                this.fetchStatus();
            }
        }, 2000);
    },

    // ========================================================================
    // 1. STATUS & SAFETY TELEMETRY
    // ========================================================================
    async fetchStatus() {
        try {
            const res = await fetch('/api/v1/status');
            if (!res.ok) throw new Error('Network status error');
            const json = await res.json();
            if (json.success) {
                this.updateStatusUI(json.data);
            }
        } catch (err) {
            this.updateOfflineUI();
        }
    },

    updateStatusUI(data) {
        const statusDot = document.getElementById('statusDot');
        const headerIp = document.getElementById('headerIp');
        const dashState = document.getElementById('dashState');
        const dashStateSub = document.getElementById('dashStateSub');
        const dashRssi = document.getElementById('dashRssi');
        const dashWifiMode = document.getElementById('dashWifiMode');
        const dashUptime = document.getElementById('dashUptime');
        const dashHeap = document.getElementById('dashHeap');

        headerIp.textContent = data.wifi.ip || '192.168.4.1';
        dashRssi.textContent = `${data.wifi.rssi || 0} dBm`;
        dashWifiMode.textContent = `${data.wifi.mode || 'AP'} Mode`;
        
        // Format Uptime HH:MM:SS
        const sec = data.uptime_seconds || 0;
        const hrs = Math.floor(sec / 3600).toString().padStart(2, '0');
        const mins = Math.floor((sec % 3600) / 60).toString().padStart(2, '0');
        const secs = (sec % 60).toString().padStart(2, '0');
        dashUptime.textContent = `${hrs}:${mins}:${secs}`;

        dashHeap.textContent = `${Math.round((data.free_heap_bytes || 0) / 1024)} KB`;

        this.currentState.estopActive = data.estop_active;
        this.currentState.safetyState = data.safety_state;

        // Safety State Badge & E-STOP Banner
        const estopBanner = document.getElementById('estopBanner');
        const motionBtns = document.querySelectorAll('.motion-req');

        if (data.estop_active || data.safety_state === 2) {
            statusDot.className = 'status-dot estoppe';
            dashState.textContent = 'ESTOPPED';
            dashState.className = 'metric-value status-estop';
            dashStateSub.textContent = 'Servo Outputs Disabled';

            estopBanner.classList.remove('hidden');
            motionBtns.forEach(btn => btn.disabled = true);
        } else {
            statusDot.className = 'status-dot online';
            dashState.textContent = data.safety_state === 1 ? 'RUNNING' : 'READY';
            dashState.className = 'metric-value status-online';
            dashStateSub.textContent = 'System Operational';

            estopBanner.classList.add('hidden');
            motionBtns.forEach(btn => btn.disabled = false);
        }
    },

    updateOfflineUI() {
        document.getElementById('statusDot').className = 'status-dot offline';
        document.getElementById('dashState').textContent = 'OFFLINE';
        document.getElementById('dashState').className = 'metric-value';
        document.getElementById('dashStateSub').textContent = 'Connecting...';
    },

    // ========================================================================
    // 2. E-STOP ACTIONS
    // ========================================================================
    async triggerEstop() {
        try {
            const res = await fetch('/api/v1/estop', { method: 'POST' });
            const json = await res.json();
            this.fetchStatus();
        } catch (err) {
            alert('Failed to send E-STOP command!');
        }
    },

    async resetEstop() {
        try {
            const res = await fetch('/api/v1/estop/reset', { method: 'POST' });
            const json = await res.json();
            if (json.success) {
                this.fetchStatus();
            } else {
                alert(json.error || 'Reset rejected');
            }
        } catch (err) {
            alert('Failed to send E-STOP reset request!');
        }
    },

    // ========================================================================
    // 3. MOTION CONTROL API
    // ========================================================================
    async sendMotion(action, forward = 0, lateral = 0, turn = 0) {
        if (this.currentState.estopActive) return;
        try {
            await fetch('/api/v1/control', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ action, forward, lateral, turn })
            });
        } catch (err) {
            console.error('Motion control error', err);
        }
    },

    async triggerPose(poseName) {
        if (this.currentState.estopActive) return;
        try {
            await fetch('/api/v1/control', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ action: 'walk', forward: 0, lateral: 0, turn: 0 })
            });
        } catch (err) {
            console.error('Pose trigger error', err);
        }
    },

    async changeGait(gaitVal) {
        try {
            await fetch('/api/v1/control', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ action: 'gait', gait: parseInt(gaitVal) })
            });
        } catch (err) {}
    },

    async changeSpeed(speedVal) {
        document.getElementById('speedVal').textContent = `${speedVal}%`;
        try {
            await fetch('/api/v1/control', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ action: 'speed', speed: parseInt(speedVal) })
            });
        } catch (err) {}
    },

    // ========================================================================
    // 4. SERVO CALIBRATION STUDIO API
    // ========================================================================
    async fetchServos() {
        try {
            const res = await fetch('/api/v1/servos');
            const json = await res.json();
            if (json.success) {
                this.renderServoGrid(json.data);
            }
        } catch (err) {}
    },

    renderServoGrid(data) {
        const grid = document.getElementById('servoCalibGrid');
        grid.innerHTML = '';

        const names = [
            'FL Coxa (Hip)', 'FL Femur (Knee)',
            'FR Coxa (Hip)', 'FR Femur (Knee)',
            'BL Coxa (Hip)', 'BL Femur (Knee)',
            'BR Coxa (Hip)', 'BR Femur (Knee)'
        ];

        for (let i = 0; i < 8; i++) {
            const angle = data.angles ? data.angles[i] : 90;
            const offset = data.offsets ? data.offsets[i] : 0;

            const card = document.createElement('div');
            card.className = 'servo-card';
            card.innerHTML = `
                <div class="servo-card-head">
                    <span class="servo-name">J${i}: ${names[i]}</span>
                    <span class="servo-angle" id="angVal_${i}">${angle}°</span>
                </div>
                <div class="form-group">
                    <label>Subtrim Offset: <span id="offVal_${i}">${offset > 0 ? '+' + offset : offset}°</span></label>
                    <input type="range" min="-30" max="30" value="${offset}" 
                           oninput="App.updateServoOffset(${i}, this.value)">
                </div>
            `;
            grid.appendChild(card);
        }
    },

    updateServoOffset(index, val) {
        document.getElementById(`offVal_${index}`).textContent = val > 0 ? `+${val}°` : `${val}°`;
        if (!this.currentState.servos[index]) this.currentState.servos[index] = {};
        this.currentState.servos[index].offset = parseInt(val);
    },

    async saveCalibration() {
        const offsets = [];
        for (let i = 0; i < 8; i++) {
            const el = document.getElementById(`offVal_${i}`);
            offsets.push(el ? parseInt(el.textContent) : 0);
        }
        try {
            const res = await fetch('/api/v1/servos/calibrate', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ offsets, save: true })
            });
            const json = await res.json();
            alert(json.message || 'Calibration saved');
        } catch (err) {
            alert('Failed to save calibration!');
        }
    },

    // ========================================================================
    // 5. TIMELINE ANIMATION STUDIO API
    // ========================================================================
    async fetchAnimations() {
        try {
            const res = await fetch('/api/v1/animations');
            const json = await res.json();
            if (json.success) {
                this.renderAnimationList(json.animations || []);
            }
        } catch (err) {}
    },

    renderAnimationList(anims) {
        const list = document.getElementById('animFileList');
        list.innerHTML = '';
        if (anims.length === 0) {
            list.innerHTML = '<div class="anim-item"><span class="text-muted">No saved animations</span></div>';
            return;
        }

        anims.forEach(anim => {
            const item = document.createElement('div');
            item.className = 'anim-item';
            item.innerHTML = `
                <span><strong>${anim.id}</strong> (${anim.size_bytes}B)</span>
                <div class="btn-group-sm">
                    <button class="btn btn-cyan btn-sm" onclick="App.loadAnimation('${anim.id}')">Load</button>
                    <button class="btn btn-danger btn-sm" onclick="App.deleteAnimation('${anim.id}')">Del</button>
                </div>
            `;
            list.appendChild(item);
        });

        this.renderTimelineFrames();
    },

    renderTimelineFrames() {
        const container = document.getElementById('timelineFrames');
        container.innerHTML = '';

        this.currentState.currentAnim.frames.forEach((f, idx) => {
            const card = document.createElement('div');
            card.className = 'keyframe-card';
            card.innerHTML = `
                <div class="servo-card-head">
                    <span>Frame #${idx + 1}</span>
                    <button class="btn btn-danger btn-sm" onclick="App.removeKeyframe(${idx})">×</button>
                </div>
                <div class="form-group">
                    <label>Duration (ms):</label>
                    <input type="number" class="form-control" value="${f.duration}" min="50" max="5000" 
                           onchange="App.currentState.currentAnim.frames[${idx}].duration = parseInt(this.value)">
                </div>
                <small class="text-muted">8 Joint Angles Set</small>
            `;
            container.appendChild(card);
        });
    },

    addKeyframe() {
        if (this.currentState.currentAnim.frames.length >= 16) {
            alert('Maximum 16 keyframes allowed');
            return;
        }
        this.currentState.currentAnim.frames.push({
            angles: [90, 90, 90, 90, 90, 90, 90, 90],
            duration: 300
        });
        this.renderTimelineFrames();
    },

    removeKeyframe(idx) {
        if (this.currentState.currentAnim.frames.length <= 1) return;
        this.currentState.currentAnim.frames.splice(idx, 1);
        this.renderTimelineFrames();
    },

    createNewAnimation() {
        const id = prompt('Enter new animation ID name:', 'custom_anim');
        if (!id) return;
        this.currentState.currentAnim = {
            id: id,
            frames: [{ angles: [90, 90, 90, 90, 90, 90, 90, 90], duration: 300 }]
        };
        document.getElementById('animNameInput').value = id;
        this.renderTimelineFrames();
    },

    async saveCurrentAnimation() {
        const animId = document.getElementById('animNameInput').value.trim();
        if (!animId) {
            alert('Please specify an animation ID name');
            return;
        }
        this.currentState.currentAnim.id = animId;

        try {
            const res = await fetch('/api/v1/animations', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(this.currentState.currentAnim)
            });
            const json = await res.json();
            if (json.success) {
                alert('Animation sequence saved to LittleFS!');
                this.fetchAnimations();
            } else {
                alert(json.error || 'Failed to save animation');
            }
        } catch (err) {
            alert('Error connecting to backend!');
        }
    },

    async loadAnimation(animId) {
        try {
            const res = await fetch(`/api/v1/animations/${animId}`);
            const json = await res.json();
            this.currentState.currentAnim = json;
            document.getElementById('animNameInput').value = animId;
            this.renderTimelineFrames();
        } catch (err) {
            alert('Failed to load animation file');
        }
    },

    async deleteAnimation(animId) {
        if (!confirm(`Delete animation file "${animId}"?`)) return;
        try {
            await fetch(`/api/v1/animations/${animId}`, { method: 'DELETE' });
            this.fetchAnimations();
        } catch (err) {}
    },

    async playCurrentAnimation() {
        const animId = document.getElementById('animNameInput').value.trim();
        try {
            const res = await fetch(`/api/v1/animations/${animId}/play`, { method: 'POST' });
            const json = await res.json();
            if (!json.success) alert(json.error || 'Playback failed');
        } catch (err) {}
    },

    // ========================================================================
    // 6. DIAGNOSTICS TELEMETRY API
    // ========================================================================
    async fetchDiagnostics() {
        try {
            const res = await fetch('/api/v1/diagnostics');
            const json = await res.json();
            if (json.success) {
                this.renderDiagnosticsTable(json.data);
            }
        } catch (err) {}
    },

    renderDiagnosticsTable(data) {
        const tbody = document.getElementById('diagTableBody');
        tbody.innerHTML = '';

        const rows = [
            { label: 'System Uptime', value: `${data.uptime_seconds} sec`, status: 'OK' },
            { label: 'Free Heap SRAM', value: `${Math.round(data.free_heap_bytes / 1024)} KB`, status: data.free_heap_bytes > 50000 ? 'OK' : 'WARN' },
            { label: 'Min Free Heap', value: `${Math.round(data.min_free_heap_bytes / 1024)} KB`, status: 'OK' },
            { label: 'CPU Frequency', value: `${data.cpu_freq_mhz} MHz`, status: 'OK' },
            { label: 'Chip Revision', value: `ESP32 Rev ${data.chip_revision}`, status: 'OK' },
            { label: 'Firmware Version', value: data.firmware_version, status: 'OK' },
            { label: 'Wi-Fi Mode / IP', value: `${data.wifi_mode} (${data.ip_address})`, status: 'OK' },
            { label: 'Wi-Fi RSSI Signal', value: `${data.rssi_dbm} dBm`, status: 'OK' },
            { label: 'Safety State', value: data.estop_active ? 'ESTOPPED' : 'READY', status: data.estop_active ? 'CRITICAL' : 'OK' },
            { label: 'Servo Driver Backend', value: data.servo_backend === 0 ? 'Direct ESP32 GPIO LEDC' : 'PCA9685 I2C', status: 'OK' },
            { label: 'Hardware Attached', value: data.hardware_attached ? 'YES' : 'NO', status: 'OK' },
            { label: 'Last System Error', value: data.last_error || 'None', status: 'OK' }
        ];

        rows.forEach(r => {
            const tr = document.createElement('tr');
            tr.innerHTML = `
                <td>${r.label}</td>
                <td>${r.value}</td>
                <td><span class="badge ${r.status === 'OK' ? 'status-online' : 'status-estop'}">${r.status}</span></td>
            `;
            tbody.appendChild(tr);
        });
    },

    // ========================================================================
    // 7. NETWORK CONFIGURATION API
    // ========================================================================
    async fetchNetwork() {
        try {
            const res = await fetch('/api/v1/network');
            const json = await res.json();
            if (json.success) {
                document.getElementById('netMode').value = json.data.mode;
                document.getElementById('netStaSsid').value = json.data.sta_ssid || '';
                document.getElementById('netHostname').value = json.data.hostname || 'strider32';
            }
        } catch (err) {}
    },

    async scanNetworks() {
        const box = document.getElementById('scanResultsBox');
        box.classList.remove('hidden');
        box.innerHTML = '<small class="text-muted">Scanning Wi-Fi networks...</small>';

        try {
            const res = await fetch('/api/v1/network/scan');
            const json = await res.json();
            box.innerHTML = '';
            if (json.networks && json.networks.length > 0) {
                json.networks.forEach(net => {
                    const item = document.createElement('div');
                    item.className = 'anim-item';
                    item.innerHTML = `<span><strong>${net.ssid}</strong> (${net.rssi} dBm)</span>`;
                    item.onclick = () => {
                        document.getElementById('netStaSsid').value = net.ssid;
                        box.classList.add('hidden');
                    };
                    box.appendChild(item);
                });
            } else {
                box.innerHTML = '<small class="text-muted">No networks found</small>';
            }
        } catch (err) {
            box.innerHTML = '<small class="text-muted">Scan failed</small>';
        }
    },

    async saveNetworkSettings(e) {
        e.preventDefault();
        const mode = document.getElementById('netMode').value;
        const sta_ssid = document.getElementById('netStaSsid').value.trim();
        const sta_password = document.getElementById('netStaPass').value;
        const hostname = document.getElementById('netHostname').value.trim();

        try {
            const res = await fetch('/api/v1/network', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ mode, sta_ssid, sta_password, hostname })
            });
            const json = await res.json();
            alert(json.message || 'Network settings saved to LittleFS!');
        } catch (err) {
            alert('Failed to save network settings!');
        }
    }
};
