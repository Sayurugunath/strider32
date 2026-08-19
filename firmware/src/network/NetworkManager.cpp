#include "NetworkManager.h"
#include <ArduinoJson.h>

NetworkManager::NetworkManager() : m_isAPMode(true), m_lastCheckMs(0) {}

void NetworkManager::begin(const NetworkConfig& config) {
    m_config = config;

    // Force disconnect & mode reset
    WiFi.disconnect(true);
    delay(100);

    if (!m_config.ap_mode && strlen(m_config.sta_ssid) > 0) {
        // Attempt Station (STA) connection
        WiFi.mode(WIFI_STA);
        WiFi.begin(m_config.sta_ssid, m_config.sta_password);

        uint8_t attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            m_isAPMode = false;
        } else {
            // Fallback to AP Mode on connection failure
            m_isAPMode = true;
        }
    }

    if (m_isAPMode) {
        WiFi.mode(WIFI_AP);
        if (strlen(m_config.ap_password) > 0) {
            WiFi.softAP(m_config.ap_ssid, m_config.ap_password);
        } else {
            WiFi.softAP(m_config.ap_ssid);
        }
    }

    // Register mDNS domain
    if (MDNS.begin(m_config.hostname)) {
        MDNS.addService("http", "tcp", 80);
    }
}

void NetworkManager::update() {
    uint32_t now = millis();
    if (now - m_lastCheckMs < 5000) return; // Check every 5 seconds
    m_lastCheckMs = now;

    // Auto-reconnect STA mode if dropped
    if (!m_isAPMode && WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
    }
}

bool NetworkManager::isConnected() const {
    return (m_isAPMode || WiFi.status() == WL_CONNECTED);
}

String NetworkManager::getIPAddress() const {
    if (m_isAPMode) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}

int8_t NetworkManager::getRSSI() const {
    if (m_isAPMode) return 0;
    return WiFi.RSSI();
}

String NetworkManager::scanNetworksJson() {
    int n = WiFi.scanNetworks();
    JsonDocument doc;
    JsonArray networks = doc["networks"].to<JsonArray>();

    for (int i = 0; i < n; ++i) {
        JsonObject item = networks.add<JsonObject>();
        item["ssid"] = WiFi.SSID(i);
        item["rssi"] = WiFi.RSSI(i);
        item["encryption"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    }

    String output;
    serializeJson(doc, output);
    return output;
}
