#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

#include <Arduino.h>

// ============================================================================
// PRIMARY HARDWARE TARGET PROFILE: ESP32-WROOM-32 (30-pin DevKit)
// ============================================================================

// ============================================================================
// 1. I2C BUS PINS (SSD1306 OLED Display / Optional PCA9685)
// ============================================================================
constexpr int PIN_I2C_SDA = 21;
constexpr int PIN_I2C_SCL = 22;

// Optional PCA9685 I2C Module Configuration
constexpr uint8_t PCA9685_I2C_ADDR = 0x40;
constexpr uint32_t SERVO_PWM_FREQ   = 50; // 50 Hz standard for RC servos (20ms period)

// ============================================================================
// 2. DIRECT ESP32 GPIO SERVO PIN MAPPING (PRIMARY TARGET)
// ============================================================================
// Hardware Safety Validation Matrix for ESP32-WROOM-32:
// - GPIO 13: Joint 0 - FL Coxa  (Safe output, Touch4, RTC_GPIO14)
// - GPIO 14: Joint 1 - FL Femur (Safe output, Touch6, RTC_GPIO16)
// - GPIO 27: Joint 2 - FR Coxa  (Safe output, Touch7, RTC_GPIO17)
// - GPIO 26: Joint 3 - FR Femur (Safe output, DAC2, RTC_GPIO7)
// - GPIO 25: Joint 4 - BL Coxa  (Safe output, DAC1, RTC_GPIO6)
// - GPIO 33: Joint 5 - BL Femur (Safe output, Touch8, RTC_GPIO8)
// - GPIO 32: Joint 6 - BR Coxa  (Safe output, Touch9, RTC_GPIO9)
// - GPIO 4 : Joint 7 - BR Femur (Safe output, Touch0, RTC_GPIO10)
//
// Reserved Pins (Excluded from Servo Mapping):
// - GPIO 6, 7, 8, 9, 10, 11: Connected to SPI Flash memory (CRITICAL: DO NOT USE!)
// - GPIO 0, 2, 5, 12, 15: Bootstrapping / Flash Voltage / Log Output pins
// - GPIO 34, 35, 36, 39: Input-only GPI pins (Cannot output PWM)
// ============================================================================

constexpr int PIN_SERVO_FL_COXA  = 13; // Joint 0: Front-Left Hip
constexpr int PIN_SERVO_FL_FEMUR = 14; // Joint 1: Front-Left Knee
constexpr int PIN_SERVO_FR_COXA  = 27; // Joint 2: Front-Right Hip
constexpr int PIN_SERVO_FR_FEMUR = 26; // Joint 3: Front-Right Knee
constexpr int PIN_SERVO_BL_COXA  = 25; // Joint 4: Back-Left Hip
constexpr int PIN_SERVO_BL_FEMUR = 33; // Joint 5: Back-Left Knee
constexpr int PIN_SERVO_BR_COXA  = 32; // Joint 6: Back-Right Hip
constexpr int PIN_SERVO_BR_FEMUR = 4;  // Joint 7: Back-Right Knee

constexpr int DIRECT_SERVO_PINS[8] = {
    PIN_SERVO_FL_COXA,  // 0
    PIN_SERVO_FL_FEMUR, // 1
    PIN_SERVO_FR_COXA,  // 2
    PIN_SERVO_FR_FEMUR, // 3
    PIN_SERVO_BL_COXA,  // 4
    PIN_SERVO_BL_FEMUR, // 5
    PIN_SERVO_BR_COXA,  // 6
    PIN_SERVO_BR_FEMUR  // 7
};

// ============================================================================
// 3. SAFETY & SYSTEM INPUT PINS
// ============================================================================
// E-STOP Input Pin Assignment:
// - Selected: GPIO 34 (Input-only GPI pin, ADC1_CH6).
// - Hardware Requirement: Requires an EXTERNAL 10kΩ pull-up resistor connected 
//   between GPIO 34 and 3.3V power rail.
// - Switch Configuration: Normally-Open (NO) switch connected between GPIO 34 and GND.
//   Pressing the E-STOP button pulls GPIO 34 to LOW (0V).
// - Bootstrapping Safety: GPIO 34 is not a bootstrapping pin; pressing E-STOP 
//   during boot will NOT cause brownout or flashing mode interference.
// ============================================================================
constexpr int PIN_ESTOP_BUTTON = 34; // Dedicated Input-only pin (Requires 10k external pullup)
constexpr int PIN_BATTERY_ADC  = 35; // Dedicated Input-only pin (ADC1_CH7)
constexpr int PIN_STATUS_LED   = 2;  // Onboard LED

#endif // PIN_DEFINITIONS_H
