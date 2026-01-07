//
// Created by Harry Skerritt on 06/01/2026.
//

#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>


class WifiManager {
public:
    static WifiManager& getInstance() {
        static WifiManager instance;
        return instance;
    }

    void update();

    void requestConnect(uint32_t timeoutMs = 15000);
    void processConnect();

    void requestScan();
    void processScan();

    void requestReset();
    void processReset();

private:

    WifiManager() {}

    // Connection
    String ssid_to_connect;
    String pass_to_connect;
    uint32_t connect_timeout;
    uint32_t connect_start_time;
    void handleConnecting();

    // Scanning
    void handleScanning();



    WifiManager(const WifiManager&) = delete;
    void operator=(const WifiManager&) = delete;
};



#endif //WIFIMANAGER_H
