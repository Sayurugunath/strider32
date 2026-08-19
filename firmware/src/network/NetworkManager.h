#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "robot_types.h"
#include <WiFi.h>
#include <ESPmDNS.h>

class NetworkManager {
public:
    NetworkManager();
    void begin(const NetworkConfig& config);
    void update();

    bool isConnected() const;
    String getIPAddress() const;
    int8_t getRSSI() const;
    bool isAPMode() const { return m_isAPMode; }

    String scanNetworksJson();

private:
    NetworkConfig m_config;
    bool m_isAPMode;
    uint32_t m_lastCheckMs;
};

#endif // NETWORK_MANAGER_H
