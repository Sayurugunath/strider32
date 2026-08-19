#include "Kinematics.h"

Kinematics::Kinematics() {}

void Kinematics::getPoseAngles(RobotPose pose, float outAngles[NUM_SERVOS]) {
    switch (pose) {
        case RobotPose::STAND:
            // All legs extended in stable standing posture (Coxa 90°, Femur 90°)
            for (uint8_t i = 0; i < NUM_SERVOS; i++) outAngles[i] = 90.0f;
            break;

        case RobotPose::REST:
            // Body resting on chassis floor (Femur folded to 30°)
            outAngles[JOINT_FL_COXA]  = 90.0f; outAngles[JOINT_FL_FEMUR] = 30.0f;
            outAngles[JOINT_FR_COXA]  = 90.0f; outAngles[JOINT_FR_FEMUR] = 30.0f;
            outAngles[JOINT_BL_COXA]  = 90.0f; outAngles[JOINT_BL_FEMUR] = 30.0f;
            outAngles[JOINT_BR_COXA]  = 90.0f; outAngles[JOINT_BR_FEMUR] = 30.0f;
            break;

        case RobotPose::SIT:
            // Back legs crouched, front legs upright
            outAngles[JOINT_FL_COXA]  = 90.0f; outAngles[JOINT_FL_FEMUR] = 110.0f;
            outAngles[JOINT_FR_COXA]  = 90.0f; outAngles[JOINT_FR_FEMUR] = 110.0f;
            outAngles[JOINT_BL_COXA]  = 90.0f; outAngles[JOINT_BL_FEMUR] = 30.0f;
            outAngles[JOINT_BR_COXA]  = 90.0f; outAngles[JOINT_BR_FEMUR] = 30.0f;
            break;

        case RobotPose::WAVE:
            // Front-right leg raised to wave
            outAngles[JOINT_FL_COXA]  = 90.0f; outAngles[JOINT_FL_FEMUR] = 90.0f;
            outAngles[JOINT_FR_COXA]  = 45.0f; outAngles[JOINT_FR_FEMUR] = 150.0f; // Raised leg
            outAngles[JOINT_BL_COXA]  = 90.0f; outAngles[JOINT_BL_FEMUR] = 80.0f;
            outAngles[JOINT_BR_COXA]  = 90.0f; outAngles[JOINT_BR_FEMUR] = 80.0f;
            break;

        case RobotPose::DANCE:
            // Playful tilt stance
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
