# Contributing Guidelines

Thank you for your interest in contributing to the **ESP32 Quadruped Robotics Platform**!

---

## 1. Development Setup

### Prerequisites
* **Python 3.8+**
* **PlatformIO CLI Core:** Installed via `pip install platformio`
* **VS Code (Optional):** Recommended IDE with the PlatformIO extension installed.

### Hardware Prerequisites
* **ESP32-WROOM-32** (30-pin DevKit board)
* **8x Micro Servos** (SG90 / MG90S class)
* **0.96" SSD1306 OLED Display** (I2C)
* **5V 5A External Regulated BEC / Buck Converter** for servo power
* **Normally-Open Switch** & 10kΩ Pull-Up Resistor (GPIO 34 E-STOP)

---

## 2. Firmware Build & Flash Workflow

### Build Firmware
```bash
python -m platformio run -d "firmware"
```

### Upload Firmware Binary
```bash
python -m platformio run -t upload -d "firmware"
```

### Upload Web & LittleFS Filesystem Assets
```bash
python -m platformio run -t uploadfs -d "firmware"
```

### Launch Real-Time Serial Monitor
```bash
python -m platformio device monitor -d "firmware" -b 115200
```

---

## 3. Code Standards & Architecture Guidelines

1. **Non-Blocking Execution:** Avoid `delay()` calls inside `loop()` or motion updates. Use non-blocking timers (`millis()`).
2. **Safety First:** All movement calls must be guarded by `safetySystem->canExecuteMotion()`. Never bypass safety checks.
3. **Clean Abstractions:** Maintain hardware separation (`IServoDriver` interface). Do not tie gait logic directly to specific hardware pins or registers.
4. **Web Frontend:** Use browser-native HTML5, CSS3, and Vanilla JavaScript (ES6+). Do not add heavy external node frameworks to keep LittleFS flash footprint minimal.

---

## 4. Submitting Pull Requests

1. Fork the repository and create a feature branch (`git checkout -b feature/my-feature`).
2. Ensure firmware compiles cleanly with **0 errors and 0 warnings**:
   `python -m platformio run -d "firmware"`
3. Push your branch and open a Pull Request with a clear description of changes.

---

*Thank you for helping build accessible open-source quadruped robotics!*
