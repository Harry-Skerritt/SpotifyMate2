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


private:

    // Constructor for singleton
    WifiManager() {}

    String ssid;
    String pass;

    // Internal Helper
    void loadWifiConfig();

    // Prevent copying
    WifiManager(const WifiManager&) = delete;
    void operator=(const WifiManager&) = delete;
};

#endif //WIFIMANAGER_H
