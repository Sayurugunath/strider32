/**
 * Strider32 Simulator Application Controller (v0.3 QA Certified)
 * Master controller connecting UI DOM, GaitSimulator, RobotModel, Gamepad, & RobotApi.
 * Author: Sayuru Gunathilaka
 */

const SimulatorApp = {
    mode: 'SIMULATION', // 'SIMULATION' or 'REAL'
    activeTab: 'tab-motion',
    lastFrameTime: performance.now(),

    init() {
        // Initialize 3D Viewport
        const container = document.getElementById('webglCanvas');
        if (container && typeof RobotModel !== 'undefined') {
            RobotModel.initScene(container);
        }

        if (typeof GaitSimulator !== 'undefined') {
            GaitSimulator.init();
        }

        // Initialize Gamepad Handler
        if (typeof GamepadController !== 'undefined') {
            GamepadController.init(
                (gpId) => {
                    const statusEl = document.getElementById('gpStatus');
                    if (statusEl) {
                        statusEl.textContent = `Connected: ${gpId}`;
                        statusEl.style.color = 'var(--emerald)';
                    }
                },
                () => {
                    const statusEl = document.getElementById('gpStatus');
                    if (statusEl) {
                        statusEl.textContent = 'No Gamepad Connected';
                        statusEl.style.color = 'var(--amber)';
                    }
                }
            );
        }

        // Setup Event Listeners
        this.bindEvents();

        // Start Main Render & Simulation Loop
        requestAnimationFrame((now) => this.mainLoop(now));
    },

    bindEvents() {
        // Mode Switcher
        document.getElementById('modeSimBtn')?.addEventListener('click', () => this.switchMode('SIMULATION'));
        document.getElementById('modeRealBtn')?.addEventListener('click', () => this.switchMode('REAL'));

        // Tab Navigation
        document.querySelectorAll('.tab-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                const targetTab = e.target.getAttribute('data-tab');
                this.switchTab(targetTab);
            });
        });

        // E-STOP Triggers
        document.getElementById('estopBtn')?.addEventListener('click', () => this.triggerEstop());
        document.getElementById('bannerResetBtn')?.addEventListener('click', () => this.resetEstop());

        // Manual Joint Sliders (J0 - J7)
        for (let i = 0; i < 8; i++) {
            const slider = document.getElementById(`s${i}`);
            if (slider) {
                slider.addEventListener('input', () => this.updateManualJoints());
            }
        }

        // Cartesian Inputs
        ['cxVal', 'czVal'].forEach(id => {
            const el = document.getElementById(id);
            if (el) {
                el.addEventListener('input', () => this.updateCartesianInput());
            }
        });
    },

    updateManualJoints() {
        const angles = [];
        for (let i = 0; i < 8; i++) {
            const slider = document.getElementById(`s${i}`);
            const val = slider ? parseInt(slider.value) : 90;
            const label = document.getElementById(`jv${i}`);
            if (label) label.textContent = `${val}°`;
            angles.push(val);
        }
        if (typeof RobotModel !== 'undefined') {
            RobotModel.setJointAngles(angles);
        }
    },

    updateCartesianInput() {
        const legSelect = document.getElementById('cartLegSelect');
        const legIdx = legSelect ? parseInt(legSelect.value) : 0;
        const xVal = parseInt(document.getElementById('cxInput')?.value || 60);
        const zVal = parseInt(document.getElementById('czInput')?.value || -40);

        if (typeof RobotKinematics !== 'undefined') {
            const ik = RobotKinematics.solveLegIK(xVal, 0, zVal);
            const currentAngles = RobotModel.getJointState();
            currentAngles[legIdx * 2] = ik.coxa;
            currentAngles[legIdx * 2 + 1] = ik.femur;
            RobotModel.setJointAngles(currentAngles);

            const labelX = document.getElementById('cxVal');
            const labelZ = document.getElementById('czVal');
            if (labelX) labelX.textContent = xVal;
            if (labelZ) labelZ.textContent = zVal;
        }
    },

    switchMode(newMode) {
        if (newMode === 'REAL') {
            const confirmMsg = "SECURITY NOTICE: Entering Real Robot Mode will transmit REST API commands over Wi-Fi. Ensure physical robot clearance!";
            if (!confirm(confirmMsg)) return;
        }

        this.mode = newMode;
        document.getElementById('modeSimBtn').classList.toggle('active', newMode === 'SIMULATION');
        document.getElementById('modeRealBtn').classList.toggle('active', newMode === 'REAL');

        const modeBadge = document.getElementById('modeBadge');
        if (modeBadge) {
            modeBadge.textContent = newMode === 'SIMULATION' ? 'SIMULATION MODE (OFFLINE)' : 'REAL ROBOT MODE (LIVE)';
            modeBadge.className = newMode === 'SIMULATION' ? 'badge badge-cyan' : 'badge badge-rose';
        }
    },

    switchTab(tabId) {
        this.activeTab = tabId;
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.toggle('active', b.getAttribute('data-tab') === tabId));
        document.querySelectorAll('.tab-content').forEach(c => c.classList.toggle('active', c.id === tabId));
    },

    triggerEstop() {
        if (typeof GaitSimulator !== 'undefined') GaitSimulator.pause();
        if (this.mode === 'REAL' && typeof RobotApi !== 'undefined') RobotApi.triggerEstop();

        document.getElementById('estopBanner')?.classList.remove('hidden');
    },

    resetEstop() {
        if (this.mode === 'REAL' && typeof RobotApi !== 'undefined') RobotApi.resetEstop();
        document.getElementById('estopBanner')?.classList.add('hidden');
    },

    mainLoop(now) {
        requestAnimationFrame((n) => this.mainLoop(n));

        const dt = (now - this.lastFrameTime) / 1000.0;
        this.lastFrameTime = now;

        // 1. Poll Gamepad Input
        if (typeof GamepadController !== 'undefined' && GamepadController.connected) {
            const gp = GamepadController.poll();
            if (gp) {
                if (gp.btnEstop) this.triggerEstop();
                if (this.mode === 'SIMULATION' && typeof GaitSimulator !== 'undefined') {
                    GaitSimulator.setMotionCommand(gp.forward, gp.lateral, gp.turn);
                    if (!GaitSimulator.state.isPlaying && (gp.forward !== 0 || gp.lateral !== 0 || gp.turn !== 0)) {
                        GaitSimulator.play();
                    }
                }
            }
        }

        // 2. Advance Simulation Frame
        if (this.mode === 'SIMULATION' && typeof GaitSimulator !== 'undefined') {
            if (GaitSimulator.state.isPlaying) {
                GaitSimulator.stepFrame(dt);
            }
        }

        // 3. Update Telemetry UI
        this.updateTelemetryUI();
    },

    updateTelemetryUI() {
        if (typeof GaitSimulator === 'undefined') return;
        const telem = GaitSimulator.getTelemetry();

        const pEl = document.getElementById('telemPhase');
        if (pEl) pEl.textContent = `${telem.phasePercent}%`;

        const gEl = document.getElementById('telemGait');
        if (gEl) gEl.textContent = telem.activeGait;

        const grid = document.getElementById('telemetryGrid');
        if (grid) {
            let html = '';
            ['FL', 'FR', 'BL', 'BR'].forEach(k => {
                const leg = telem.legs[k];
                html += `
                    <div class="telemetry-item">
                        <strong>${k}:</strong> ${leg.state}<br>
                        XYZ: (${leg.target.x.toFixed(0)}, ${leg.target.y.toFixed(0)}, ${leg.target.z.toFixed(0)})<br>
                        Angles: (${leg.angles.coxa.toFixed(0)}°, ${leg.angles.femur.toFixed(0)}°)
                    </div>
                `;
            });
            grid.innerHTML = html;
        }
    }
};

document.addEventListener('DOMContentLoaded', () => {
    SimulatorApp.init();
});
