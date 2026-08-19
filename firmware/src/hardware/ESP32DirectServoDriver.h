#ifndef ESP32_DIRECT_SERVO_DRIVER_H
#define ESP32_DIRECT_SERVO_DRIVER_H

#include "IServoDriver.h"

// 50 Hz PWM Frequency for RC Servos (20ms Period)
constexpr uint32_t LEDC_PWM_FREQ = 50;
constexpr uint8_t  LEDC_PWM_BITS = 14; // 14-bit resolution: 0..16383 counts per 20ms

class ESP32DirectServoDriver : public IServoDriver {
public:
    ESP32DirectServoDriver();
    virtual ~ESP32DirectServoDriver() {}

    virtual bool begin() override;
    virtual void update() override;

    virtual void setServoAngle(uint8_t jointIndex, float angle) override;
    virtual float getServoAngle(uint8_t jointIndex) const override;

    virtual void setServoOffset(uint8_t jointIndex, int16_t offset) override;
    virtual int16_t getServoOffset(uint8_t jointIndex) const override;

    virtual void setServoLimits(uint8_t jointIndex, uint8_t minAngle, uint8_t maxAngle) override;
    virtual void setServoInverted(uint8_t jointIndex, bool inverted) override;

    virtual void setAllAngles(const float angles[NUM_SERVOS]) override;
    virtual void emergencyDisableAll() override;

    virtual const ServoConfig& getConfig() const override { return m_config; }
    virtual void updateConfig(const ServoConfig& config) override;

    virtual bool isHardwareAttached() const override { return m_initialized; }
    virtual ServoBackendType getBackendType() const override { return ServoBackendType::ESP32_DIRECT_LEDC; }

private:
    ServoConfig m_config;
    float m_targetAngles[NUM_SERVOS];
    float m_currentAngles[NUM_SERVOS];
    bool  m_initialized;
    bool  m_rampingActive;
    uint32_t m_lastRampMs;

    uint32_t angleToDutyCycle(uint8_t jointIndex, float angle);
    void writeLedcDuty(uint8_t jointIndex, uint32_t duty);
};

#endif // ESP32_DIRECT_SERVO_DRIVER_H
