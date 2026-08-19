#include "ESP32DirectServoDriver.h"
#include <esp_arduino_version.h>

ESP32DirectServoDriver::ESP32DirectServoDriver()
    : m_initialized(false), m_rampingActive(true), m_lastRampMs(0) {
    m_config.backend = ServoBackendType::ESP32_DIRECT_LEDC;
    m_config.pulse_min_us = DEFAULT_PULSE_MIN_US;
    m_config.pulse_max_us = DEFAULT_PULSE_MAX_US;

    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        m_config.offsets[i] = 0;
        m_config.limits_min[i] = 10;  // Conservative safe minimum angle (10°)
        m_config.limits_max[i] = 170; // Conservative safe maximum angle (170°)
        m_config.inverted[i] = false;
        m_targetAngles[i] = 90.0f;    // Neutral safe hold angle (90°)
        m_currentAngles[i] = 90.0f;
    }
}

bool ESP32DirectServoDriver::begin() {
    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        int pin = DIRECT_SERVO_PINS[i];
        pinMode(pin, OUTPUT);

#if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcAttach(pin, LEDC_PWM_FREQ, LEDC_PWM_BITS);
#else
        ledcSetup(i, LEDC_PWM_FREQ, LEDC_PWM_BITS);
        ledcAttachPin(pin, i);
#endif
    }

    m_initialized = true;
    
    // Initial safe attach write to neutral 90 degrees
    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        uint32_t duty = angleToDutyCycle(i, m_currentAngles[i]);
        writeLedcDuty(i, duty);
    }

    m_lastRampMs = millis();
    return true;
}

uint32_t ESP32DirectServoDriver::angleToDutyCycle(uint8_t jointIndex, float angle) {
    if (jointIndex >= NUM_SERVOS) return 0;

    // Apply subtrim offset
    float adjusted = angle + m_config.offsets[jointIndex];

    // Apply inversion flag
    if (m_config.inverted[jointIndex]) {
        adjusted = 180.0f - adjusted;
    }

    // Enforce configured software limits
    adjusted = constrain(adjusted, (float)m_config.limits_min[jointIndex], (float)m_config.limits_max[jointIndex]);

    // Map 0 - 180 degrees dynamically using m_config.pulse_min_us and m_config.pulse_max_us
    uint16_t minUs = (m_config.pulse_min_us > 0) ? m_config.pulse_min_us : DEFAULT_PULSE_MIN_US;
    uint16_t maxUs = (m_config.pulse_max_us > 0) ? m_config.pulse_max_us : DEFAULT_PULSE_MAX_US;

    float pulseUs = minUs + (adjusted / 180.0f) * (maxUs - minUs);

    // Calculate 14-bit timer duty cycle ticks:
    // Period = 20,000 us (50Hz), Resolution = 2^14 = 16384 ticks
    // Ticks = pulseUs * 16384 / 20000 = pulseUs * 0.8192
    uint32_t duty = (uint32_t)(pulseUs * 0.8192f);
    return duty;
}

void ESP32DirectServoDriver::writeLedcDuty(uint8_t jointIndex, uint32_t duty) {
    if (jointIndex >= NUM_SERVOS || !m_initialized) return;

#if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(DIRECT_SERVO_PINS[jointIndex], duty);
#else
    ledcWrite(jointIndex, duty);
#endif
}

void ESP32DirectServoDriver::update() {
    if (!m_initialized || !m_rampingActive) return;

    uint32_t now = millis();
    if (now - m_lastRampMs < 10) return; // Ramp step interval: 10ms
    m_lastRampMs = now;

    const float MAX_STEP_DEG = 1.5f; // Controlled smooth ramping: 1.5° per 10ms (150 deg/sec max)

    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        if (abs(m_currentAngles[i] - m_targetAngles[i]) > 0.1f) {
            if (m_currentAngles[i] < m_targetAngles[i]) {
                m_currentAngles[i] += MAX_STEP_DEG;
                if (m_currentAngles[i] > m_targetAngles[i]) m_currentAngles[i] = m_targetAngles[i];
            } else {
                m_currentAngles[i] -= MAX_STEP_DEG;
                if (m_currentAngles[i] < m_targetAngles[i]) m_currentAngles[i] = m_targetAngles[i];
            }

            uint32_t duty = angleToDutyCycle(i, m_currentAngles[i]);
            writeLedcDuty(i, duty);
        }
    }
}

void ESP32DirectServoDriver::setServoAngle(uint8_t jointIndex, float angle) {
    if (jointIndex >= NUM_SERVOS) return;
    m_targetAngles[jointIndex] = angle;
}

float ESP32DirectServoDriver::getServoAngle(uint8_t jointIndex) const {
    if (jointIndex >= NUM_SERVOS) return 0.0f;
    return m_currentAngles[jointIndex];
}

void ESP32DirectServoDriver::setServoOffset(uint8_t jointIndex, int16_t offset) {
    if (jointIndex >= NUM_SERVOS) return;
    m_config.offsets[jointIndex] = offset;
}

int16_t ESP32DirectServoDriver::getServoOffset(uint8_t jointIndex) const {
    if (jointIndex >= NUM_SERVOS) return 0;
    return m_config.offsets[jointIndex];
}

void ESP32DirectServoDriver::setServoLimits(uint8_t jointIndex, uint8_t minAngle, uint8_t maxAngle) {
    if (jointIndex >= NUM_SERVOS) return;
    m_config.limits_min[jointIndex] = minAngle;
    m_config.limits_max[jointIndex] = maxAngle;
}

void ESP32DirectServoDriver::setServoInverted(uint8_t jointIndex, bool inverted) {
    if (jointIndex >= NUM_SERVOS) return;
    m_config.inverted[jointIndex] = inverted;
}

void ESP32DirectServoDriver::setAllAngles(const float angles[NUM_SERVOS]) {
    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        setServoAngle(i, angles[i]);
    }
}

void ESP32DirectServoDriver::emergencyDisableAll() {
    if (!m_initialized) return;
    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        writeLedcDuty(i, 0); // 0 duty cycle immediately turns off PWM signal
    }
}

void ESP32DirectServoDriver::updateConfig(const ServoConfig& config) {
    m_config = config;
}
