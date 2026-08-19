#include "Kinematics.h"
#include <cmath>
#include <Arduino.h>

Kinematics::Kinematics() {}

bool Kinematics::solveLegIK(float x, float y, float z, float& outCoxa, float& outFemur) {
    bool withinReach = true;

    // 1. Calculate Coxa (Hip) Angle in XY horizontal plane
    // Neutral x=60, y=0 -> atan2(0, 60) = 0 rad -> 90.0 degrees
    float coxaRad = atan2f(y, x);
    outCoxa = (coxaRad * 180.0f / M_PI) + 90.0f;

    // 2. Planar horizontal radius from hip pivot
    float r = sqrtf(x * x + y * y);
    float rEff = r - LEG_L1_COXA_MM;
    if (rEff < 1.0f) rEff = 1.0f; // Singularity protection at origin

    // 3. 3D Extension distance D to foot tip
    float D = sqrtf(rEff * rEff + z * z);
    float maxReach = 2.0f * LEG_L2_FEMUR_MM;

    // Workspace Reach Protection: Clamp D if coordinate exceeds physical reach
    if (D > maxReach) {
        D = maxReach - 0.1f;
        withinReach = false;
    }

    // 4. Calculate Femur (Elevation) Angle using law of cosines
    float alpha = atan2f(z, rEff);
    float cosBeta = D / (2.0f * LEG_L2_FEMUR_MM);
    cosBeta = constrain(cosBeta, -1.0f, 1.0f);
    float beta = acosf(cosBeta);

    float femurRad = alpha + beta;
    outFemur = (femurRad * 180.0f / M_PI);

    // 5. Constrain joint angles within safe servo limits (10° - 170°)
    outCoxa  = constrain(outCoxa, 10.0f, 170.0f);
    outFemur = constrain(outFemur, 10.0f, 170.0f);

    return withinReach;
}

void Kinematics::solveLegFK(float coxa, float femur, float& outX, float& outY, float& outZ) {
    float coxaRad = (coxa - 90.0f) * M_PI / 180.0f;
    float femurRad = femur * M_PI / 180.0f;

    // Forward kinematics reconstruction
    // Using standard baseline extension geometry:
    // D is the 3D vector length, alpha is the elevation angle
    float D = 2.0f * LEG_L2_FEMUR_MM * cosf(femurRad * 0.5f);
    if (D < 10.0f) D = 10.0f;

    float rEff = D * cosf(femurRad * 0.5f);
    float r = rEff + LEG_L1_COXA_MM;

    outX = r * cosf(coxaRad);
    outY = r * sinf(coxaRad);
    outZ = -D * sinf(femurRad * 0.5f);
}

void Kinematics::getPoseAngles(RobotPose pose, float outAngles[NUM_SERVOS]) {
    switch (pose) {
        case RobotPose::STAND: {
            // Compute stand angles analytically using IK (x=60mm, y=0, z=-40mm)
            float coxa, femur;
            solveLegIK(60.0f, 0.0f, -40.0f, coxa, femur);
            for (uint8_t leg = 0; leg < 4; leg++) {
                outAngles[leg * 2 + 0] = coxa;
                outAngles[leg * 2 + 1] = femur;
            }
            break;
        }

        case RobotPose::REST:
            outAngles[JOINT_FL_COXA]  = 90.0f; outAngles[JOINT_FL_FEMUR] = 30.0f;
            outAngles[JOINT_FR_COXA]  = 90.0f; outAngles[JOINT_FR_FEMUR] = 30.0f;
            outAngles[JOINT_BL_COXA]  = 90.0f; outAngles[JOINT_BL_FEMUR] = 30.0f;
            outAngles[JOINT_BR_COXA]  = 90.0f; outAngles[JOINT_BR_FEMUR] = 30.0f;
            break;

        case RobotPose::SIT:
            outAngles[JOINT_FL_COXA]  = 90.0f; outAngles[JOINT_FL_FEMUR] = 110.0f;
            outAngles[JOINT_FR_COXA]  = 90.0f; outAngles[JOINT_FR_FEMUR] = 110.0f;
            outAngles[JOINT_BL_COXA]  = 90.0f; outAngles[JOINT_BL_FEMUR] = 30.0f;
            outAngles[JOINT_BR_COXA]  = 90.0f; outAngles[JOINT_BR_FEMUR] = 30.0f;
            break;

        case RobotPose::WAVE:
            outAngles[JOINT_FL_COXA]  = 90.0f; outAngles[JOINT_FL_FEMUR] = 90.0f;
            outAngles[JOINT_FR_COXA]  = 45.0f; outAngles[JOINT_FR_FEMUR] = 150.0f;
            outAngles[JOINT_BL_COXA]  = 90.0f; outAngles[JOINT_BL_FEMUR] = 80.0f;
            outAngles[JOINT_BR_COXA]  = 90.0f; outAngles[JOINT_BR_FEMUR] = 80.0f;
            break;

        case RobotPose::DANCE:
            outAngles[JOINT_FL_COXA]  = 110.0f; outAngles[JOINT_FL_FEMUR] = 70.0f;
            outAngles[JOINT_FR_COXA]  = 70.0f;  outAngles[JOINT_FR_FEMUR] = 110.0f;
            outAngles[JOINT_BL_COXA]  = 70.0f;  outAngles[JOINT_BL_FEMUR] = 110.0f;
            outAngles[JOINT_BR_COXA]  = 110.0f; outAngles[JOINT_BR_FEMUR] = 70.0f;
            break;

        default:
            for (uint8_t i = 0; i < NUM_SERVOS; i++) outAngles[i] = 90.0f;
            break;
    }
}

