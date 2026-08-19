/**
 * Strider32 Browser Kinematics Engine (v0.3)
 * JavaScript implementation of 3D Inverse & Forward Kinematics
 * Synchronized with firmware/src/motion/Kinematics.cpp
 * Author: Sayuru Gunathilaka
 */

const RobotKinematics = {
    // Structural Link Dimensions (in millimeters)
    L1_COXA_MM: 30.0,  // Coxa hip offset link length
    L2_FEMUR_MM: 60.0, // Femur thigh link length

    // Joint Angle Servo Limits (in degrees)
    MIN_ANGLE_DEG: 10.0,
    MAX_ANGLE_DEG: 170.0,

    /**
     * Helper utility to clamp numbers within [min, max]
     */
    clamp(val, min, max) {
        return Math.max(min, Math.min(max, val));
    },

    /**
     * Analytical 3D Inverse Kinematics Solver
     * Converts target foot Cartesian position (x, y, z) in mm relative to shoulder origin
     * into joint angles (coxa, femur).
     * 
     * @param {number} x - Forward foot coordinate (+X forward)
     * @param {number} y - Lateral foot coordinate (+Y outward)
     * @param {number} z - Vertical foot coordinate (+Z upward, z < 0 for ground)
     * @returns {Object} { coxa: number, femur: number, withinReach: boolean, D: number, alpha: number, beta: number }
     */
    solveLegIK(x, y, z) {
        let withinReach = true;

        // 1. Calculate Coxa (Hip) Angle in XY horizontal plane
        // Neutral x=60, y=0 -> Math.atan2(0, 60) = 0 rad -> 90.0 degrees
        const coxaRad = Math.atan2(y, x);
        let coxaDeg = (coxaRad * 180.0 / Math.PI) + 90.0;

        // 2. Planar horizontal radius from hip pivot
        const r = Math.sqrt(x * x + y * y);
        let rEff = r - this.L1_COXA_MM;
        if (rEff < 1.0) rEff = 1.0; // Singularity protection at origin

        // 3. 3D Extension distance D to foot tip
        let D = Math.sqrt(rEff * rEff + z * z);
        const maxReach = 2.0 * this.L2_FEMUR_MM;

        // Workspace Reach Protection: Clamp D if target exceeds physical reach
        if (D > maxReach) {
            D = maxReach - 0.1;
            withinReach = false;
        }

        // 4. Calculate Femur (Elevation) Angle using law of cosines
        const alpha = Math.atan2(z, rEff);
        let cosBeta = D / (2.0 * this.L2_FEMUR_MM);
        cosBeta = this.clamp(cosBeta, -1.0, 1.0);
        const beta = Math.acos(cosBeta);

        const femurRad = alpha + beta;
        let femurDeg = (femurRad * 180.0 / Math.PI);

        // 5. Constrain joint angles within safe servo limits (10° - 170°)
        coxaDeg = this.clamp(coxaDeg, this.MIN_ANGLE_DEG, this.MAX_ANGLE_DEG);
        femurDeg = this.clamp(femurDeg, this.MIN_ANGLE_DEG, this.MAX_ANGLE_DEG);

        return {
            coxa: coxaDeg,
            femur: femurDeg,
            withinReach: withinReach,
            D: D,
            alpha: alpha,
            beta: beta
        };
    },

    /**
     * Analytical Forward Kinematics Solver
     * Reconstructs 3D Cartesian coordinates (x, y, z) in mm from joint angles
     * 
     * @param {number} coxa - Coxa joint angle in degrees
     * @param {number} femur - Femur joint angle in degrees
     * @param {number} [alpha] - Optional elevation angle in radians (if passed from IK)
     * @param {number} [beta] - Optional knee angle in radians (if passed from IK)
     * @param {number} [D] - Optional 3D extension distance in mm
     * @returns {Object} { x: number, y: number, z: number }
     */
    solveLegFK(coxa, femur, alpha = null, beta = null, D = null) {
        const coxaRad = (coxa - 90.0) * Math.PI / 180.0;

        let rEff, z;
        if (alpha !== null && D !== null) {
            rEff = D * Math.cos(alpha);
            z = D * Math.sin(alpha);
        } else {
            const femurRad = femur * Math.PI / 180.0;
            D = 2.0 * this.L2_FEMUR_MM * Math.cos(femurRad * 0.5);
            if (D < 10.0) D = 10.0;
            rEff = D * Math.cos(femurRad * 0.5);
            z = -D * Math.sin(femurRad * 0.5);
        }

        const r = rEff + this.L1_COXA_MM;
        const x = r * Math.cos(coxaRad);
        const y = r * Math.sin(coxaRad);

        return { x, y, z };
    },

    /**
     * Applies leg coordinate transformation / mirroring
     * FL & BL use (+X, +Y, Z), FR & BR use (+X, -Y, Z)
     */
    getLegTargetCoordinates(legIndex, forward, lateral, vertical) {
        // 0=FL, 1=FR, 2=BL, 3=BR
        const isRightSide = (legIndex === 1 || legIndex === 3);
        return {
            x: forward,
            y: isRightSide ? -lateral : lateral,
            z: vertical
        };
    }
};

if (typeof module !== 'undefined' && module.exports) {
    module.exports = RobotKinematics;
}
