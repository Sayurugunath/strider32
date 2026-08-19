# Phase 1 Final Review — Hardware Validation & Servo Abstraction

**Project:** Independent ESP32 8-DOF Quadruped Robotics Platform  
**Lead Developer:** Sayuru Gunathilaka  
**Date:** August 19, 2026  
**Status:** PHASE 1 VERIFIED & REAL PLATFORMIO BUILD SUCCESSFUL  

---

## 1. Primary Hardware Profile (ESP32-WROOM-32 Target)

* **Microcontroller Board:** ESP32-WROOM-32 (30-pin DevKit).
* **Actuation System:** 8 micro-servos (SG90 / MG90S class) driven directly via ESP32 GPIO PWM (LEDC peripheral).
* **Display System:** 0.96" SSD1306 OLED (128x64) via I2C (`SDA` GPIO 21, `SCL` GPIO 22).
* **Safety Interrupt:** Hardware Emergency Stop (E-STOP) button on **GPIO 34**.

---

## 2. Centralized GPIO Pin Assignment & Safety Matrix

| Joint / Peripheral | Joint Name | ESP32 GPIO | Pin Type | Validation & Electrical Configuration |
|---|---|---|---|---|
| Joint 0 | FL Coxa (Hip) | **GPIO 13** | Output (PWM) | Safe output (Touch4, RTC_GPIO14). LEDC Channel 0. |
| Joint 1 | FL Femur (Knee)| **GPIO 14** | Output (PWM) | Safe output (Touch6, RTC_GPIO16). LEDC Channel 1. |
| Joint 2 | FR Coxa (Hip) | **GPIO 27** | Output (PWM) | Safe output (Touch7, RTC_GPIO17). LEDC Channel 2. |
| Joint 3 | FR Femur (Knee)| **GPIO 26** | Output (PWM) | Safe output (DAC2, RTC_GPIO7). LEDC Channel 3. |
| Joint 4 | BL Coxa (Hip) | **GPIO 25** | Output (PWM) | Safe output (DAC1, RTC_GPIO6). LEDC Channel 4. |
| Joint 5 | BL Femur (Knee)| **GPIO 33** | Output (PWM) | Safe output (Touch8, RTC_GPIO8). LEDC Channel 5. |
| Joint 6 | BR Coxa (Hip) | **GPIO 32** | Output (PWM) | Safe output (Touch9, RTC_GPIO9). LEDC Channel 6. |
| Joint 7 | BR Femur (Knee)| **GPIO 4**  | Output (PWM) | Safe output (Touch0, RTC_GPIO10). LEDC Channel 7. |
| **I2C SDA** | OLED Display | **GPIO 21** | I2C Data | Dedicated I2C bus data line. |
| **I2C SCL** | OLED Display | **GPIO 22** | I2C Clock | Dedicated I2C bus clock line. |
| **E-STOP Switch**| Safety Cutoff | **GPIO 34** | Input-only (GPI)| Dedicated Input-only pin (ADC1_CH6). |

### ⚡ E-STOP Input Electrical Configuration & Safety Notes
* **Selected Pin:** **GPIO 34** (Input-only GPI pin).
* **Hardware Requirement:** Requires an **external 10kΩ pull-up resistor** connected between GPIO 34 and the 3.3V rail. (GPIO 34 is input-only and lacks internal pull-up resistors in ESP32 hardware).
* **Switch Connection:** Normally-Open (NO) switch connected between GPIO 34 and GND. Pressing the button pulls GPIO 34 to GND (Active LOW).
* **Strapping Pin Protection:** GPIO 34 is **not a bootstrapping pin**. Operating the E-STOP button during boot will NOT interfere with flash programming or cause brownout boot loops (resolving the previous GPIO 15 conflict).

---

## 3. Servo PWM Range & Configurable Pulse Limits

* **PWM Frequency:** 50 Hz (20,000 µs period).
* **LEDC Timer Resolution:** 14-bit ($2^{14} = 16,384$ counts; $0.8192 \text{ ticks/µs}$).
* **Configurable Pulse Limits (`ServoConfig`):**
  * `pulse_min_us`: Configurable minimum pulse width (Conservative Default: **600 µs**).
  * `pulse_max_us`: Configurable maximum pulse width (Conservative Default: **2400 µs**).
  * `limits_min`: Per-joint safe minimum angle boundary (Conservative Default: **10°**).
  * `limits_max`: Per-joint safe maximum angle boundary (Conservative Default: **170°**).
