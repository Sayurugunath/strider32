#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "robot_types.h"
#include "config_keys.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

class ConfigManager {
public:
    ConfigManager();
    bool begin();

    bool loadServoConfig(ServoConfig& config);
    bool saveServoConfig(const ServoConfig& config);

    bool loadNetworkConfig(NetworkConfig& config);
    bool saveNetworkConfig(const NetworkConfig& config);

    bool loadRobotConfig(RobotConfig& config);
    bool saveRobotConfig(const RobotConfig& config);

    // Animation LittleFS Persistence Helpers
    String listAnimationsJson();
    bool saveAnimationJson(const char* animId, const String& jsonContent, String& outError);
    bool loadAnimationJson(const char* animId, String& outJson);
    bool deleteAnimationJson(const char* animId);

    void resetToDefaults();

private:
    bool m_fsMounted;
    String sanitizeFilename(const char* animId);
};

#endif // CONFIG_MANAGER_H
