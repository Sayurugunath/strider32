#include "PCA9685ServoDriver.h"
#include <Wire.h>

PCA9685ServoDriver::PCA9685ServoDriver()
    : m_pwm(PCA9685_I2C_ADDR), m_initialized(false), m_lastRampMs(0) {
    m_config.backend = ServoBackendType::PCA9685_I2C;
    m_config.pulse_min_us = DEFAULT_PULSE_MIN_US;
    m_config.pulse_max_us = DEFAULT_PULSE_MAX_US;

    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        m_config.offsets[i] = 0;
        m_config.limits_min[i] = 10;
        m_config.limits_max[i] = 170;
        m_config.inverted[i] = false;
        m_targetAngles[i] = 90.0f;
        m_currentAngles[i] = 90.0f;
    }
}

bool PCA9685ServoDriver::begin() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    m_pwm.begin();
    m_pwm.setPWMFreq(SERVO_PWM_FREQ);
    delay(10);
    m_initialized = true;
    m_lastRampMs = millis();
    return true;
}

uint16_t PCA9685ServoDriver::angleToPulse(uint8_t jointIndex, float angle) {
    if (jointIndex >= NUM_SERVOS) return 102;
    
    float adjustedAngle = angle + m_config.offsets[jointIndex];

    if (m_config.inverted[jointIndex]) {
        adjustedAngle = 180.0f - adjustedAngle;
    }

    adjustedAngle = constrain(adjustedAngle, (float)m_config.limits_min[jointIndex], (float)m_config.limits_max[jointIndex]);

    uint16_t minUs = (m_config.pulse_min_us > 0) ? m_config.pulse_min_us : 600;
    uint16_t maxUs = (m_config.pulse_max_us > 0) ? m_config.pulse_max_us : 2400;

    // Map minUs - maxUs (in microseconds) to 12-bit PCA9685 count (4096 counts per 20ms)
    // count = pulseUs * 4096 / 20000 = pulseUs * 0.2048
    float pulseUs = minUs + (adjustedAngle / 180.0f) * (maxUs - minUs);
    uint16_t pulse = (uint16_t)(pulseUs * 0.2048f);
    return pulse;
}

void PCA9685ServoDriver::update() {
    if (!m_initialized) return;

    uint32_t now = millis();
    if (now - m_lastRampMs < 10) return;
    m_lastRampMs = now;

    const float MAX_STEP_DEG = 1.5f;

    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        if (abs(m_currentAngles[i] - m_targetAngles[i]) > 0.1f) {
            if (m_currentAngles[i] < m_targetAngles[i]) {
                m_currentAngles[i] += MAX_STEP_DEG;
                if (m_currentAngles[i] > m_targetAngles[i]) m_currentAngles[i] = m_targetAngles[i];
            } else {
                m_currentAngles[i] -= MAX_STEP_DEG;
                if (m_currentAngles[i] < m_targetAngles[i]) m_currentAngles[i] = m_targetAngles[i];
            }

            uint16_t pulse = angleToPulse(i, m_currentAngles[i]);
            m_pwm.setPWM(i, 0, pulse);
        }
    }
}

void PCA9685ServoDriver::setServoAngle(uint8_t jointIndex, float angle) {
    if (jointIndex >= NUM_SERVOS) return;
    m_targetAngles[jointIndex] = angle;
}

float PCA9685ServoDriver::getServoAngle(uint8_t jointIndex) const {
    if (jointIndex >= NUM_SERVOS) return 0.0f;
    return m_currentAngles[jointIndex];
}

void PCA9685ServoDriver::setServoOffset(uint8_t jointIndex, int16_t offset) {
    if (jointIndex >= NUM_SERVOS) return;
    m_config.offsets[jointIndex] = offset;
}

int16_t PCA9685ServoDriver::getServoOffset(uint8_t jointIndex) const {
    if (jointIndex >= NUM_SERVOS) return 0;
    return m_config.offsets[jointIndex];
}

void PCA9685ServoDriver::setServoLimits(uint8_t jointIndex, uint8_t minAngle, uint8_t maxAngle) {
    if (jointIndex >= NUM_SERVOS) return;
    m_config.limits_min[jointIndex] = minAngle;
    m_config.limits_max[jointIndex] = maxAngle;
}

void PCA9685ServoDriver::setServoInverted(uint8_t jointIndex, bool inverted) {
    if (jointIndex >= NUM_SERVOS) return;
    m_config.inverted[jointIndex] = inverted;
}

void PCA9685ServoDriver::setAllAngles(const float angles[NUM_SERVOS]) {
    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        setServoAngle(i, angles[i]);
    }
}

void PCA9685ServoDriver::emergencyDisableAll() {
    if (!m_initialized) return;
    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        m_pwm.setPWM(i, 0, 0);
    }
}

void PCA9685ServoDriver::updateConfig(const ServoConfig& config) {
    m_config = config;
}
