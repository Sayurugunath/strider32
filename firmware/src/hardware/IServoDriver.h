#ifndef ISERVO_DRIVER_H
#define ISERVO_DRIVER_H

#include "robot_types.h"
#include "pin_definitions.h"

class IServoDriver {
public:
    virtual ~IServoDriver() {}

    // Lifecycle Management
    virtual bool begin() = 0;
    virtual void update() = 0; // Soft-start ramping task update (non-blocking)

    // Servo Target Controls
    virtual void setServoAngle(uint8_t jointIndex, float angle) = 0;
    virtual float getServoAngle(uint8_t jointIndex) const = 0;
    
    // Calibration & Limits
    virtual void setServoOffset(uint8_t jointIndex, int16_t offset) = 0;
    virtual int16_t getServoOffset(uint8_t jointIndex) const = 0;
    
    virtual void setServoLimits(uint8_t jointIndex, uint8_t minAngle, uint8_t maxAngle) = 0;
    virtual void setServoInverted(uint8_t jointIndex, bool inverted) = 0;
    
    virtual void setAllAngles(const float angles[NUM_SERVOS]) = 0;
    
    // Safety Cutoff
    virtual void emergencyDisableAll() = 0;

    // Config Registry
    virtual const ServoConfig& getConfig() const = 0;
    virtual void updateConfig(const ServoConfig& config) = 0;

    virtual bool isHardwareAttached() const = 0;
    virtual ServoBackendType getBackendType() const = 0;
};

#endif // ISERVO_DRIVER_H
