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
        vTaskDelay(pdMS_TO_TICKS(100));

        IPAddress local_IP(192, 168, 0, 141);
        IPAddress gateway(192, 168, 0, 1);
        IPAddress subnet(255, 255, 255, 0);
        IPAddress primaryDNS(8, 8, 8, 8);   // Google DNS
        IPAddress secondaryDNS(1, 1, 1, 1);

        if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
            Serial.println("STA: Static IP Failed to Configure");
        }

        networkState.selected_ssid = ssid;
        networkState.selected_pass = pass;
        networkState.setup_complete = true;


        WiFi.setSleep(false);

        //WiFi.begin(ssid.c_str(), pass.c_str());
        vTaskDelay(pdMS_TO_TICKS(100));
        networkState.start_connect_trigger = true;
    }
}


void WifiManager::loadWifiConfig() {
    File file = LittleFS.open("/config.json", "r");

    if (file) {
        JsonDocument doc;
        deserializeJson(doc, file);
        ssid = doc["ssid"].as<String>();
        pass = doc["password"].as<String>();
        networkState.setup_complete = doc["setup_complete"].as<bool>();
        networkState.spotify_linked = doc["spotify_linked"].as<bool>();
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

// Config
void WifiManager::saveWifiConfig(const String& s, const String& p) {
    File file = LittleFS.open("/config.json", "w");
    if (!file) {
        Serial.println("WiFi: Failed to open config.json");
        deviceState.fatal_error_trigger = true;
        return;
    }

    JsonDocument doc;
    doc["ssid"] = s;
    doc["password"] = p;
    doc["setup_complete"] = true;

    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to file");
        file.close();
        return;
    }

    file.close();
    Serial.println("Config Saved Successfully");
}