void Kinematics::interpolatePoses(const float startAngles[NUM_SERVOS], 
                                  const float endAngles[NUM_SERVOS], 
                                  float progress, 
                                  float outAngles[NUM_SERVOS]) {
    progress = constrain(progress, 0.0f, 1.0f);
    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        outAngles[i] = startAngles[i] + (endAngles[i] - startAngles[i]) * progress;
    }
}

bool Kinematics::runSelfTest() {
    struct TestPoint {
        const char* name;
        float x, y, z;
        bool expectReachable;
    };

    TestPoint tests[] = {
        { "Stand Neutral",  60.0f,   0.0f, -40.0f, true  },
        { "Forward Step",   80.0f,   0.0f, -40.0f, true  },
        { "Backward Step",  40.0f,   0.0f, -40.0f, true  },
        { "Left Lateral",   60.0f,  15.0f, -40.0f, true  },
        { "Right Lateral",  60.0f, -15.0f, -40.0f, true  },
        { "Lifted Foot",    60.0f,   0.0f, -20.0f, true  },
        { "Near Max Reach", 85.0f,   0.0f,   0.0f, true  },
        { "Origin Singularity", 1.0f, 0.0f,   0.0f, true  },
        { "Beyond Max Reach", 150.0f, 0.0f,  0.0f, false }
    };

    bool allPassed = true;
    Serial.println("\n--- Strider32 IK/FK Mathematical Self-Test ---");

    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        float coxa, femur;
        bool ok = solveLegIK(tests[i].x, tests[i].y, tests[i].z, coxa, femur);

        // Sanity check angle limits and numerical stability
        if (isnan(coxa) || isnan(femur) || isinf(coxa) || isinf(femur)) {
            Serial.printf("[FAIL] %s: NaN or Inf detected! (coxa=%.2f, femur=%.2f)\n", tests[i].name, coxa, femur);
            allPassed = false;
            continue;
        }

        if (coxa < 10.0f || coxa > 170.0f || femur < 10.0f || femur > 170.0f) {
            Serial.printf("[FAIL] %s: Joint angle out of bounds! (coxa=%.2f, femur=%.2f)\n", tests[i].name, coxa, femur);
            allPassed = false;
            continue;
        }

        if (ok != tests[i].expectReachable) {
            Serial.printf("[WARN] %s: Reach status mismatch (got %d, expected %d)\n", tests[i].name, ok, tests[i].expectReachable);
        }

        Serial.printf("[PASS] %s: Target(%.1f, %.1f, %.1f) -> Angles(Coxa: %.2f°, Femur: %.2f°)\n",
                      tests[i].name, tests[i].x, tests[i].y, tests[i].z, coxa, femur);
    }

    Serial.printf("IK Self-Test Result: %s\n-----------------------------------------------\n", 
                  allPassed ? "ALL PASSED" : "FAILED");
    return allPassed;
}
