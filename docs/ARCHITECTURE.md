# Quadruped Robotics Architecture Guide

## Overview
This platform features a decoupled, multi-layered software architecture for ESP32-based 8-DOF quadruped robots.

```
┌─────────────────────────────────────────────────────────────────┐
│                    Web Control Center (Web UI)                  │
│       Dashboard | Motion | Calibration | Animation | Diag       │
└────────────────────────────────┬────────────────────────────────┘
                                 │ HTTP REST / WebSocket
┌────────────────────────────────▼────────────────────────────────┐
│                          API Layer                              │
│              ApiRoutes.cpp  |  JsonFormatter.cpp                │
└───────┬────────────────────────┬────────────────────────┬───────┘
        │                        │                        │
┌───────▼────────┐      ┌────────▼────────┐      ┌────────▼───────┐
│  Network Layer │      │   Config Layer  │      │ Diagnostic Lyr │
│ NetworkManager │      │ ConfigManager   │      │ System metrics │
└────────────────┘      └────────┬────────┘      └────────────────┘
                                 │
                        ┌────────▼────────┐
                        │   Motion Core   │
                        │ GaitEngine      │
                        │ Kinematics      │
                        │ AnimationPlayer │
                        └────────┬────────┘
                                 │
                        ┌────────▼────────┐
                        │  Hardware Layer │
                        │ ServoDriver     │
                        │ SafetySystem    │
                        └─────────────────┘
```

## Module Responsibilities

### 1. Hardware Layer (`firmware/src/hardware`)
- **`ServoDriver.h/.cpp`**: Interfacing with PCA9685 I2C servo controller or direct ESP32 LEDC PWM. Performs microsecond pulse calculation and software angle limits clamping.
- **`SafetySystem.h/.cpp`**: Emergency stop (E-STOP) state machine, brownout detection, soft-start ramp rate calculation, and watchdog timers.

### 2. Motion Core (`firmware/src/motion`)
- **`Kinematics.h/.cpp`**: Inverse and forward kinematics for 2-DOF legs (Coxa/Hip pitch & Femur/Knee pitch).
- **`GaitEngine.h/.cpp`**: Procedural gait generation (Trot gait, Crawl gait, Wave gait) with step height, frequency, and stance duty cycle controls.
- **`AnimationPlayer.h/.cpp`**: Keyframe timeline parser and playback engine supporting interpolated frames, custom durations, easing functions, and looping.

### 3. Config Layer (`firmware/src/config`)
- **`ConfigManager.h/.cpp`**: Central configuration registry interfacing with LittleFS storage. Manages JSON files for servos, network, robot settings, and custom keyframes.

### 4. Network Layer (`firmware/src/network`)
- **`NetworkManager.h/.cpp`**: Dual Wi-Fi mode management (AP & Station), auto-reconnect fallback logic, network scanning, and mDNS domain registration (`http://quadrobot.local`).

### 5. API Layer (`firmware/src/api`)
- **`ApiRoutes.h/.cpp`**: Asynchronous HTTP endpoint handlers (`/api/v1/*`) returning structured JSON payloads (`success`, `data`, `error`).
