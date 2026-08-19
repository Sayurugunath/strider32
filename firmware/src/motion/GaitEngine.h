#ifndef GAIT_ENGINE_H
#define GAIT_ENGINE_H

#include "robot_types.h"

class ServoDriver;

class GaitEngine {
public:
    GaitEngine();
    void begin(ServoDriver* driver);
    void update();

    void setGait(GaitType gait);
    GaitType getGait() const { return m_activeGait; }

    void setSpeed(uint8_t speed); // 10 - 100%
    uint8_t getSpeed() const { return m_speed; }

    void moveDirection(float forwardBackward, float leftRight, float turn);
    void stop();
    bool isMoving() const { return m_moving; }

private:
    ServoDriver* m_driver;
    GaitType m_activeGait;
    uint8_t  m_speed;
    float    m_phase;
    uint32_t m_lastUpdateMs;
    bool     m_moving;

    float m_cmdForward;
    float m_cmdLateral;
    float m_cmdTurn;

    void updateTrotGait(float dt);
    void updateCrawlGait(float dt);
};

#endif // GAIT_ENGINE_H
