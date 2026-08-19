#include "ConfigManager.h"

ConfigManager::ConfigManager() : m_fsMounted(false) {}

bool ConfigManager::begin() {
    if (!LittleFS.begin(true)) {
        m_fsMounted = false;
        return false;
    }
    m_fsMounted = true;

    if (!LittleFS.exists("/config")) {
        LittleFS.mkdir("/config");
    }
    if (!LittleFS.exists(DIR_ANIMATIONS)) {
        LittleFS.mkdir(DIR_ANIMATIONS);
    }
    return true;
}

String ConfigManager::sanitizeFilename(const char* animId) {
    String name = String(animId);
    name.trim();
    name.toLowerCase();
    String clean = "";
    for (size_t i = 0; i < name.length(); i++) {
        char c = name.charAt(i);
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-') {
            clean += c;
        }
    }
    if (clean.length() == 0) clean = "anim_unnamed";
    return clean;
}

bool ConfigManager::loadServoConfig(ServoConfig& config) {
    config.backend = ServoBackendType::ESP32_DIRECT_LEDC;
    config.pulse_min_us = DEFAULT_PULSE_MIN_US;
    config.pulse_max_us = DEFAULT_PULSE_MAX_US;

    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        config.offsets[i] = 0;
        config.limits_min[i] = 10;
        config.limits_max[i] = 170;
        config.inverted[i] = false;
    }

    if (!m_fsMounted || !LittleFS.exists(FILE_SERVO_CONFIG)) return false;

    File file = LittleFS.open(FILE_SERVO_CONFIG, "r");
    if (!file) return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error) return false;

    if (!doc["backend"].isNull()) {
        config.backend = (ServoBackendType)doc["backend"].as<uint8_t>();
    }
    if (!doc["pulse_min_us"].isNull()) {
        config.pulse_min_us = doc["pulse_min_us"].as<uint16_t>();
    }
    if (!doc["pulse_max_us"].isNull()) {
        config.pulse_max_us = doc["pulse_max_us"].as<uint16_t>();
    }

    JsonArray offsetsArr = doc["offsets"];
    JsonArray minArr     = doc["limits_min"];
    JsonArray maxArr     = doc["limits_max"];
    JsonArray invArr     = doc["inverted"];

    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        if (i < offsetsArr.size()) config.offsets[i] = offsetsArr[i].as<int16_t>();
        if (i < minArr.size())     config.limits_min[i] = minArr[i].as<uint8_t>();
        if (i < maxArr.size())     config.limits_max[i] = maxArr[i].as<uint8_t>();
        if (i < invArr.size())     config.inverted[i] = invArr[i].as<bool>();
    }

    return true;
}

bool ConfigManager::saveServoConfig(const ServoConfig& config) {
    if (!m_fsMounted) return false;

    JsonDocument doc;
    doc["backend"] = (uint8_t)config.backend;
    doc["pulse_min_us"] = config.pulse_min_us;
    doc["pulse_max_us"] = config.pulse_max_us;

    JsonArray offsetsArr = doc["offsets"].to<JsonArray>();
    JsonArray minArr     = doc["limits_min"].to<JsonArray>();
    JsonArray maxArr     = doc["limits_max"].to<JsonArray>();
    JsonArray invArr     = doc["inverted"].to<JsonArray>();

    for (uint8_t i = 0; i < NUM_SERVOS; i++) {
        offsetsArr.add(config.offsets[i]);
        minArr.add(config.limits_min[i]);
        maxArr.add(config.limits_max[i]);
        invArr.add(config.inverted[i]);
    }

    File file = LittleFS.open(FILE_SERVO_CONFIG, "w");
    if (!file) return false;

    serializeJson(doc, file);
    file.close();
    return true;
}

bool ConfigManager::loadNetworkConfig(NetworkConfig& config) {
    strncpy(config.ap_ssid, DEFAULT_AP_SSID, sizeof(config.ap_ssid));
    strncpy(config.ap_password, DEFAULT_AP_PASS, sizeof(config.ap_password));
    strncpy(config.sta_ssid, "", sizeof(config.sta_ssid));
    strncpy(config.sta_password, "", sizeof(config.sta_password));
    strncpy(config.hostname, DEFAULT_HOSTNAME, sizeof(config.hostname));
    config.ap_mode = true;

    if (!m_fsMounted || !LittleFS.exists(FILE_NETWORK_CONFIG)) return false;

    File file = LittleFS.open(FILE_NETWORK_CONFIG, "r");
    if (!file) return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error) return false;

    if (!doc["ap_ssid"].isNull()) strncpy(config.ap_ssid, doc["ap_ssid"], sizeof(config.ap_ssid));
    if (!doc["ap_password"].isNull()) strncpy(config.ap_password, doc["ap_password"], sizeof(config.ap_password));
    if (!doc["sta_ssid"].isNull()) strncpy(config.sta_ssid, doc["sta_ssid"], sizeof(config.sta_ssid));
    if (!doc["sta_password"].isNull()) strncpy(config.sta_password, doc["sta_password"], sizeof(config.sta_password));
    if (!doc["hostname"].isNull()) strncpy(config.hostname, doc["hostname"], sizeof(config.hostname));
    if (!doc["ap_mode"].isNull()) config.ap_mode = doc["ap_mode"].as<bool>();

    return true;
}

