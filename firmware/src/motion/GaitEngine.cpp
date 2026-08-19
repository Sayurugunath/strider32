#include "GaitEngine.h"
#include "Kinematics.h"
#include "hardware/ServoDriver.h"
#include <cmath>

GaitEngine::GaitEngine()
    : m_driver(nullptr), m_activeGait(GaitType::TROT), m_speed(50), 
      m_phase(0.0f), m_lastUpdateMs(0), m_moving(false),
      m_cmdForward(0.0f), m_cmdLateral(0.0f), m_cmdTurn(0.0f) {}

void GaitEngine::begin(ServoDriver* driver) {
    m_driver = driver;
    m_lastUpdateMs = millis();
}

void GaitEngine::setGait(GaitType gait) {
    m_activeGait = gait;
    m_phase = 0.0f;
}

void GaitEngine::setSpeed(uint8_t speed) {
    m_speed = constrain(speed, 10, 100);
}

void GaitEngine::moveDirection(float forwardBackward, float leftRight, float turn) {
    m_cmdForward = constrain(forwardBackward, -1.0f, 1.0f);
    m_cmdLateral = constrain(leftRight, -1.0f, 1.0f);
    m_cmdTurn    = constrain(turn, -1.0f, 1.0f);
    m_moving     = (abs(m_cmdForward) > 0.05f || abs(m_cmdLateral) > 0.05f || abs(m_cmdTurn) > 0.05f);
}

void GaitEngine::stop() {
    m_cmdForward = 0.0f;
    m_cmdLateral = 0.0f;
    m_cmdTurn    = 0.0f;
    m_moving     = false;

    // Return to analytical stand pose using IK when stopped
    if (m_driver) {
        float standAngles[NUM_SERVOS];
        Kinematics::getPoseAngles(RobotPose::STAND, standAngles);
        m_driver->setAllAngles(standAngles);
    }
}

void GaitEngine::update() {
    uint32_t now = millis();
    float dt = (now - m_lastUpdateMs) / 1000.0f;
    m_lastUpdateMs = now;

    if (!m_moving || m_activeGait == GaitType::IDLE || !m_driver) return;

    switch (m_activeGait) {
        case GaitType::TROT:
            updateTrotGait(dt);
            break;
        case GaitType::CRAWL:
            updateCrawlGait(dt);
            break;
        default:
            updateTrotGait(dt);
            break;
    }
}

void GaitEngine::updateTrotGait(float dt) {
    // Frequency scales with m_speed setting (1.0 Hz to 2.5 Hz)
    float frequency = 1.0f + (m_speed / 100.0f) * 1.5f;
    m_phase += frequency * dt;
    if (m_phase >= 1.0f) m_phase -= 1.0f;

    // Trot gait alternates diagonal leg pairs (Pair A: FL & BR, Pair B: FR & BL)
    float sinPhaseA = sinf(m_phase * 2.0f * M_PI);
    float sinPhaseB = sinf((m_phase + 0.5f) * 2.0f * M_PI);

    // Baseline Cartesian ground stance: x=60mm, y=0mm, z=-40mm
    const float baseX = 60.0f;
    const float baseZ = -40.0f;

    float fwdStride = 20.0f * m_cmdForward;
    float latStride = 12.0f * m_cmdLateral;
    float turnStride = 12.0f * m_cmdTurn;
    float liftHeight = 18.0f;

    float angles[NUM_SERVOS];

    // Compute Pair A (FL & BR) foot positions in 3D Cartesian coordinates
    float xA = baseX + (sinPhaseA * fwdStride);
    float yA_FL = (sinPhaseA * latStride) + turnStride;
    float yA_BR = (-sinPhaseA * latStride) + turnStride;
    float zA = baseZ + (sinPhaseA > 0 ? sinPhaseA * liftHeight : 0.0f);

    // Compute Pair B (FR & BL) foot positions in 3D Cartesian coordinates
    float xB = baseX + (sinPhaseB * fwdStride);
    float yB_FR = (-sinPhaseB * latStride) - turnStride;
    float yB_BL = (sinPhaseB * latStride) - turnStride;
    float zB = baseZ + (sinPhaseB > 0 ? sinPhaseB * liftHeight : 0.0f);

    // Solve Analytical 3D IK for each leg
    Kinematics::solveLegIK(xA, yA_FL, zA, angles[JOINT_FL_COXA], angles[JOINT_FL_FEMUR]);
    Kinematics::solveLegIK(xB, yB_FR, zB, angles[JOINT_FR_COXA], angles[JOINT_FR_FEMUR]);
    Kinematics::solveLegIK(xB, yB_BL, zB, angles[JOINT_BL_COXA], angles[JOINT_BL_FEMUR]);
    Kinematics::solveLegIK(xA, yA_BR, zA, angles[JOINT_BR_COXA], angles[JOINT_BR_FEMUR]);

    m_driver->setAllAngles(angles);
}

void GaitEngine::updateCrawlGait(float dt) {
    // 4-Phase Static Stability Crawl Gait driven by 3D Cartesian IK
    float frequency = 0.4f + (m_speed / 100.0f) * 0.6f; // 0.4 Hz to 1.0 Hz
    m_phase += frequency * dt;
    if (m_phase >= 1.0f) m_phase -= 1.0f;

    float legPhase = m_phase * 4.0f; // Range 0.0 to 4.0
    uint8_t activeSwingLeg = (uint8_t)legPhase; // 0, 1, 2, or 3
    float subPhase = legPhase - activeSwingLeg; // 0.0 to 1.0 within active leg swing phase

    const float baseX = 60.0f;
    const float baseZ = -40.0f;

    float fwdStride  = 18.0f * m_cmdForward;
    float latStride  = 10.0f * m_cmdLateral;
    float turnStride = 10.0f * m_cmdTurn;
    float liftHeight = 18.0f;

    float angles[NUM_SERVOS];
    const uint8_t swingOrder[4] = { LEG_FL, LEG_BR, LEG_FR, LEG_BL };

    for (uint8_t i = 0; i < NUM_LEGS; i++) {
        float x = baseX;
        float y = 0.0f;
        float z = baseZ;

        if (swingOrder[activeSwingLeg] == i) {
            // SWING PHASE: Parabolic Lift and forward step
            z += sinf(subPhase * M_PI) * liftHeight;
            x += (subPhase - 0.5f) * 2.0f * fwdStride;
        } else {
            // STANCE PHASE: Propel body backward along ground plane
            x += -0.33f * (subPhase - 0.5f) * 2.0f * fwdStride;
        }

        // Apply lateral and rotation offsets
        if (i == LEG_FL || i == LEG_BL) {
            y += latStride;
        } else {
            y -= latStride;
        }

        if (i == LEG_FL || i == LEG_BR) {
            y += turnStride;
        } else {
            y -= turnStride;
        }

        uint8_t coxaIdx  = i * 2;
        uint8_t femurIdx = i * 2 + 1;
        Kinematics::solveLegIK(x, y, z, angles[coxaIdx], angles[femurIdx]);
    }

    m_driver->setAllAngles(angles);
}
