#ifndef ANIMATION_PLAYER_H
#define ANIMATION_PLAYER_H

#include "robot_types.h"
#include <ArduinoJson.h>

class ServoDriver;

struct Keyframe {
    float angles[NUM_SERVOS];
    uint16_t durationMs;
};

class AnimationPlayer {
public:
    AnimationPlayer();
    void begin(ServoDriver* driver);
    void update();

    bool loadFromJson(const char* jsonStr);
    void play(bool loop = false);
    void pause();
    void stop();

    bool isPlaying() const { return m_playing; }
    uint8_t getCurrentFrameIndex() const { return m_currentFrame; }

private:
    ServoDriver* m_driver;
    Keyframe m_frames[16]; // Up to 16 keyframes per animation sequence
    uint8_t m_frameCount;
    uint8_t m_currentFrame;
    bool m_playing;
    bool m_loop;
    
    uint32_t m_frameStartMs;
    float m_startAngles[NUM_SERVOS];
};

#endif // ANIMATION_PLAYER_H
