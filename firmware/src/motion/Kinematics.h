#ifndef KINEMATICS_H
#define KINEMATICS_H

#include "robot_types.h"

// Structural Link Dimensions (in millimeters)
constexpr float LEG_L1_COXA_MM  = 30.0f; // Hip offset / Coxa link length
constexpr float LEG_L2_FEMUR_MM = 60.0f; // Thigh / Femur link length

struct Point3D {
    float x;
    float y;
    float z;
};

class Kinematics {
public:
    Kinematics();

    // Analytical 3D Inverse Kinematics solver for 1 leg
    // Input: Target Cartesian coordinate (x, y, z) in mm relative to shoulder origin
    // Output: Out angle coxa (0-180°), Out angle femur (0-180°)
    // Returns: true if coordinate within reach, false if clamped to physical workspace boundary
    static bool solveLegIK(float x, float y, float z, float& outCoxa, float& outFemur);

    // Analytical Forward Kinematics solver for 1 leg
    // Input: Joint angles coxa, femur (degrees)
    // Output: Out 3D Cartesian position (x, y, z) in mm
    static void solveLegFK(float coxa, float femur, float& outX, float& outY, float& outZ);

    // Joint poses for defined standard states
    static void getPoseAngles(RobotPose pose, float outAngles[NUM_SERVOS]);

    // Calculate leg joint angles for smooth pose transitions
    static void interpolatePoses(const float startAngles[NUM_SERVOS], 
                                 const float endAngles[NUM_SERVOS], 
                                 float progress, 
                                 float outAngles[NUM_SERVOS]);

    // Automated mathematical self-test harness for IK/FK validation
    // Returns true if all test points achieve < 1.0mm error and boundary protection succeeds
    static bool runSelfTest();
};

#endif // KINEMATICS_H
