#ifndef SERVO_DRIVER_H
#define SERVO_DRIVER_H

#include "IServoDriver.h"
#include "ESP32DirectServoDriver.h"
#include "PCA9685ServoDriver.h"

class ServoDriver : public IServoDriver {
public:
    ServoDriver();
    virtual ~ServoDriver();

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

    virtual const ServoConfig& getConfig() const override;
    virtual void updateConfig(const ServoConfig& config) override;

    virtual bool isHardwareAttached() const override;
    virtual ServoBackendType getBackendType() const override;

    void setBackend(ServoBackendType backend);

private:
    IServoDriver* m_activeBackend;
    ESP32DirectServoDriver m_directDriver;
    PCA9685ServoDriver     m_pcaDriver;
    ServoConfig m_config;
};

#endif // SERVO_DRIVER_H
