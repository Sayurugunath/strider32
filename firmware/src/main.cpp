#include <Arduino.h>
#include "robot_types.h"
#include "hardware/ServoDriver.h"
#include "hardware/SafetySystem.h"
#include "motion/GaitEngine.h"
#include "motion/Kinematics.h"
#include "motion/AnimationPlayer.h"
#include "config/ConfigManager.h"
#include "network/NetworkManager.h"
#include "api/ApiRoutes.h"
#include <ESPAsyncWebServer.h>

// Global Subsystem Instances
ServoDriver     g_servoDriver;
SafetySystem    g_safetySystem;
GaitEngine      g_gaitEngine;
AnimationPlayer g_animPlayer;
ConfigManager   g_configMgr;
NetworkManager  g_networkMgr;

AsyncWebServer  g_webServer(80);

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- Initializing Strider32 Quadruped Robotics Platform (v0.2.0) ---");

    // 1. Run Automated IK/FK Mathematical Validation Self-Test
    Kinematics::runSelfTest();

    // 2. Initialize Persistent Config Storage
    g_configMgr.begin();

    // 3. Load Saved Hardware & Servo Calibration Config (Default backend: ESP32_DIRECT_LEDC)
    ServoConfig sCfg;
    g_configMgr.loadServoConfig(sCfg);
    g_servoDriver.updateConfig(sCfg);

    // 4. Initialize Servo Hardware Driver (Attaches GPIO PWM or PCA9685 I2C)
    g_servoDriver.begin();

    // 5. Initialize Safety System
    g_safetySystem.begin(&g_servoDriver);

    // 6. Initialize Motion Engine
    g_gaitEngine.begin(&g_servoDriver);
    g_animPlayer.begin(&g_servoDriver);

    // Initial Safe Neutral Stand Pose with soft-start ramping
    float standAngles[NUM_SERVOS];
    Kinematics::getPoseAngles(RobotPose::STAND, standAngles);
    g_servoDriver.setAllAngles(standAngles);

    // 7. Initialize Network Manager
    NetworkConfig netCfg;
    g_configMgr.loadNetworkConfig(netCfg);
    g_networkMgr.begin(netCfg);

    // 8. Register REST & Static Web Server API Routes
    ApiRoutes::registerRoutes(g_webServer, &g_servoDriver, &g_safetySystem, &g_gaitEngine, &g_animPlayer, &g_configMgr, &g_networkMgr);
    g_webServer.begin();

    Serial.println("Strider32 System Initialized Successfully.");
    Serial.print("Servo Driver Backend: ");
    Serial.println(g_servoDriver.getBackendType() == ServoBackendType::ESP32_DIRECT_LEDC ? "Direct ESP32 GPIO PWM" : "PCA9685 I2C Module");
    Serial.print("Web UI available at IP: ");
    Serial.println(g_networkMgr.getIPAddress());
}

void loop() {
    // Non-blocking task updates
    g_safetySystem.update();
    g_networkMgr.update();
    g_servoDriver.update(); // Smooth soft-start ramping service update

    if (!g_safetySystem.isEmergencyStopActive()) {
        g_gaitEngine.update();
        g_animPlayer.update();
    }

    delay(2);
}
