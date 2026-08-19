#include "ApiRoutes.h"
#include "hardware/ServoDriver.h"
#include "hardware/SafetySystem.h"
#include "motion/GaitEngine.h"
#include "motion/AnimationPlayer.h"
#include "config/ConfigManager.h"
#include "network/NetworkManager.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>

void ApiRoutes::registerRoutes(AsyncWebServer& server,
                                ServoDriver* servoDriver,
                                SafetySystem* safetySystem,
                                GaitEngine* gaitEngine,
                                AnimationPlayer* animPlayer,
                                ConfigManager* configMgr,
                                NetworkManager* netMgr) {

    // Serve Web UI static files from LittleFS
    server.serveStatic("/", LittleFS, "/public/").setDefaultFile("index.html");

    // ========================================================================
    // 1. SYSTEM & STATUS ENDPOINTS
    // ========================================================================

    // GET /api/v1/status
    server.on("/api/v1/status", HTTP_GET, [=](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["success"] = true;
        JsonObject data = doc["data"].to<JsonObject>();
        data["robot_name"] = "Strider32";
        data["firmware_version"] = "0.3.0";
        data["uptime_seconds"] = millis() / 1000;
        data["free_heap_bytes"] = ESP.getFreeHeap();
        data["estop_active"] = safetySystem->isEmergencyStopActive();
        data["safety_state"] = (uint8_t)safetySystem->getSafetyState();
        data["gait"] = (uint8_t)gaitEngine->getGait();
        data["speed"] = gaitEngine->getSpeed();
        data["servo_backend"] = (uint8_t)servoDriver->getBackendType();

        JsonObject wifiObj = data["wifi"].to<JsonObject>();
        wifiObj["mode"] = netMgr->isAPMode() ? "AP" : "STA";
        wifiObj["ip"] = netMgr->getIPAddress();
        wifiObj["rssi"] = netMgr->getRSSI();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // GET /api/v1/diagnostics (Real Unfiltered System Metrics)
    server.on("/api/v1/diagnostics", HTTP_GET, [=](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["success"] = true;
        JsonObject data = doc["data"].to<JsonObject>();
        data["uptime_seconds"] = millis() / 1000;
        data["free_heap_bytes"] = ESP.getFreeHeap();
        data["min_free_heap_bytes"] = ESP.getMinFreeHeap();
        data["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
        data["chip_revision"] = ESP.getChipRevision();
        data["firmware_version"] = "0.2.0";
        data["wifi_mode"] = netMgr->isAPMode() ? "AP" : "STA";
        data["wifi_connected"] = netMgr->isConnected();
        data["rssi_dbm"] = netMgr->getRSSI();
        data["ip_address"] = netMgr->getIPAddress();
        data["safety_state"] = (uint8_t)safetySystem->getSafetyState();
        data["estop_active"] = safetySystem->isEmergencyStopActive();
        data["servo_backend"] = (uint8_t)servoDriver->getBackendType();
        data["hardware_attached"] = servoDriver->isHardwareAttached();
        data["last_error"] = safetySystem->getLastErrorReason();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // ========================================================================
    // 2. EMERGENCY STOP ENDPOINTS
    // ========================================================================

    // POST /api/v1/estop
    server.on("/api/v1/estop", HTTP_POST, [=](AsyncWebServerRequest *request) {
        safetySystem->triggerEmergencyStop("API Request");
        gaitEngine->stop();
        animPlayer->stop();

        request->send(200, "application/json", "{\"success\":true,\"message\":\"Emergency Stop Activated. Servo outputs disabled.\"}");
    });

    // POST /api/v1/estop/reset
    server.on("/api/v1/estop/reset", HTTP_POST, [=](AsyncWebServerRequest *request) {
        const char* msg = nullptr;
        bool ok = safetySystem->resetEmergencyStop(&msg);

        JsonDocument doc;
        doc["success"] = ok;
        if (ok) {
            doc["message"] = msg;
            doc["state"] = "READY";
            String resp;
            serializeJson(doc, resp);
            request->send(200, "application/json", resp);
        } else {
            doc["error"] = msg;
            doc["state"] = "ESTOPPED";
            String resp;
            serializeJson(doc, resp);
            request->send(400, "application/json", resp);
        }
    });

    // ========================================================================
    // 3. NETWORK CONFIGURATION ENDPOINTS
    // ========================================================================

    // GET /api/v1/network
    server.on("/api/v1/network", HTTP_GET, [=](AsyncWebServerRequest *request) {
        NetworkConfig netCfg;
        configMgr->loadNetworkConfig(netCfg);

        JsonDocument doc;
        doc["success"] = true;
        JsonObject data = doc["data"].to<JsonObject>();
        data["mode"] = netCfg.ap_mode ? "AP" : "STA";
        data["ap_ssid"] = netCfg.ap_ssid;
        data["sta_ssid"] = netCfg.sta_ssid;
        data["sta_password"] = "***"; // Masked for security!
        data["hostname"] = netCfg.hostname;
        data["current_ip"] = netMgr->getIPAddress();
        data["rssi"] = netMgr->getRSSI();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // POST /api/v1/network
    AsyncCallbackJsonWebHandler *netHandler = new AsyncCallbackJsonWebHandler("/api/v1/network",
        [=](AsyncWebServerRequest *request, JsonVariant &json) {
            JsonObject obj = json.as<JsonObject>();
            NetworkConfig netCfg;
            configMgr->loadNetworkConfig(netCfg);

            if (!obj["mode"].isNull()) {
                const char* m = obj["mode"];
                netCfg.ap_mode = (strcmp(m, "AP") == 0 || strcmp(m, "ap") == 0);
            }
            if (!obj["ap_ssid"].isNull()) strncpy(netCfg.ap_ssid, obj["ap_ssid"], sizeof(netCfg.ap_ssid));
            if (!obj["ap_password"].isNull()) strncpy(netCfg.ap_password, obj["ap_password"], sizeof(netCfg.ap_password));
            if (!obj["sta_ssid"].isNull()) strncpy(netCfg.sta_ssid, obj["sta_ssid"], sizeof(netCfg.sta_ssid));
            if (!obj["sta_password"].isNull() && strcmp(obj["sta_password"], "***") != 0) {
                strncpy(netCfg.sta_password, obj["sta_password"], sizeof(netCfg.sta_password));
            }
            if (!obj["hostname"].isNull()) strncpy(netCfg.hostname, obj["hostname"], sizeof(netCfg.hostname));

            bool saved = configMgr->saveNetworkConfig(netCfg);
            if (saved) {
                request->send(200, "application/json", "{\"success\":true,\"message\":\"Network configuration saved to LittleFS\"}");
            } else {
                request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to write network configuration to flash\"}");
            }
        });
    server.addHandler(netHandler);

    // GET /api/v1/network/scan
    server.on("/api/v1/network/scan", HTTP_GET, [=](AsyncWebServerRequest *request) {
        String json = netMgr->scanNetworksJson();
        request->send(200, "application/json", json);
    });

    // ========================================================================
    // 4. SERVO & CONFIGURATION ENDPOINTS
    // ========================================================================

    // GET /api/v1/config
    server.on("/api/v1/config", HTTP_GET, [=](AsyncWebServerRequest *request) {
        ServoConfig sCfg = servoDriver->getConfig();
        RobotConfig rCfg;
        configMgr->loadRobotConfig(rCfg);

        JsonDocument doc;
        doc["success"] = true;
        JsonObject data = doc["data"].to<JsonObject>();
        data["robot_name"] = rCfg.robot_name;
        data["default_speed"] = rCfg.default_speed;
        data["default_gait"] = (uint8_t)rCfg.default_gait;
        data["startup_pose"] = (uint8_t)rCfg.startup_pose;
        data["backend"] = (uint8_t)sCfg.backend;
        data["pulse_min_us"] = sCfg.pulse_min_us;
        data["pulse_max_us"] = sCfg.pulse_max_us;

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // POST /api/v1/config
    AsyncCallbackJsonWebHandler *configHandler = new AsyncCallbackJsonWebHandler("/api/v1/config",
        [=](AsyncWebServerRequest *request, JsonVariant &json) {
            JsonObject obj = json.as<JsonObject>();
            RobotConfig rCfg;
            configMgr->loadRobotConfig(rCfg);

            if (!obj["robot_name"].isNull()) strncpy(rCfg.robot_name, obj["robot_name"], sizeof(rCfg.robot_name));
            if (!obj["default_speed"].isNull()) rCfg.default_speed = obj["default_speed"].as<uint8_t>();
            if (!obj["default_gait"].isNull()) rCfg.default_gait = (GaitType)obj["default_gait"].as<uint8_t>();
            if (!obj["startup_pose"].isNull()) rCfg.startup_pose = (RobotPose)obj["startup_pose"].as<uint8_t>();

            bool saved = configMgr->saveRobotConfig(rCfg);
            if (saved) {
                request->send(200, "application/json", "{\"success\":true,\"message\":\"Robot configuration saved\"}");
            } else {
                request->send(500, "application/json", "{\"success\":false,\"error\":\"Failed to save robot configuration\"}");
            }
        });
    server.addHandler(configHandler);

    // GET /api/v1/servos
    server.on("/api/v1/servos", HTTP_GET, [=](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["success"] = true;
        JsonObject data = doc["data"].to<JsonObject>();
        data["backend"] = (uint8_t)servoDriver->getBackendType();
        data["pulse_min_us"] = servoDriver->getConfig().pulse_min_us;
        data["pulse_max_us"] = servoDriver->getConfig().pulse_max_us;

        JsonArray anglesArr  = data["angles"].to<JsonArray>();
        JsonArray offsetsArr = data["offsets"].to<JsonArray>();

        for (uint8_t i = 0; i < NUM_SERVOS; i++) {
            anglesArr.add(servoDriver->getServoAngle(i));
            offsetsArr.add(servoDriver->getServoOffset(i));
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    // POST /api/v1/servos/calibrate
    AsyncCallbackJsonWebHandler *calibHandler = new AsyncCallbackJsonWebHandler("/api/v1/servos/calibrate", 
        [=](AsyncWebServerRequest *request, JsonVariant &json) {
            if (!safetySystem->canExecuteMotion()) {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Operation rejected: System is ESTOPPED\"}");
                return;
            }

            JsonObject obj = json.as<JsonObject>();
            if (obj["offsets"].is<JsonArray>()) {
                JsonArray offsets = obj["offsets"].as<JsonArray>();
                for (uint8_t i = 0; i < NUM_SERVOS && i < offsets.size(); i++) {
                    servoDriver->setServoOffset(i, offsets[i].as<int16_t>());
                }
            }

            if (!obj["backend"].isNull()) {
                servoDriver->setBackend((ServoBackendType)obj["backend"].as<uint8_t>());
            }

            if (obj["save"].as<bool>()) {
                configMgr->saveServoConfig(servoDriver->getConfig());
            }

            request->send(200, "application/json", "{\"success\":true,\"message\":\"Calibration updated\"}");
        });
    server.addHandler(calibHandler);

    // ========================================================================
    // 5. MOTION CONTROL ENDPOINT
    // ========================================================================

    // POST /api/v1/control
    AsyncCallbackJsonWebHandler *controlHandler = new AsyncCallbackJsonWebHandler("/api/v1/control", 
        [=](AsyncWebServerRequest *request, JsonVariant &json) {
            if (!safetySystem->canExecuteMotion()) {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Motion command rejected: System is ESTOPPED\"}");
                return;
            }

            JsonObject obj = json.as<JsonObject>();
            const char* action = obj["action"] | "stop";
            
            if (strcmp(action, "walk") == 0) {
                float fwd = obj["forward"] | 0.0f;
                float lat = obj["lateral"] | 0.0f;
                float turn = obj["turn"] | 0.0f;
                gaitEngine->moveDirection(fwd, lat, turn);
                safetySystem->setRunningState(true);
            } else if (strcmp(action, "stop") == 0) {
                gaitEngine->stop();
                safetySystem->setRunningState(false);
            } else if (strcmp(action, "gait") == 0) {
                uint8_t g = obj["gait"] | 0;
                gaitEngine->setGait((GaitType)g);
            } else if (strcmp(action, "speed") == 0) {
                uint8_t spd = obj["speed"] | 50;
                gaitEngine->setSpeed(spd);
            }

            request->send(200, "application/json", "{\"success\":true}");
        });
    server.addHandler(controlHandler);

    // ========================================================================
    // 6. ANIMATION STORAGE & PLAYBACK ENDPOINTS (PHASE 3D)
    // ========================================================================

    // GET /api/v1/animations (List stored animations)
    server.on("/api/v1/animations", HTTP_GET, [=](AsyncWebServerRequest *request) {
        String jsonList = configMgr->listAnimationsJson();
        request->send(200, "application/json", jsonList);
    });

    // POST /api/v1/animations (Save new animation)
    AsyncCallbackJsonWebHandler *saveAnimHandler = new AsyncCallbackJsonWebHandler("/api/v1/animations",
        [=](AsyncWebServerRequest *request, JsonVariant &json) {
            JsonObject obj = json.as<JsonObject>();
            const char* id = obj["id"] | "new_anim";
            
            String jsonStr;
            serializeJson(obj, jsonStr);

            String errStr;
            bool ok = configMgr->saveAnimationJson(id, jsonStr, errStr);
            if (ok) {
                request->send(200, "application/json", "{\"success\":true,\"message\":\"Animation saved to LittleFS\"}");
            } else {
                JsonDocument errDoc;
                errDoc["success"] = false;
                errDoc["error"] = errStr;
                String resp;
                serializeJson(errDoc, resp);
                request->send(400, "application/json", resp);
            }
        });
    server.addHandler(saveAnimHandler);

    // GET /api/v1/animations/* (Get specific animation file)
    server.on("^\\/api\\/v1\\/animations\\/([a-zA-Z0-9_-]+)$", HTTP_GET, [=](AsyncWebServerRequest *request) {
        String animId = request->pathArg(0);
        String jsonContent;
        if (configMgr->loadAnimationJson(animId.c_str(), jsonContent)) {
            request->send(200, "application/json", jsonContent);
        } else {
            request->send(404, "application/json", "{\"success\":false,\"error\":\"Animation file not found\"}");
        }
    });

    // DELETE /api/v1/animations/* (Delete animation file)
    server.on("^\\/api\\/v1\\/animations\\/([a-zA-Z0-9_-]+)$", HTTP_DELETE, [=](AsyncWebServerRequest *request) {
        String animId = request->pathArg(0);
        if (configMgr->deleteAnimationJson(animId.c_str())) {
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Animation deleted\"}");
        } else {
            request->send(404, "application/json", "{\"success\":false,\"error\":\"Animation file not found\"}");
        }
    });

    // POST /api/v1/animations/*/play (Play animation file)
    server.on("^\\/api\\/v1\\/animations\\/([a-zA-Z0-9_-]+)\\/play$", HTTP_POST, [=](AsyncWebServerRequest *request) {
        if (!safetySystem->canExecuteMotion()) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Animation playback rejected: System is ESTOPPED\"}");
            return;
        }

        String animId = request->pathArg(0);
        String jsonContent;
        if (!configMgr->loadAnimationJson(animId.c_str(), jsonContent)) {
            request->send(404, "application/json", "{\"success\":false,\"error\":\"Animation file not found\"}");
            return;
        }

        if (animPlayer->loadFromJson(jsonContent.c_str())) {
            animPlayer->play(false);
            safetySystem->setRunningState(true);
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Animation playback started\"}");
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid animation keyframe format\"}");
        }
    });
}
