#include "GaitEngine.h"
#include "hardware/ServoDriver.h"

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

    // Return to neutral safe standing pose when stopped
    if (m_driver) {
        float neutralAngles[NUM_SERVOS];
        for (uint8_t i = 0; i < NUM_SERVOS; i++) neutralAngles[i] = 90.0f;
        m_driver->setAllAngles(neutralAngles);
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

    float fwdSwing = 25.0f * m_cmdForward;
    float latSwing = 15.0f * m_cmdLateral;
    float turnBias = 15.0f * m_cmdTurn;
    float liftHeight = 20.0f;

    float angles[NUM_SERVOS];

    // Neutral baseline = 90° for all joints
    for (uint8_t i = 0; i < NUM_SERVOS; i++) angles[i] = 90.0f;

    // --- PAIR A: Front-Left (FL) & Back-Right (BR) ---
    // FL (Leg 0)
    angles[JOINT_FL_COXA]  += (sinPhaseA * fwdSwing) + latSwing + turnBias;
    angles[JOINT_FL_FEMUR] += (sinPhaseA > 0 ? sinPhaseA * liftHeight : 0.0f);

    // BR (Leg 3)
    angles[JOINT_BR_COXA]  += (sinPhaseA * fwdSwing) - latSwing + turnBias;
    angles[JOINT_BR_FEMUR] += (sinPhaseA > 0 ? sinPhaseA * liftHeight : 0.0f);

    // --- PAIR B: Front-Right (FR) & Back-Left (BL) ---
    // FR (Leg 1)
    angles[JOINT_FR_COXA]  += (sinPhaseB * fwdSwing) - latSwing - turnBias;
    angles[JOINT_FR_FEMUR] += (sinPhaseB > 0 ? sinPhaseB * liftHeight : 0.0f);

    // BL (Leg 2)
    angles[JOINT_BL_COXA]  += (sinPhaseB * fwdSwing) + latSwing - turnBias;
    angles[JOINT_BL_FEMUR] += (sinPhaseB > 0 ? sinPhaseB * liftHeight : 0.0f);

    m_driver->setAllAngles(angles);
}

void GaitEngine::updateCrawlGait(float dt) {
    // 4-Phase Static Stability Crawl Gait
    // Phase 0: FL swings (FR, BL, BR stance)
    // Phase 1: BR swings (FL, FR, BL stance)
    // Phase 2: FR swings (FL, BL, BR stance)
    // Phase 3: BL swings (FL, FR, BR stance)

    float frequency = 0.4f + (m_speed / 100.0f) * 0.6f; // 0.4 Hz to 1.0 Hz
    m_phase += frequency * dt;
    if (m_phase >= 1.0f) m_phase -= 1.0f;

    float legPhase = m_phase * 4.0f; // Range 0.0 to 4.0
    uint8_t activeSwingLeg = (uint8_t)legPhase; // 0, 1, 2, or 3
    float subPhase = legPhase - activeSwingLeg; // 0.0 to 1.0 within active leg swing phase

    float fwdSwing   = 20.0f * m_cmdForward;
    float latSwing   = 12.0f * m_cmdLateral;
    float turnBias   = 12.0f * m_cmdTurn;
    float liftHeight = 22.0f;

    float angles[NUM_SERVOS];
    for (uint8_t i = 0; i < NUM_SERVOS; i++) angles[i] = 90.0f;

    // Mapping active swing leg order: 0=FL, 1=BR, 2=FR, 3=BL
    const uint8_t swingOrder[4] = { LEG_FL, LEG_BR, LEG_FR, LEG_BL };

    for (uint8_t i = 0; i < NUM_LEGS; i++) {
        uint8_t coxaIdx  = i * 2;
        uint8_t femurIdx = i * 2 + 1;

        if (swingOrder[activeSwingLeg] == i) {
            // SWING PHASE: Lift leg and move forward
            float swingLift = sinf(subPhase * M_PI) * liftHeight;
            float swingAdvance = (subPhase - 0.5f) * 2.0f * fwdSwing;

            angles[femurIdx] += swingLift;
            angles[coxaIdx]  += swingAdvance;
        } else {
            // STANCE PHASE: Foot remains on ground and propels body
            float stancePropel = -0.33f * (subPhase - 0.5f) * 2.0f * fwdSwing;
            angles[coxaIdx] += stancePropel;
        }

        // Apply lateral and rotation offsets
        if (i == LEG_FL || i == LEG_BL) {
            angles[coxaIdx] += latSwing;
        } else {
            angles[coxaIdx] -= latSwing;
        }

        if (i == LEG_FL || i == LEG_BR) {
            angles[coxaIdx] += turnBias;
        } else {
            angles[coxaIdx] -= turnBias;
        }
    }

    m_driver->setAllAngles(angles);
}