bool ConfigManager::saveNetworkConfig(const NetworkConfig& config) {
    if (!m_fsMounted) return false;

    JsonDocument doc;
    doc["ap_ssid"] = config.ap_ssid;
    doc["ap_password"] = config.ap_password;
    doc["sta_ssid"] = config.sta_ssid;
    doc["sta_password"] = config.sta_password;
    doc["hostname"] = config.hostname;
    doc["ap_mode"] = config.ap_mode;

    File file = LittleFS.open(FILE_NETWORK_CONFIG, "w");
    if (!file) return false;

    serializeJson(doc, file);
    file.close();
    return true;
}

bool ConfigManager::loadRobotConfig(RobotConfig& config) {
    strncpy(config.robot_name, "Strider32", sizeof(config.robot_name));
    config.default_speed = 50;
    config.default_gait = GaitType::TROT;
    config.startup_pose = RobotPose::REST;

    if (!m_fsMounted || !LittleFS.exists(FILE_ROBOT_CONFIG)) return false;

    File file = LittleFS.open(FILE_ROBOT_CONFIG, "r");
    if (!file) return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error) return false;

    if (!doc["robot_name"].isNull()) strncpy(config.robot_name, doc["robot_name"], sizeof(config.robot_name));
    if (!doc["default_speed"].isNull()) config.default_speed = doc["default_speed"].as<uint8_t>();
    if (!doc["default_gait"].isNull()) config.default_gait = (GaitType)doc["default_gait"].as<uint8_t>();
    if (!doc["startup_pose"].isNull()) config.startup_pose = (RobotPose)doc["startup_pose"].as<uint8_t>();

    return true;
}

bool ConfigManager::saveRobotConfig(const RobotConfig& config) {
    if (!m_fsMounted) return false;

    JsonDocument doc;
    doc["robot_name"] = config.robot_name;
    doc["default_speed"] = config.default_speed;
    doc["default_gait"] = (uint8_t)config.default_gait;
    doc["startup_pose"] = (uint8_t)config.startup_pose;

    File file = LittleFS.open(FILE_ROBOT_CONFIG, "w");
    if (!file) return false;

    serializeJson(doc, file);
    file.close();
    return true;
}

String ConfigManager::listAnimationsJson() {
    JsonDocument doc;
    doc["success"] = true;
    JsonArray anims = doc["animations"].to<JsonArray>();

    if (m_fsMounted && LittleFS.exists(DIR_ANIMATIONS)) {
        File dir = LittleFS.open(DIR_ANIMATIONS);
        if (dir && dir.isDirectory()) {
            File file = dir.openNextFile();
            while (file) {
                String fname = String(file.name());
                if (fname.endsWith(".json")) {
                    String animId = fname.substring(0, fname.length() - 5);
                    JsonObject item = anims.add<JsonObject>();
                    item["id"] = animId;
                    item["size_bytes"] = file.size();
                }
                file = dir.openNextFile();
            }
        }
    }

    String out;
    serializeJson(doc, out);
    return out;
}

bool ConfigManager::saveAnimationJson(const char* animId, const String& jsonContent, String& outError) {
    if (!m_fsMounted) {
        outError = "LittleFS filesystem not mounted";
        return false;
    }

    // Validate JSON schema
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, jsonContent);
    if (err) {
        outError = "Malformed animation JSON format";
        return false;
    }

    if (!doc["frames"].is<JsonArray>()) {
        outError = "Missing or invalid 'frames' array in animation payload";
        return false;
    }

    JsonArray frames = doc["frames"].as<JsonArray>();
    if (frames.size() == 0 || frames.size() > 16) {
        outError = "Keyframe count must be between 1 and 16 frames";
        return false;
    }

    for (JsonObject f : frames) {
        if (!f["angles"].is<JsonArray>() || f["angles"].as<JsonArray>().size() != NUM_SERVOS) {
            outError = "Every keyframe must contain exactly 8 joint angles";
            return false;
        }
        for (JsonVariant a : f["angles"].as<JsonArray>()) {
            float ang = a.as<float>();
            if (ang < 0.0f || ang > 180.0f) {
                outError = "Joint angle out of safe 0-180 degree bounds";
                return false;
            }
        }
        if (!f["duration"].is<uint16_t>() || f["duration"].as<uint16_t>() < 50) {
            outError = "Keyframe duration must be at least 50 ms";
            return false;
        }
    }

    String cleanId = sanitizeFilename(animId);
    String filePath = String(DIR_ANIMATIONS) + "/" + cleanId + ".json";

    File file = LittleFS.open(filePath, "w");
    if (!file) {
        outError = "Failed to open LittleFS file for writing";
        return false;
    }

    file.print(jsonContent);
    file.close();
    return true;
}

bool ConfigManager::loadAnimationJson(const char* animId, String& outJson) {
    if (!m_fsMounted) return false;

    String cleanId = sanitizeFilename(animId);
    String filePath = String(DIR_ANIMATIONS) + "/" + cleanId + ".json";

    if (!LittleFS.exists(filePath)) return false;

    File file = LittleFS.open(filePath, "r");
    if (!file) return false;

    outJson = file.readString();
    file.close();
    return true;
}

bool ConfigManager::deleteAnimationJson(const char* animId) {
    if (!m_fsMounted) return false;

    String cleanId = sanitizeFilename(animId);
    String filePath = String(DIR_ANIMATIONS) + "/" + cleanId + ".json";

    if (!LittleFS.exists(filePath)) return false;
    return LittleFS.remove(filePath);
}

void ConfigManager::resetToDefaults() {
    if (!m_fsMounted) return;
    LittleFS.remove(FILE_SERVO_CONFIG);
    LittleFS.remove(FILE_NETWORK_CONFIG);
    LittleFS.remove(FILE_ROBOT_CONFIG);
}
