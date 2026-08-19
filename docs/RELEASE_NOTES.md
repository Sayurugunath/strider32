# Strider32 Release Notes

## Version 0.2.0 — 3D Inverse Kinematics (August 2026)

**Project Lead:** Sayuru Gunathilaka  
**License:** Apache 2.0  

---

### 🚀 Highlights & Features Included

1. **Analytical 3D Inverse Kinematics Solver (`Kinematics::solveLegIK`):**
   * Analytical 3D IK converting Cartesian foot coordinates $(x, y, z)$ to joint angles $(\theta_{\text{coxa}}, \theta_{\text{femur}})$.
   * Trigonometric law-of-cosines formulation in $O(1)$ constant time.
2. **Forward Kinematics Reconstruction (`Kinematics::solveLegFK`):**
   * Converts joint angle outputs back into foot tip Cartesian coordinates for validation and posture alignment.
3. **Workspace Boundary Protection & Clamping:**
   * Automatically clamps 3D extension vector $D$ to physical maximum reach $2 \cdot L_2 - 0.1\text{mm} = 119.9\text{mm}$ if target point is unreachable.
   * Numerical stability protection prevents `NaN`, `Inf`, or `acos` out-of-bounds math errors.
4. **Cartesian Gait Trajectory Engine (`GaitEngine`):**
   * Replaced open-loop empirical sine waves with parametric 3D Cartesian foot trajectories for both **TROT** and **CRAWL** gaits.
   * Continuous foot trajectories across phase boundaries ($0.00\text{mm}$ position discontinuity).
5. **Leg Mirroring & Coordinate Conventions:**
   * $+X$ (Forward), $+Y$ (Lateral Outward), $+Z$ (Upward).
   * Explicit 4-leg mapping for Front-Left, Front-Right, Back-Left, and Back-Right legs.
6. **Automated IK Validation Harness (`Kinematics::runSelfTest`):**
   * Executes automated mathematical self-tests during boot to verify roundtrip accuracy, workspace clamping, and joint limits.

---

### ⚠️ Hardware Validation Disclaimer

> [!NOTE]
> **Hardware Status:** Software verified; physical quadruped walking has not yet been hardware validated.

---

## Version 0.1.0 — Initial Release (August 2026)

* Initial release of direct ESP32 GPIO PWM servo driver, latched E-STOP safety system, REST API suite, and embedded Vanilla JS Web Control Center.
