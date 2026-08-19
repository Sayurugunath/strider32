# Contributing Guidelines for Strider32

Thank you for your interest in contributing to **Strider32 — ESP32 8-DOF Quadruped Robotics Platform**!

---

## 🚀 10-Step Developer Workflow

Follow this step-by-step workflow when contributing bug fixes, features, or improvements to Strider32:

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/Sayurugunath/strider32.git
   cd strider32
   ```
2. **Install Python:**
   Ensure Python 3.8 or newer is installed on your development machine.
3. **Install PlatformIO CLI Core:**
   ```bash
   pip install platformio
   ```
4. **Build Local Firmware:**
   Verify compilation using PlatformIO CLI:
   ```bash
   python -m platformio run -d "firmware"
   ```
5. **Upload Firmware Binary (to Physical Hardware):**
   ```bash
   python -m platformio run -t upload -d "firmware"
   ```
6. **Upload LittleFS Filesystem Assets (Web HUD & Config):**
   ```bash
   python -m platformio run -t uploadfs -d "firmware"
   ```
7. **Run Serial Monitor:**
   Inspect 115200 baud console telemetry:
   ```bash
   python -m platformio device monitor -d "firmware" -b 115200
   ```
8. **Make Code Changes:**
   Implement your feature or fix in `firmware/src/`, `firmware/include/`, or `web/public/`.
9. **Run Local Verification Build:**
   Confirm your changes compile with **0 errors and 0 warnings**:
   ```bash
   python -m platformio run -d "firmware"
   ```
10. **Submit a Pull Request:**
    Push your topic branch to GitHub and open a Pull Request using the PR template.

---

## ⚠️ Testing Expectations & Verification Classification

> [!IMPORTANT]
> **Hardware Testing Note:** Physical quadruped walking on real 8-servo hardware is currently **NOT HARDWARE TESTED / PENDING HARDWARE VALIDATION**.

When submitting a Pull Request, contributors MUST explicitly classify their changes into:

* **`SOFTWARE VERIFIED`**: Firmware build succeeds with 0 errors/0 warnings (`python -m platformio run -d firmware`), API endpoints respond correctly, unit/logic models verified.
* **`HARDWARE VERIFIED`**: Verified on physical ESP32 board and 8-servo quadruped hardware.

---

## 🛠️ Architecture & Coding Standards

1. **Non-Blocking Execution:** Avoid blocking `delay()` calls inside `loop()` or motion routines. Use non-blocking timers (`millis()`).
2. **Safety System Guarding:** All movement commands MUST check `safetySystem->canExecuteMotion()`. Never bypass safety limits or E-STOP state checks.
3. **Hardware Abstraction Layer:** Maintain clean separation via `IServoDriver`.
4. **Web Frontend:** Use browser-native HTML5, CSS3, and Vanilla JavaScript (ES6+). Do not add heavy external node frameworks to maintain the small LittleFS footprint (`< 60%` flash usage).

---

*Thank you for building accessible open-source quadruped robotics!*
