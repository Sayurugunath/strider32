# Primary Hardware Target Profile & Wiring Guide

**Target Hardware Profile:** ESP32-WROOM-32 (30-pin DevKit) + 8 Direct Servos + SSD1306 OLED + GPIO34 E-STOP

---

## ⚡ CRITICAL POWER ARCHITECTURE REQUIREMENTS

> [!CAUTION]
> **DO NOT POWER SERVOS DIRECTLY FROM THE ESP32 5V / VIN PIN OR USB POWER!**  
> An ESP32's onboard 3.3V linear regulator and USB 5V rail CANNOT supply the peak current required by 8 micro-servos (which can draw up to 4.0A to 5.0A total under leg motion stalls). Attempting to power servos from the MCU rail will cause severe voltage dips, constant brownout resets, and permanent microcontroller damage.

### Recommended Power Distribution Architecture

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

1. **Dedicated Servo Power Rail:** High-current 5V (5A peak) Buck Converter / BEC supplying V+ to all 8 servo positive wires (Red).
2. **Common Ground:** The ground wire (Black/Brown) of ALL 8 servos AND the 5V BEC MUST be connected together with the **ESP32 GND pin**.
3. **ESP32 Microcontroller Power:** Powered via USB port or regulated 5V into Vin/5V pin from a separate filtered line.

---

## 🛠️ Primary Hardware Bill of Materials (BOM)

| Qty | Component | Specification / Model | Purpose & Connection |
|---|---|---|---|
| 1 | Microcontroller | ESP32-WROOM-32 (30-pin DevKit) | Main Controller & Web Host |
| 8 | Micro Servos | SG90 or MG90S Metal Gear 9g Servos | Direct PWM Joint Actuation |
| 1 | Voltage Regulator | 5V 5A High-Current BEC / Step-down Buck | Servo Power Rail Supply |
| 1 | Display (Optional) | 0.96" SSD1306 OLED (128x64 I2C) | SDA: GPIO21, SCL: GPIO22 |
| 1 | E-STOP Switch | Normally-Open (NO) Push Button | GPIO34 & GND |
| 1 | Pull-Up Resistor | 10kΩ Resistor (1/4W) | External Pull-Up for GPIO34 E-STOP |
| 1 | Power Capacitor | 1000µF 10V Electrolytic | Servo 5V Power Rail Filtering |

---

## 📌 Centralized GPIO Pin Assignments (ESP32-WROOM-32 Profile)

```
                            ESP32-WROOM-32
                             +---\__/---+
                     3V3     | [ ]  [ ] | GND
                     EN      | [ ]  [ ] | GPIO 23
    (E-STOP)  GPI 34 <------ | [ ]  [ ] | GPIO 22 ----> I2C SCL (OLED)
   (ADC Bat)  GPI 35 <------ | [ ]  [ ] | GPIO 1  ----> TX0
              GPI 36         | [ ]  [ ] | GPIO 3  ----> RX0
              GPI 39         | [ ]  [ ] | GPIO 21 ----> I2C SDA (OLED)
 (FL Femur)  GPIO 34 (No)    |          |
 (BL Femur)  GPIO 33 ------> | [ ]  [ ] | GPIO 19
  (BR Coxa)  GPIO 32 ------> | [ ]  [ ] | GPIO 18
  (FR Coxa)  GPIO 27 ------> | [ ]  [ ] | GPIO 5
  (FR Femur) GPIO 26 ------> | [ ]  [ ] | GPIO 17
  (BL Coxa)  GPIO 25 ------> | [ ]  [ ] | GPIO 16
 (FL Coxa)   GPIO 13 ------> | [ ]  [ ] | GPIO 4  -----> (BR Femur)
 (FL Femur)  GPIO 14 ------> | [ ]  [ ] | GPIO 0
             (Status)        | [ ]  [ ] | GPIO 2
                             +----------+
```

| Joint Index | Joint Name | ESP32 GPIO Pin | Function |
|---|---|---|---|
| 0 | FL Coxa (Hip) | **GPIO 13** | PWM LEDC Channel 0 |
| 1 | FL Femur (Knee) | **GPIO 14** | PWM LEDC Channel 1 |
| 2 | FR Coxa (Hip) | **GPIO 27** | PWM LEDC Channel 2 |
| 3 | FR Femur (Knee) | **GPIO 26** | PWM LEDC Channel 3 |
| 4 | BL Coxa (Hip) | **GPIO 25** | PWM LEDC Channel 4 |
| 5 | BL Femur (Knee) | **GPIO 33** | PWM LEDC Channel 5 |
| 6 | BR Coxa (Hip) | **GPIO 32** | PWM LEDC Channel 6 |
| 7 | BR Femur (Knee) | **GPIO 4** | PWM LEDC Channel 7 |
