//
// Created by Harry Skerritt on 05/01/2026.
//

#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>


class WifiManager {
public:

    static WifiManager& getInstance() {
        static WifiManager instance;
        return instance;
    }

    void init();
    void update();
    void handleAsyncScan();
    void saveWifiConfig(const String& ssid, const String& password);


private:

    // Constructor for singleton
    WifiManager() {}

    String ssid;
    String pass;
    uint32_t connect_start_time;
    uint32_t current_timeout = 15000;

    // Internal Helper
    bool loadWifiConfig();
    void startConnection();
    void handleConnectionSuccess();
    void handleConnectionFailure();

    // Prevent copying
    WifiManager(const WifiManager&) = delete;
    void operator=(const WifiManager&) = delete;
};

#endif //WIFIMANAGER_H
