# Phase 5 Hardware Bring-Up & Validation Report

**Project:** Independent ESP32 8-DOF Quadruped Robotics Platform  
**Lead Developer:** Sayuru Gunathilaka  
**Date:** August 19, 2026  
**Status:** HARDWARE BRING-UP PROCEDURES PREPARED & SOFTWARE STACK VERIFIED (`[SUCCESS]`)  

---

## 1. Primary Hardware Profile & Wiring Architecture

* **Microcontroller:** ESP32-WROOM-32 (30-pin DevKit, 4MB Flash).
* **Actuator Hardware:** 8 Micro Servos (SG90 / MG90S class) connected directly via ESP32 GPIO PWM.
* **Display System:** 0.96" SSD1306 OLED (128x64) via I2C (`SDA` GPIO 21, `SCL` GPIO 22).
* **Safety Interrupt:** Hardware Emergency Stop switch on **GPIO 34** (Input-only GPI pin with external 10kΩ pull-up resistor to 3.3V, Active LOW).
* **Power Supply:** External 5V 5A regulated BEC / Buck Converter supplying servo power rail ($+$) with common ground to ESP32 GND.

```
                     +--------------------------------------+
                     |     External 7.4V LiPo / Battery     |
                     +----+---------------------------+-----+
                          | (+)                       | (-)
                      [ SWITCH ]                  [ BATTERY GND ]
                          |                           |
                          v                           |
            +-------------+-------------+             |
            | 5V 5A High-Current BEC /  |             |
            | Buck Converter            |             |
            +------+--------------+-----+             |
                   | (+5V)        | GND               |
                   |              +-------------------+-------------------+
                   |                                  |                   |
                   v                                  v                   v
          +----------------+                 +-----------------+  +---------------+
          | Servo Power V+ |                 | ESP32 GND Pin   |  | Servo GND Pins|
          | (All 8 Servos) |                 | (Common Ground) |  | (All 8 Servos)|
          +----------------+                 +-----------------+  +---------------+
```

---

## 2. Flash & Deployment Command Suite (Phase 5B)

The exact Python PlatformIO commands required for deployment and bring-up:

1. **Build Firmware Binary:**
   ```bash
   python -m platformio run -d "firmware"
   ```
2. **Upload Firmware Binary to ESP32:**
   ```bash
   python -m platformio run -t upload -d "firmware"
   ```
3. **Upload LittleFS Filesystem (Web Control Center & Animations):**
   ```bash
   python -m platformio run -t uploadfs -d "firmware"
   ```
4. **Launch Real-Time Serial Monitor (115200 Baud):**
   ```bash
   python -m platformio device monitor -d "firmware" -b 115200
   ```

---

## 3. Subsystem Validation Classification Matrix

Per instructions, every test scenario is explicitly classified into **HARDWARE VERIFIED**, **SOFTWARE VERIFIED**, or **NOT TESTED**:

| Subsystem / Test Scenario | Verification Status | Verification Details & Logs |
|---|---|---|
| C++ Firmware Compilation | **SOFTWARE VERIFIED** | 0 Errors, 0 Warnings (`[SUCCESS]`). RAM: 16.4%, Flash: 58.5%. |
| Custom Partition Table (1.92MB App / 1.92MB LittleFS) | **SOFTWARE VERIFIED** | Verified partition binary mapping. |
| LittleFS Web Asset Storage & API Routing | **SOFTWARE VERIFIED** | Static asset serving (`/public/index.html`) & REST routes verified. |
| Safety State Machine & E-STOP Latch Reset | **SOFTWARE VERIFIED** | `POST /api/v1/estop/reset` and physical pin check logic verified. |
| Motion Engine (Trot, 4-Phase Crawl, Strafe, Rotation) | **SOFTWARE VERIFIED** | Math models for joint angles & stance transitions verified. |
| ESP32 Physical Boot Sequence (No Servos) | **NOT TESTED** | Pending connection to physical ESP32 board. |
| SSD1306 OLED Display Initialization | **NOT TESTED** | Pending I2C bus probe on physical hardware. |
| Single Servo Angle & Calibration Range (GPIO 13) | **NOT TESTED** | Pending physical servo connection test. |
| 8-Servo Simultaneous PWM Drive & BEC Power Rail | **NOT TESTED** | Pending physical BEC 5V 5A power rail test. |
| Static Poses (`REST`, `STAND`, `SIT`) | **NOT TESTED** | Pending physical quadruped robot stance test. |
| Physical 4-Leg Crawl Walking on Ground | **NOT TESTED** | Pending physical quadruped chassis testing. |
| Physical E-STOP Button (GPIO 34 Interrupt Cutoff) | **NOT TESTED** | Pending physical button toggle test. |

---

## 4. Hardware Bring-Up & Validation Testing Procedures

### Procedure 1: Boot Test (No Servos Connected)
1. Flash firmware and LittleFS filesystem using PlatformIO upload commands.
2. Monitor serial output at 115200 baud.
3. Expected output trace:
   ```text
   --- Initializing ESP32 Quadruped Robotics Platform ---
   [LittleFS] Mounted successfully.
   [Config] Loaded servo calibration & network configs.
   [Driver] ESP32 Direct LEDC PWM initialized (50Hz).
   [Safety] E-STOP monitoring active on GPIO 34.
   [Network] AP Mode started. SSID: QuadRobot-AP, IP: 192.168.4.1
   [Web] HTTP Async Server running on port 80.
   System Initialized Successfully.
   ```
4. Verify free heap remains stable (>170 KB) without watchdog timeouts or boot loops.

### Procedure 2: Single Servo Test (Servo #0 on GPIO 13)
1. Connect 1 micro-servo to GPIO 13, BEC 5V (+), and BEC GND (-).
2. Boot system. Verify servo holds neutral 90° position cleanly.
3. Via Web Control Center (`Calibration` tab), adjust subtrim slider from $-15^\circ$ to $+15^\circ$. Verify small smooth movement.
4. Trigger E-STOP button. Verify PWM output disables immediately.

### Procedure 3: 8-Servo Integration & BEC Power Validation
1. Connect all 8 servos to GPIOs 13, 14, 27, 26, 25, 33, 32, 4.
2. Connect external 5V 5A BEC supply to servo power rail with common GND to ESP32.
3. Boot system. Verify all 8 channels hold neutral 90° without brownouts, ESP32 resets, or excessive jitter.
4. Execute `STAND` pose followed by `REST` pose. Verify smooth transition ramping ($1.5^\circ / 10\text{ms}$).

### Procedure 4: Crawl Gait & Directional Walk Validation
1. Place quadruped robot in a safe test area on a flat surface (not on an elevated table).
2. Open Web Control Center `Motion` tab.
3. Set Gait to `CRAWL` (Static 4-Phase Crawl) at 30% speed.
4. Test Forward ($\mathbf{\blacktriangle}$), Backward ($\mathbf{\blackdown triangle}$), Strafe Left ($\mathbf{\blackleft triangle}$), Strafe Right ($\mathbf{\blackright triangle}$), and Yaw Rotation ($\circlearrowleft / \circlearrowright$).
5. Test physical E-STOP button press during motion. Confirm motion freezes immediately and state switches to `ESTOPPED`.

---

## 5. Summary & Next Steps

* The software stack, REST API, Web Control Center, and size-optimized firmware binary (`58.5%` Flash usage, `0` Errors, `0` Warnings) are fully verified and ready for hardware deployment.
* Physical hardware tests are marked **NOT TESTED** pending connection to physical ESP32 and quadruped robot hardware.

---

*End of Phase 5 Hardware Report. Waiting for approval before proceeding.*
