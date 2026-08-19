# Strider32 Release Notes

## Version 0.3.0 — Browser-Based 3D Robot Simulator (August 2026)

**Project Lead:** Sayuru Gunathilaka  
**License:** Apache 2.0  

---

### 🚀 Highlights & Features Included

1. **Browser-Native 3D Robot Simulator HUD (`web/simulator/`):**
   * Complete, standalone WebGL 3D robot simulator interface for visualization, testing, and control.
   * 100% standalone offline operation with zero runtime CDN dependencies.
2. **Three.js Procedural Scenegraph Renderer (`RobotModel.js`):**
   * Interactive 3D robot model with 360° OrbitControls, ground grid reference, and debug axes.
   * Procedural mesh geometry matching Strider32 hardware parameters ($L_1 = 30\text{mm}, L_2 = 60\text{mm}$).
3. **Analytical 3D IK/FK Engine (`RobotKinematics.js`):**
   * Pure JavaScript implementation of `solveLegIK` and `solveLegFK` synchronized with firmware reference vectors ($0.0000\text{mm}$ numerical error).
   * Includes workspace reach protection ($D \le 119.9\text{mm}$), origin protection ($r_{\text{eff}} \ge 1.0\text{mm}$), and joint limits ($10^\circ \le \theta \le 170^\circ$).
4. **Cartesian Gait Trajectory Simulation Engine (`GaitSimulator.js`):**
   * Real-time 3D simulation of **TROT** (2-Pair Diagonal) and **CRAWL** (4-Phase Static FL $\rightarrow$ BR $\rightarrow$ FR $\rightarrow$ BL) gaits with continuous foot trajectories.
   * Supports forward, backward, lateral strafe, and yaw rotation steering.
5. **Timeline Animation Studio (`AnimationController.js`):**
   * Record keyframe pose sequences, play back timelines, and export/import JSON animation files.
6. **Browser Gamepad API Integration (`GamepadController.js`):**
   * Drive the virtual quadruped using any USB/Bluetooth game controller (DualShock, Xbox, etc.).
7. **Dual-Mode Safety Architecture (`RobotApi.js`):**
   * Seamlessly toggle between offline `[SIMULATION MODE]` and live `[REAL ROBOT MODE]` with E-STOP guards and explicit user confirmation before hardware network calls.

---

### ⚠️ Hardware Validation Disclaimer

> [!NOTE]
> **Hardware Status:** Software verified; physical quadruped walking has not yet been hardware validated.

---

## Version 0.2.0 — 3D Inverse Kinematics (August 2026)

* Analytical 3D Inverse Kinematics solver (`Kinematics::solveLegIK`), Forward Kinematics (`Kinematics::solveLegFK`), workspace protection, and Cartesian trajectories.

---

## Version 0.1.0 — Initial Release (August 2026)

* Initial release of direct ESP32 GPIO PWM servo driver, latched E-STOP safety system, REST API suite, and embedded Vanilla JS Web Control Center.
