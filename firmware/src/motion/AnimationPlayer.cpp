#include "AnimationPlayer.h"
#include "hardware/ServoDriver.h"
#include "Kinematics.h"

AnimationPlayer::AnimationPlayer() 
    : m_driver(nullptr), m_frameCount(0), m_currentFrame(0), 
      m_playing(false), m_loop(false), m_frameStartMs(0) {}

void AnimationPlayer::begin(ServoDriver* driver) {
    m_driver = driver;
}

bool AnimationPlayer::loadFromJson(const char* jsonStr) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    if (error) return false;

    JsonArray framesArray = doc["frames"].as<JsonArray>();
    m_frameCount = 0;

    for (JsonObject frameObj : framesArray) {
        if (m_frameCount >= 16) break;

        JsonArray anglesArr = frameObj["angles"];
        for (uint8_t i = 0; i < NUM_SERVOS && i < anglesArr.size(); i++) {
            m_frames[m_frameCount].angles[i] = anglesArr[i].as<float>();
        }

        m_frames[m_frameCount].durationMs = frameObj["duration"].as<uint16_t>();
        if (m_frames[m_frameCount].durationMs == 0) m_frames[m_frameCount].durationMs = 500;

        m_frameCount++;
    }

    return (m_frameCount > 0);
}

void AnimationPlayer::play(bool loop) {
    if (m_frameCount == 0 || !m_driver) return;
    
    m_loop = loop;
    m_currentFrame = 0;
    m_playing = true;
    m_frameStartMs = millis();

    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        m_startAngles[i] = m_driver->getServoAngle(i);
    }
}

void AnimationPlayer::pause() {
    m_playing = false;
}

void AnimationPlayer::stop() {
    m_playing = false;
    m_currentFrame = 0;
}

void AnimationPlayer::update() {
    if (!m_playing || m_frameCount == 0 || !m_driver) return;

    uint32_t now = millis();
    uint32_t elapsed = now - m_frameStartMs;
    uint16_t frameDuration = m_frames[m_currentFrame].durationMs;

    float progress = (float)elapsed / (float)frameDuration;

    if (progress >= 1.0f) {
        // Apply final frame angle exactly
        m_driver->setAllAngles(m_frames[m_currentFrame].angles);

        // Advance to next keyframe
        m_currentFrame++;
        if (m_currentFrame >= m_frameCount) {
            if (m_loop) {
                m_currentFrame = 0;
            } else {
                m_playing = false;
                return;
            }
        }

        // Prepare next frame interpolation start
        m_frameStartMs = now;
        for (uint8_t i = 0; i < NUM_SERVOS; i++) {
            m_startAngles[i] = m_driver->getServoAngle(i);
        }
    } else {
        // Smooth linear interpolation between startAngles and current frame target angles
        float currentAngles[NUM_SERVOS];
        Kinematics::interpolatePoses(m_startAngles, m_frames[m_currentFrame].angles, progress, currentAngles);
        m_driver->setAllAngles(currentAngles);
    }
}