* **Angle-to-Duty Cycle Formula:**
  $$\text{pulseUs} = \text{pulse\_min\_us} + \left( \frac{\text{adjustedAngle}}{180.0} \right) \times (\text{pulse\_max\_us} - \text{pulse\_min\_us})$$
  $$\text{dutyTicks} = \lfloor \text{pulseUs} \times 0.8192 \rfloor$$

---

## 4. Safe Servo Startup & Ramping Sequence

```
Boot
  │
  v
Initialize Storage & Filesystem (LittleFS)
  │
  v
Load Servo Calibration & Bounds (`servos.json`) BEFORE Output Setup
  │
  v
Initialize Hardware Drivers (`ESP32DirectServoDriver::begin()`)
  ├─ Configure GPIOs 13,14,27,26,25,33,32,4 as LEDC Output
  └─ Set initial duty cycle to neutral 90°
  │
  v
Initialize Safety System (`SafetySystem::begin()` on GPIO 34)
  │
  v
Establish Safe Neutral Pose Target (`Kinematics::RobotPose::REST`)
  │
  v
Non-Blocking Ramping Task (`g_servoDriver.update()`)
  └─ Steps angle outputs smoothly at max 1.5° / 10ms (150°/sec max speed limit)
```

---

## 5. Power Supply Architecture

```
                 +--------------------------------------+
                 |      External Power Source           |
                 | (2S 7.4V LiPo or 2x 18650 Li-ion)    |
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

> [!CAUTION]
> **ESP32 5V RAIL WARNING:** All 8 micro-servos must be powered from a dedicated external regulated 5V power supply (5V 5A BEC). Do NOT power servos from the ESP32 5V/Vin pin or USB port. Connect external 5V GND to ESP32 GND to establish a common ground reference.

---

## 6. Real PlatformIO Compilation Build Result

* **Environment:** PlatformIO 6.1.19 / Espressif 32 (v6.13.0) / ESP32 Dev Module
* **Build Command:** `python -m platformio run -d "firmware"`
* **Compiler Status:** **`[SUCCESS]`** (Code 0)
* **Build Output Log Summary:**
  ```
  Processing esp32dev (platform: espressif32 @ ^6.5.0; board: esp32dev; framework: arduino)
  --------------------------------------------------------------------------------
  Building in release mode
  Compiling .pio\build\esp32dev\src\api\ApiRoutes.cpp.o
  Compiling .pio\build\esp32dev\src\config\ConfigManager.cpp.o
  Compiling .pio\build\esp32dev\src\hardware\ESP32DirectServoDriver.cpp.o
  Compiling .pio\build\esp32dev\src\hardware\PCA9685ServoDriver.cpp.o
  Compiling .pio\build\esp32dev\src\hardware\SafetySystem.cpp.o
  Compiling .pio\build\esp32dev\src\hardware\ServoDriver.cpp.o
  Compiling .pio\build\esp32dev\src\main.cpp.o
  Compiling .pio\build\esp32dev\src\motion\AnimationPlayer.cpp.o
  Compiling .pio\build\esp32dev\src\motion\GaitEngine.cpp.o
  Compiling .pio\build\esp32dev\src\motion\Kinematics.cpp.o
  Compiling .pio\build\esp32dev\src\network\NetworkManager.cpp.o
  Linking .pio\build\esp32dev\firmware.elf
  Checking size .pio\build\esp32dev\firmware.elf
  RAM:   [=         ]  14.9% (used 48892 bytes from 327680 bytes)
  Flash: [=======   ]  71.8% (used 940905 bytes from 1310720 bytes)
  Building .pio\build\esp32dev\firmware.bin
  esptool.py v4.11.0
  Successfully created esp32 image.
  ========================= [SUCCESS] Took 76.31 seconds =========================
  ```
* **Errors & Warnings:** 0 Errors, 0 Warnings.

---

## 7. Remaining Hardware Risks

1. **SG90 Plastic Gear Mechanical Binding:** Operating SG90 servos beyond physical mechanical limits ($< 10^\circ$ or $> 170^\circ$) can strip plastic gears. Software angle bounds (`limits_min` / `limits_max`) are enforced at 10° and 170° by default.
2. **Missing External Pull-up Resistor:** If the user forgets to install the 10kΩ external pull-up resistor on GPIO 34, floating noise on GPIO 34 may falsely trigger E-STOP interrupts.

---

*End of Phase 1 Final Review. Waiting for approval before proceeding to Phase 2.*
