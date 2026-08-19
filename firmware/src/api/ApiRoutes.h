#ifndef API_ROUTES_H
#define API_ROUTES_H

#include "robot_types.h"
#include <ESPAsyncWebServer.h>

class ServoDriver;
class SafetySystem;
class GaitEngine;
class AnimationPlayer;
class ConfigManager;
class NetworkManager;

class ApiRoutes {
public:
    static void registerRoutes(AsyncWebServer& server,
                                ServoDriver* servoDriver,
                                SafetySystem* safetySystem,
                                GaitEngine* gaitEngine,
                                AnimationPlayer* animPlayer,
                                ConfigManager* configMgr,
                                NetworkManager* netMgr);
};

#endif // API_ROUTES_H
