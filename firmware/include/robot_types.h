#ifndef ROBOT_TYPES_H
#define ROBOT_TYPES_H

#include <Arduino.h>

// Number of joints (2 joints per leg x 4 legs = 8)
constexpr uint8_t NUM_SERVOS = 8;
constexpr uint8_t NUM_LEGS = 4;

// Conservative Default Servo Pulse Width Range (in microseconds)
constexpr uint16_t DEFAULT_PULSE_MIN_US = 600;  // Conservative 0 degrees (0.6 ms)
constexpr uint16_t DEFAULT_PULSE_MAX_US = 2400; // Conservative 180 degrees (2.4 ms)

// Hardware Servo Driver Backend Selection
enum class ServoBackendType : uint8_t {
    ESP32_DIRECT_LEDC = 0, // Default Primary: Direct ESP32 GPIO PWM via LEDC
    PCA9685_I2C       = 1  // Optional Secondary: PCA9685 16-channel I2C module
};

// Servo Joint Indices
enum ServoJoint : uint8_t {
    JOINT_FL_COXA  = 0, // Front-Left Hip
    JOINT_FL_FEMUR = 1, // Front-Left Knee
    JOINT_FR_COXA  = 2, // Front-Right Hip
    JOINT_FR_FEMUR = 3, // Front-Right Knee
    JOINT_BL_COXA  = 4, // Back-Left Hip
    JOINT_BL_FEMUR = 5, // Back-Left Knee
    JOINT_BR_COXA  = 6, // Back-Right Hip
    JOINT_BR_FEMUR = 7  // Back-Right Knee
};

// Leg Indices
enum LegIndex : uint8_t {
    LEG_FL = 0,
    LEG_FR = 1,
    LEG_BL = 2,
    LEG_BR = 3
};

// Robot Poses
enum class RobotPose : uint8_t {
    STAND,
    REST,
    SIT,
    WAVE,
    DANCE,
    CUSTOM
};

// Gait Types
enum class GaitType : uint8_t {
    TROT,
    CRAWL,
    WAVE,
    IDLE
};

// Software Safety States
enum class SafetyState : uint8_t {
    READY,          // Initialized, stationary, ready to receive motion
    RUNNING,        // Actively moving / executing gait
    EMERGENCY_STOP, // Latched emergency stop (PWM output disabled)
    FAULT           // Hardware fault state
};

// Servo Calibration, Pulse Width & Boundary Configuration
struct ServoConfig {
    int16_t  offsets[NUM_SERVOS];
    uint8_t  limits_min[NUM_SERVOS];
    uint8_t  limits_max[NUM_SERVOS];
    bool     inverted[NUM_SERVOS];
    uint16_t pulse_min_us; // Min pulse width (conservative default: 600 us)
    uint16_t pulse_max_us; // Max pulse width (conservative default: 2400 us)
    ServoBackendType backend;
};

// Network Configuration Model
struct NetworkConfig {
    char ap_ssid[32];
    char ap_password[64];
    char sta_ssid[32];
    char sta_password[64];
    char hostname[32];
    bool ap_mode;
};

// Robot Operational Configuration
struct RobotConfig {
    char robot_name[32];
    uint8_t default_speed;
    GaitType default_gait;
    RobotPose startup_pose;
};

// Robot System Status
struct SystemStatus {
    uint32_t uptime_sec;
    uint32_t free_heap;
    float battery_voltage;
    bool estop_active;
    RobotPose active_pose;
    GaitType active_gait;
    int8_t wifi_rssi;
    char wifi_ip[16];
};

#endif // ROBOT_TYPES_H
