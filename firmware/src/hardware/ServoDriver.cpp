#include "ServoDriver.h"

ServoDriver::ServoDriver() {
    m_config.backend = ServoBackendType::ESP32_DIRECT_LEDC;
    m_activeBackend = &m_directDriver; // Default Primary Backend: Direct ESP32 GPIO PWM
}

ServoDriver::~ServoDriver() {}

void ServoDriver::setBackend(ServoBackendType backend) {
    m_config.backend = backend;
    if (backend == ServoBackendType::PCA9685_I2C) {
        m_activeBackend = &m_pcaDriver;
    } else {
        m_activeBackend = &m_directDriver;
    }
}

bool ServoDriver::begin() {
    if (m_config.backend == ServoBackendType::PCA9685_I2C) {
        m_activeBackend = &m_pcaDriver;
    } else {
        m_activeBackend = &m_directDriver;
    }
    m_activeBackend->updateConfig(m_config);
    return m_activeBackend->begin();
}

void ServoDriver::update() {
    if (m_activeBackend) {
        m_activeBackend->update();
    }
}

void ServoDriver::setServoAngle(uint8_t jointIndex, float angle) {
    if (m_activeBackend) m_activeBackend->setServoAngle(jointIndex, angle);
}

float ServoDriver::getServoAngle(uint8_t jointIndex) const {
    return m_activeBackend ? m_activeBackend->getServoAngle(jointIndex) : 0.0f;
}

void ServoDriver::setServoOffset(uint8_t jointIndex, int16_t offset) {
    if (m_activeBackend) m_activeBackend->setServoOffset(jointIndex, offset);
}

int16_t ServoDriver::getServoOffset(uint8_t jointIndex) const {
    return m_activeBackend ? m_activeBackend->getServoOffset(jointIndex) : 0;
}

void ServoDriver::setServoLimits(uint8_t jointIndex, uint8_t minAngle, uint8_t maxAngle) {
    if (m_activeBackend) m_activeBackend->setServoLimits(jointIndex, minAngle, maxAngle);
}

void ServoDriver::setServoInverted(uint8_t jointIndex, bool inverted) {
    if (m_activeBackend) m_activeBackend->setServoInverted(jointIndex, inverted);
}

void ServoDriver::setAllAngles(const float angles[NUM_SERVOS]) {
    if (m_activeBackend) m_activeBackend->setAllAngles(angles);
}

void ServoDriver::emergencyDisableAll() {
    if (m_activeBackend) m_activeBackend->emergencyDisableAll();
}

const ServoConfig& ServoDriver::getConfig() const {
    return m_activeBackend ? m_activeBackend->getConfig() : m_config;
}

void ServoDriver::updateConfig(const ServoConfig& config) {
    m_config = config;
    setBackend(config.backend);
    if (m_activeBackend) m_activeBackend->updateConfig(config);
}

bool ServoDriver::isHardwareAttached() const {
    return m_activeBackend ? m_activeBackend->isHardwareAttached() : false;
}

ServoBackendType ServoDriver::getBackendType() const {
    return m_activeBackend ? m_activeBackend->getBackendType() : ServoBackendType::ESP32_DIRECT_LEDC;
}
