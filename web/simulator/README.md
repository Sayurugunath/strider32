# Strider32 Browser 3D Robot Simulator HUD (v0.3.0)

Welcome to the **Strider32 3D Robot Simulator**, an open-source, browser-native 3D robotics simulation environment for the Strider32 ESP32 quadruped platform.

---

## 🚀 Quick Start (100% Offline Access)

Launch the simulator directly in any modern desktop or mobile web browser (Chrome, Firefox, Edge, Safari):

* **Simulator Application URL:**  
  `file:///E:/Antigravity%20projects/sesame%20robot/web/simulator/index.html`

No local HTTP web server, Node.js runtime, or internet connection is required. All 3D rendering engines (`Three.js r128`) and control scripts are vendored locally.

---

## 🌟 Key Features

1. **Procedural 3D Robot Viewport (`RobotModel.js`):** Interactive WebGL 3D robot model with 360° OrbitControls, ground grid reference, and debug axes toggle.
2. **Analytical 3D IK/FK Engine (`RobotKinematics.js`):** $O(1)$ constant time analytical solver with workspace reach protection ($D \le 119.9\text{mm}$) and 0.0000 mm numerical error.
3. **Cartesian Gait Trajectory Engine (`GaitSimulator.js`):** TROT (2-Pair Diagonal) and CRAWL (4-Phase Static) gaits with continuous foot trajectories.
4. **Timeline Animation Studio (`AnimationController.js`):** Record keyframe pose sequences, play back timelines, and export/import JSON animation files.
5. **Browser Gamepad API Integration (`GamepadController.js`):** Drive the virtual quadruped using any USB/Bluetooth game controller (DualShock, Xbox, etc.).
6. **Dual-Mode Architecture & Safety (`RobotApi.js`):** Seamlessly toggle between offline `[SIMULATION MODE]` and live `[REAL ROBOT MODE]` with E-STOP guards.

---

## 🧪 Test Suites

* **[`tests/ik_test.html`](tests/ik_test.html):** Phase 1 Analytical IK/FK Mathematical Parity Suite (`0.0000 mm` error).
* **[`tests/model_test.html`](tests/model_test.html):** Phase 2 3D Model & Slider Harness.
* **[`tests/gait_test.html`](tests/gait_test.html):** Phase 3 3D Gait Simulation Harness.
