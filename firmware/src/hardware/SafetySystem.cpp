#include "SafetySystem.h"
#include "ServoDriver.h"

SafetySystem::SafetySystem() 
    : m_driver(nullptr), m_state(SafetyState::READY), m_estopActive(false), m_lastUpdateMs(0) {
    strncpy(m_lastReason, "None", sizeof(m_lastReason));
}

void SafetySystem::begin(ServoDriver* driver) {
    m_driver = driver;
    // PIN_ESTOP_BUTTON is GPIO 34 (Input-only GPI, requires 10k external pullup)
    pinMode(PIN_ESTOP_BUTTON, INPUT);
    m_estopActive = false;
    m_state = SafetyState::READY;
    strncpy(m_lastReason, "None", sizeof(m_lastReason));
}

void SafetySystem::update() {
    uint32_t now = millis();
    if (now - m_lastUpdateMs < 50) return; // Check every 50ms
    m_lastUpdateMs = now;

    // Read physical hardware E-STOP button (Active LOW when button pressed)
    if (digitalRead(PIN_ESTOP_BUTTON) == LOW) {
        if (!m_estopActive) {
            triggerEmergencyStop("Hardware E-STOP Button Pressed (GPIO 34)");
        }
    }
}

void SafetySystem::triggerEmergencyStop(const char* reason) {
    m_estopActive = true;
    m_state = SafetyState::EMERGENCY_STOP;
    if (reason) {
        strncpy(m_lastReason, reason, sizeof(m_lastReason) - 1);
        m_lastReason[sizeof(m_lastReason) - 1] = '\0';
    }
    
    // Immediately disable PWM output to all servos
    if (m_driver) {
        m_driver->emergencyDisableAll();
    }
}

bool SafetySystem::resetEmergencyStop(const char** outMessage) {
    // Check if physical hardware button is still being depressed
    if (digitalRead(PIN_ESTOP_BUTTON) == LOW) {
        if (outMessage) *outMessage = "Reset Rejected: Hardware E-STOP button is still physically pressed";
        return false;
    }

    m_estopActive = false;
    m_state = SafetyState::READY;
    strncpy(m_lastReason, "None", sizeof(m_lastReason));

    if (outMessage) *outMessage = "E-STOP Reset Successful. System state is now READY.";
    return true;
}

void SafetySystem::setRunningState(bool running) {
    if (m_estopActive || m_state == SafetyState::EMERGENCY_STOP) return;

    if (running) {
        m_state = SafetyState::RUNNING;
    } else {
        m_state = SafetyState::READY;
    }
}
