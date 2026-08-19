# Strider32 Release Notes

## Version 0.1.0 — Initial Release (August 2026)

**Project Lead:** Sayuru Gunathilaka  
**License:** Apache 2.0  

---

### 🚀 Highlights & Features Included

1. **Direct ESP32 GPIO PWM Servo Driver (`ESP32DirectServoDriver`):**
   * Drives 8 micro-servos directly using the ESP32 LEDC PWM peripheral (50 Hz, 14-bit timer resolution).
   * Smooth non-blocking soft-start ramping ($1.5^\circ / 10\text{ms}$).
   * Configurable pulse bounds (`600 µs` to `2400 µs`, $10^\circ$ to $170^\circ$ safe angle limits).
2. **Hardware Abstraction Layer (`IServoDriver`):**
   * Clean interface supporting both primary direct ESP32 GPIO PWM and secondary optional PCA9685 16-channel I2C modules.
3. **Latched E-STOP Safety System (`SafetySystem`):**
   * Hardware E-STOP button monitoring on GPIO 34 (input-only GPI pin with external 10kΩ pull-up to 3.3V).
   * Instantly disables PWM outputs upon press.
   * Manages latched states (`READY`, `RUNNING`, `EMERGENCY_STOP`, `FAULT`).
   * Explicit API reset (`POST /api/v1/estop/reset`) verifies physical button release before returning to `READY`.
4. **Motion Engine & Gaits (`GaitEngine`):**
   * Genuine 4-leg static stability crawl gait (FL $\rightarrow$ BR $\rightarrow$ FR $\rightarrow$ BL).
   * Dynamic 2-pair trot gait.
   * Planar lateral strafing (left/right) and yaw rotation turning (left/right).
   * Pre-programmed stance poses (`STAND`, `REST`, `SIT`, `WAVE`, `DANCE`).
5. **RESTful API Suite (`ApiRoutes`):**
   * Endpoints for status, diagnostics telemetry, motion control, servo calibration, network configuration, and animation storage.
   * Wi-Fi passwords safely masked as `***` in GET responses.
6. **Embedded Web Control Center (HUD):**
   * Responsive dark cyber-industrial UI built with browser-native HTML5, CSS3, and Vanilla ES6+ JS.
   * Served directly from ESP32 LittleFS flash storage (`1.92 MB`).
   * 2000 ms status polling with Page Visibility API pause/resume integration.
   * Prominent E-STOP alert banner automatically disables motion controls when estopped.
7. **Timeline Animation Studio:**
   * Create, edit, save, and play keyframe animation sequences stored as JSON files on LittleFS flash (`/animations/*.json`).
8. **Resource Optimization:**
   * Custom partition table (`1.92 MB` App / `1.92 MB` LittleFS) and compiler size optimizations (`-Os`) reduce flash usage to **58.5%** (used `1.18 MB / 2.03 MB`), leaving **842 KB of free headroom**.

---

### ⚠️ Hardware Validation Disclaimer

> [!NOTE]
> **Pending Physical Hardware Test:** All software components, C++ classes, REST API routes, LittleFS serialization handlers, and Web HUD layouts are fully verified via software compilation (`0 errors, 0 warnings`). Physical quadruped walking on real 8-servo robot hardware is currently **NOT HARDWARE TESTED / PENDING HARDWARE VALIDATION**.
