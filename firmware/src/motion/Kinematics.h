#ifndef KINEMATICS_H
#define KINEMATICS_H

#include "robot_types.h"

class Kinematics {
public:
    Kinematics();

    // Joint poses for defined standard states
    static void getPoseAngles(RobotPose pose, float outAngles[NUM_SERVOS]);

    // Calculate leg joint angles for smooth pose transitions
    static void interpolatePoses(const float startAngles[NUM_SERVOS], 
                                 const float endAngles[NUM_SERVOS], 
                                 float progress, 
                                 float outAngles[NUM_SERVOS]);
};

#endif // KINEMATICS_H
