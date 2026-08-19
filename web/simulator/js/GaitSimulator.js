/**
 * Strider32 Browser Gait & Trajectory Simulation Engine (v0.3 Phase 3)
 * Generates continuous 3D Cartesian foot trajectories for TROT & CRAWL gaits,
 * solves joint angles via RobotKinematics.js, and updates RobotModel.js.
 * Author: Sayuru Gunathilaka
 */

const GaitSimulator = {
    // Simulation Parameters & State
    state: {
        activeGait: 'TROT',      // 'TROT' or 'CRAWL'
        isPlaying: false,
        phase: 0.0,              // 0.0 to 1.0
        gaitFrequency: 1.5,      // Hz
        speedMultiplier: 1.0,
        
        // Motion Velocity Commands (-1.0 to 1.0)
        cmdForward: 0.0,
        cmdLateral: 0.0,
        cmdTurn: 0.0,

        // Physical Step Parameters (mm)
        baseX: 60.0,
        baseZ: -40.0,
        stepLength: 20.0,
        stepWidth: 12.0,
        stepHeight: 18.0,
        turnStride: 12.0
    },

    // Leg Phase States (0=Stance, 1=Swing)
    legStates: {
        FL: { state: 'STANCE', phase: 0.0, target: { x: 60, y: 0, z: -40 }, angles: { coxa: 90, femur: 90 } },
        FR: { state: 'STANCE', phase: 0.0, target: { x: 60, y: 0, z: -40 }, angles: { coxa: 90, femur: 90 } },
        BL: { state: 'STANCE', phase: 0.0, target: { x: 60, y: 0, z: -40 }, angles: { coxa: 90, femur: 90 } },
        BR: { state: 'STANCE', phase: 0.0, target: { x: 60, y: 0, z: -40 }, angles: { coxa: 90, femur: 90 } }
    },

    lastTimeMs: 0,

    /**
     * Initialize Simulation State
     */
    init() {
        this.reset();
    },

    play() {
        this.state.isPlaying = true;
        this.lastTimeMs = performance.now();
    },

    pause() {
        this.state.isPlaying = false;
    },

    togglePlay() {
        if (this.state.isPlaying) this.pause();
        else this.play();
    },

    reset() {
        this.state.phase = 0.0;
        this.state.cmdForward = 0.0;
        this.state.cmdLateral = 0.0;
        this.state.cmdTurn = 0.0;
        this.updateTrajectory(0.0);
        if (typeof RobotModel !== 'undefined' && RobotModel.robotGroup) {
            const ik = RobotKinematics.solveLegIK(this.state.baseX, 0, this.state.baseZ);
            RobotModel.setJointAngles([ik.coxa, ik.femur, ik.coxa, ik.femur, ik.coxa, ik.femur, ik.coxa, ik.femur]);
        }
    },

    setGait(gaitName) {
        if (gaitName === 'TROT' || gaitName === 'CRAWL') {
            this.state.activeGait = gaitName;
            this.state.phase = 0.0;
        }
    },

    setMotionCommand(forward, lateral, turn) {
        this.state.cmdForward = Math.max(-1.0, Math.min(1.0, forward));
        this.state.cmdLateral = Math.max(-1.0, Math.min(1.0, lateral));
        this.state.cmdTurn    = Math.max(-1.0, Math.min(1.0, turn));
    },

    /**
     * Main Simulation Frame Advance (Delta Time dt in seconds)
     */
    stepFrame(dt) {
        if (dt <= 0) return;

        const isMoving = Math.abs(this.state.cmdForward) > 0.05 || 
                         Math.abs(this.state.cmdLateral) > 0.05 || 
                         Math.abs(this.state.cmdTurn) > 0.05;

        if (isMoving && this.state.isPlaying) {
            const freq = this.state.gaitFrequency * this.state.speedMultiplier;
            this.state.phase += freq * dt;
            if (this.state.phase >= 1.0) this.state.phase -= 1.0;
        }

        this.updateTrajectory(dt);
    },

    /**
     * Evaluates Parametric 3D Cartesian Trajectories & Solves IK
     */
    updateTrajectory(dt) {
        if (typeof RobotKinematics === 'undefined') return;

        if (this.state.activeGait === 'TROT') {
            this.updateTrotTrajectory();
        } else if (this.state.activeGait === 'CRAWL') {
            this.updateCrawlTrajectory();
        }

        // Apply calculated joint angles to Three.js RobotModel
        if (typeof RobotModel !== 'undefined' && RobotModel.robotGroup) {
            const angles = [
                this.legStates.FL.angles.coxa, this.legStates.FL.angles.femur,
                this.legStates.FR.angles.coxa, this.legStates.FR.angles.femur,
                this.legStates.BL.angles.coxa, this.legStates.BL.angles.femur,
                this.legStates.BR.angles.coxa, this.legStates.BR.angles.femur
            ];
            RobotModel.setJointAngles(angles);
        }
    },

    /**
     * TROT Gait Trajectory Generator (2-Pair Diagonal Alternating)
     */
    updateTrotTrajectory() {
        const sinPhaseA = Math.sin(this.state.phase * 2.0 * Math.PI);
        const sinPhaseB = Math.sin((this.state.phase + 0.5) * 2.0 * Math.PI);

        const fwdStride  = this.state.stepLength * this.state.cmdForward;
        const latStride  = this.state.stepWidth * this.state.cmdLateral;
        const turnStride = this.state.turnStride * this.state.cmdTurn;
        const liftHeight = this.state.stepHeight;

        // PAIR A: FL & BR
        const xA = this.state.baseX + (sinPhaseA * fwdStride);
        const yA_FL = (sinPhaseA * latStride) + turnStride;
        const yA_BR = (-sinPhaseA * latStride) + turnStride;
        const zA = this.state.baseZ + (sinPhaseA > 0 ? sinPhaseA * liftHeight : 0.0);

        // PAIR B: FR & BL
        const xB = this.state.baseX + (sinPhaseB * fwdStride);
        const yB_FR = (-sinPhaseB * latStride) - turnStride;
        const yB_BL = (sinPhaseB * latStride) - turnStride;
        const zB = this.state.baseZ + (sinPhaseB > 0 ? sinPhaseB * liftHeight : 0.0);

        // Solve IK for each leg
        this.solveAndStoreLeg('FL', xA, yA_FL, zA, sinPhaseA > 0);
        this.solveAndStoreLeg('BR', xA, yA_BR, zA, sinPhaseA > 0);
        this.solveAndStoreLeg('FR', xB, yB_FR, zB, sinPhaseB > 0);
        this.solveAndStoreLeg('BL', xB, yB_BL, zB, sinPhaseB > 0);
    },

    /**
     * CRAWL Gait Trajectory Generator (4-Phase Static Stability)
     * Phase 0: FL swing, Phase 1: BR swing, Phase 2: FR swing, Phase 3: BL swing
     */
    updateCrawlTrajectory() {
        const legPhase = this.state.phase * 4.0;
        const activeSwingLegIdx = Math.floor(legPhase) % 4;
        const subPhase = legPhase - Math.floor(legPhase); // 0.0 to 1.0

        const fwdStride  = this.state.stepLength * 0.9 * this.state.cmdForward;
        const latStride  = this.state.stepWidth * 0.8 * this.state.cmdLateral;
        const turnStride = this.state.turnStride * 0.8 * this.state.cmdTurn;
        const liftHeight = this.state.stepHeight;

        const swingOrder = ['FL', 'BR', 'FR', 'BL'];
        const activeLegKey = swingOrder[activeSwingLegIdx];

        ['FL', 'FR', 'BL', 'BR'].forEach(legKey => {
            let x = this.state.baseX;
            let y = 0.0;
            let z = this.state.baseZ;
            let isSwing = (legKey === activeLegKey);

            if (isSwing) {
                // SWING PHASE: Lift along parabolic arch
                z += Math.sin(subPhase * Math.PI) * liftHeight;
                x += (subPhase - 0.5) * 2.0 * fwdStride;
            } else {
                // STANCE PHASE: Propel body backward
                x += -0.33 * (subPhase - 0.5) * 2.0 * fwdStride;
            }

            // Apply lateral & rotation offsets
            if (legKey === 'FL' || legKey === 'BL') y += latStride;
            else y -= latStride;

            if (legKey === 'FL' || legKey === 'BR') y += turnStride;
            else y -= turnStride;

            this.solveAndStoreLeg(legKey, x, y, z, isSwing);
        });
    },

    solveAndStoreLeg(legKey, x, y, z, isSwing) {
        const ik = RobotKinematics.solveLegIK(x, y, z);
        this.legStates[legKey] = {
            state: isSwing ? 'SWING' : 'STANCE',
            phase: this.state.phase,
            target: { x, y, z },
            angles: { coxa: ik.coxa, femur: ik.femur }
        };
    },

    /**
     * Get Current Simulation Telemetry Object
     */
    getTelemetry() {
        return {
            activeGait: this.state.activeGait,
            isPlaying: this.state.isPlaying,
            phasePercent: Math.round(this.state.phase * 100),
            cmdForward: this.state.cmdForward,
            cmdLateral: this.state.cmdLateral,
            cmdTurn: this.state.cmdTurn,
            legs: this.legStates
        };
    }
};

if (typeof module !== 'undefined' && module.exports) {
    module.exports = GaitSimulator;
}
