#ifndef SAFETY_SYSTEM_H
#define SAFETY_SYSTEM_H

#include "robot_types.h"

class ServoDriver;

class SafetySystem {
public:
    SafetySystem();
    void begin(ServoDriver* driver);
    void update();
    
    void triggerEmergencyStop(const char* reason = "User Request");
    bool resetEmergencyStop(const char** outMessage = nullptr);
    
    bool isEmergencyStopActive() const { return m_estopActive; }
    bool canExecuteMotion() const { return !m_estopActive && (m_state == SafetyState::READY || m_state == SafetyState::RUNNING); }
    
    SafetyState getSafetyState() const { return m_state; }
    void setRunningState(bool running);

    const char* getLastErrorReason() const { return m_lastReason; }

private:
    ServoDriver* m_driver;
    SafetyState  m_state;
    bool         m_estopActive;
    char         m_lastReason[64];
    uint32_t     m_lastUpdateMs;
};

#endif // SAFETY_SYSTEM_H
