# Strider32 — ESP32 8-DOF Quadruped Robotics Platform & Web Control Center

**Version:** 0.1.0  
**Project Lead:** Sayuru Gunathilaka  
**License:** Apache 2.0  

An open-source, independently designed **ESP32 8-DOF micro-quadruped robotics platform** featuring direct GPIO PWM servo actuation, a latched hardware E-STOP safety system, static/dynamic gait motion engines, a RESTful API, and an embedded Vanilla JavaScript Web Control Center served directly from ESP32 LittleFS flash storage.

> [!NOTE]
> **Hardware Testing Notice:** The C++ firmware, REST API, Web HUD, and LittleFS file storage architecture are fully verified via software compilation (`0 errors, 0 warnings`, `58.5% flash usage`). Physical quadruped walking on real hardware is currently **NOT HARDWARE TESTED / PENDING HARDWARE VALIDATION**.

---

## 🌟 Key Features

* **Direct GPIO PWM Actuation (Default):** Controls 8 micro-servos directly using the ESP32 LEDC PWM peripheral (50 Hz, 14-bit timer resolution).
* **Hardware Abstraction Layer (`IServoDriver`):** Clean interface allowing seamless switching between `ESP32DirectServoDriver` (primary direct GPIO) and `PCA9685ServoDriver` (optional 16-channel I2C module).
* **Latched E-STOP Safety System:** Hardware emergency stop button on GPIO 34 (input-only GPI with external pull-up). Disables PWM outputs instantly and requires explicit physical release and API reset (`POST /api/v1/estop/reset`).
* **Motion Engine & Gaits:** 4-leg static stability crawl gait, dynamic trot gait, lateral strafing (left/right), and rotational turning (yaw left/right) with smooth non-blocking soft-start ramping ($1.5^\circ / 10\text{ms}$).
* **Embedded Web Control Center (HUD):** Responsive, dark cyber-industrial UI built with browser-native HTML5/CSS3/Vanilla ES6+ JS. No heavy external frameworks—served directly from LittleFS (`1.92 MB`).
* **Timeline Animation Studio:** Create, edit, save, and play keyframe animation sequences stored as JSON files on LittleFS flash.
* **RESTful API Suite:** Full API for system status, diagnostics telemetry, servo calibration, motion control, network management, and animation CRUD.
* **Resource Optimized:** Fits in $< 60\%$ of ESP32 Flash memory (`1.18 MB / 2.03 MB`), providing **842 KB of free headroom** for future development.

---

## 📐 Architecture Overview

```
                      +------------------------------------------+
                      |       Strider32 Web Control Center       |
                      |   (HTML5 / CSS3 / Vanilla ES6+ JS)       |
                      +--------------------+---------------------+
                                           | HTTP REST API
                                           v
                      +--------------------+---------------------+
                      |         ESPAsyncWebServer & API          |
                      +----+-------------------+-----------------+
                           |                   |
                           v                   v
            +--------------+----+     +--------+------------------+
            |   Gait Engine &   |     |      Config Manager       |
            | Animation Player  |     |   (LittleFS Flash Storage)|
            +--------------+----+     +--------+------------------+
                           |                   |
                           v                   v
            +--------------+-------------------+------------------+
            |            Safety System & Guard                  |
            +----------------------+------------------------------+
                                   |
                                   v
            +----------------------+------------------------------+
            |          IServoDriver Abstraction Layer             |
            +----------+-------------------------------+----------+
                       |                               |
                       v                               v
         +-------------+---------------+   +-----------+----------+
         | ESP32DirectServoDriver      |   | PCA9685ServoDriver   |
         | (Direct GPIO LEDC PWM)      |   | (Optional I2C Module)|
         +-----------------------------+   +----------------------+
```

---

## 🛠️ Hardware Requirements & Power Architecture

> [!CAUTION]
> **POWER WARNING:** All 8 servos must be powered from a dedicated external regulated 5V power supply (5V 5A BEC / Buck Converter). Do **NOT** power servos from the ESP32 5V/Vin pin or USB port. Connect external 5V GND to ESP32 GND to establish a common ground reference.

| Quantity | Component | Description / Wiring |
|---|---|---|
| 1 | ESP32-WROOM-32 | Main Microcontroller Board (30-pin DevKit) |
| 8 | SG90 / MG90S Micro Servos | Direct PWM Joint Actuation (GPIO 13, 14, 27, 26, 25, 33, 32, 4) |
| 1 | 5V 5A BEC / Buck Converter | External Regulated Servo Power Supply Rail |
| 1 | 0.96" SSD1306 OLED | I2C Display (SDA: GPIO 21, SCL: GPIO 22) |
| 1 | E-STOP Switch & 10kΩ Resistor | Normally-Open Button on GPIO 34 (External 10kΩ pull-up to 3.3V) |

---

## 🚀 Building & Flashing Firmware

### Software Prerequisites
* **Python 3.8+**
* **PlatformIO CLI Core:** Installed via `pip install platformio`

### 1. Build Firmware Binary
```bash
python -m platformio run -d "firmware"
```

### 2. Upload Firmware Binary
```bash
python -m platformio run -t upload -d "firmware"
```

### 3. Upload LittleFS Web & Config Filesystem
```bash
python -m platformio run -t uploadfs -d "firmware"
```

### 4. Launch Serial Monitor (115200 Baud)
```bash
python -m platformio device monitor -d "firmware" -b 115200
```

---

## 🌐 Web Control Center & REST API

Connect to the Strider32 Access Point (`SSID: Strider32-AP`) and open `http://192.168.4.1` (or `http://strider32.local`) in any modern desktop or mobile browser.

### Key API Endpoints
* `GET /api/v1/status` — Operational status & hardware telemetry.
* `GET /api/v1/diagnostics` — Real system telemetry (heap, CPU, uptime, safety, RSSI).
* `POST /api/v1/control` — Directional movement (`walk`, `stop`, `gait`, `speed`).
* `POST /api/v1/estop` & `POST /api/v1/estop/reset` — E-STOP trigger and latched reset.
* `GET /api/v1/servos` & `POST /api/v1/servos/calibrate` — Calibration subtrim offsets.
* `GET /api/v1/animations` & `POST /api/v1/animations` — LittleFS keyframe sequence CRUD.

---

## 🗺️ Project Roadmap

- [x] **v0.1.0 Core Firmware & Software Stack**
  - Direct ESP32 GPIO PWM Servo Driver with soft-start ramping.
  - Latched E-STOP Safety System (GPIO 34).
  - 4-leg static stability crawl gait, trot gait, strafing, and rotation.
  - RESTful API & embedded Vanilla JS Web Control Center.
  - LittleFS keyframe animation storage.
  - Resource optimization ($< 60\%$ Flash usage).
- [ ] **v0.2.0 Hardware Bring-Up & Validation**
  - Physical 8-servo assembly calibration.
  - BEC power rail load testing.
  - Physical walking gait tuning.
- [ ] **v0.3.0 Kinematics & Extended Telemetry**
  - 3D Inverse Kinematics (IK) leg solver.
  - MPU6050 IMU auto-balancing.
  - WebSocket telemetry streaming.

---

## 📄 License & Attribution

* **License:** Distributed under the [Apache 2.0 License](LICENSE).
* **Project Lead:** Designed and developed by **Sayuru Gunathilaka**.
* **Conceptual References:** Special acknowledgment to Dorian Borian (`dorianborian/sesame-robot`) for pioneering DIY 8-DOF quadruped robot concepts. See [ATTRIBUTION.md](ATTRIBUTION.md) for full details.
