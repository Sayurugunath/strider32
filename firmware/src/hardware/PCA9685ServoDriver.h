#ifndef PCA9685_SERVO_DRIVER_H
#define PCA9685_SERVO_DRIVER_H

#include "IServoDriver.h"
#include <Adafruit_PWMServoDriver.h>

class PCA9685ServoDriver : public IServoDriver {
public:
    PCA9685ServoDriver();
    virtual ~PCA9685ServoDriver() {}

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
    virtual ServoBackendType getBackendType() const override { return ServoBackendType::PCA9685_I2C; }

private:
    Adafruit_PWMServoDriver m_pwm;
    ServoConfig m_config;
    float m_targetAngles[NUM_SERVOS];
    float m_currentAngles[NUM_SERVOS];
    bool  m_initialized;
    uint32_t m_lastRampMs;

    uint16_t angleToPulse(uint8_t jointIndex, float angle);
};

#endif // PCA9685_SERVO_DRIVER_H
