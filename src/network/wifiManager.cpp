//
// Created by Harry Skerritt on 05/01/2026.
//

#include "wifiManager.h"

#include "global_state.h"


// Init
void WifiManager::init() {
    loadWifiConfig();


    if (ssid.length() > 0 && pass.length() > 0) {
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(ssid.c_str(), pass.c_str());
        networkState.has_been_connected = true;
    }
}

void WifiManager::loadWifiConfig() {
    if (!LittleFS.begin(true)) { // 'true' forces format if mount fails
        Serial.println("LittleFS Mount Failed");
        return;
    }

    File file = LittleFS.open("/config.json", "r");

    if (file) {
        JsonDocument doc;
        deserializeJson(doc, file);
        ssid = doc["ssid"].as<String>();
        pass = doc["password"].as<String>();
        file.close();
    }

}


// Scan
void WifiManager::handleAsyncScan() {
    if (networkState.start_scan_trigger) {
        Serial.println("WiFi: Scan requested...");

        WiFi.disconnect();
        networkState.wifi_connected = false;
        networkState.found_ssids.clear();

        int n = WiFi.scanNetworks(false, false);

        for (int i = 0; i < n; ++i) {
            String currentSSID = WiFi.SSID(i);

            // CHECK FOR DUPLICATES
            bool exists = false;
            for (const String& s : networkState.found_ssids) {
                if (s == currentSSID) {
                    exists = true;
                    break;
                }
            }

            if (!exists && currentSSID.length() > 0) {
                networkState.found_ssids.push_back(currentSSID);
            }
        }

        WiFi.scanDelete();
        networkState.start_scan_trigger = false;
        networkState.scan_complete = true;
        Serial.println("WiFi: Scan complete");
    }
}




