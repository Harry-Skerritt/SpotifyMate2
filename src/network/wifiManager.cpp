//
// Created by Harry Skerritt on 05/01/2026.
//

#include "wifiManager.h"

#include "global_state.h"


// Init
void WifiManager::init() {
    if (loadWifiConfig()) {
        transitionFlags.start_connect_trigger = true;
    }
}


void WifiManager::update() {
    handleAsyncScan(); // Handles trigger

    if (transitionFlags.start_connect_trigger) {
        startConnection();
    }

    if (transitionFlags.is_connecting) {
        if (WiFi.status() == WL_CONNECTED) {
            handleConnectionSuccess();
        } else if (millis() - connect_start_time > current_timeout) {
            handleConnectionFailure();
        }
    }
}

void WifiManager::startConnection() {
    transitionFlags.start_connect_trigger = false;

    String targetSSID = (ssid.length() > 0) ? ssid : networkState.selected_ssid;
    String targetPass = (pass.length() > 0) ? pass : networkState.selected_pass;


    if (ssid.length() > 0 && pass.length() > 0) {
        WiFi.disconnect();
        WiFi.mode(WIFI_STA);

        connect_start_time = millis();
        current_timeout = (transitionFlags.failed_to_connect_trigger) ? 30000 : 15000;

        networkState.selected_ssid = targetSSID;
        networkState.selected_pass = targetPass;

        WiFi.begin(ssid.c_str(), pass.c_str());

        transitionFlags.is_connecting = true;
        transitionFlags.failed_to_connect_trigger = false;

        Serial.println("WiFi: Connecting...");
    }
}

void WifiManager::handleConnectionSuccess() {
    transitionFlags.is_connecting = false;
    networkState.wifi_connected = true;
    networkState.ip = WiFi.localIP().toString();

    saveWifiConfig(networkState.selected_ssid, networkState.selected_pass);

    transitionFlags.show_success_trigger = true;
    Serial.println("WiFi: Connection Successful");
}


void WifiManager::handleConnectionFailure() {
    transitionFlags.is_connecting = false;
    networkState.wifi_connected = false;
    transitionFlags.failed_to_connect_trigger = true;
    Serial.println("WiFi: Connection Timeout");
}


bool WifiManager::loadWifiConfig() {
    if (!LittleFS.exists("/config.json")) return false;

    File file = LittleFS.open("/config.json", "r");
    if (!file) return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("WiFi: Config file corrupted");
        return false;
    }

    // Assign to local variables
    ssid = doc["ssid"].as<String>();
    pass = doc["password"].as<String>();

    // Update global states
    deviceState.setup_complete = doc["setup_complete"].as<bool>();
    deviceState.spotify_linked = doc["spotify_linked"].as<bool>();

    // Only return true if the setup is actually finished
    return deviceState.setup_complete;
}


// Scan
void WifiManager::handleAsyncScan() {
    if (transitionFlags.start_scan_trigger) {
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
        transitionFlags.start_scan_trigger = false;
        transitionFlags.scan_complete = true;
        Serial.println("WiFi: Scan complete");
    }
}

// Config
void WifiManager::saveWifiConfig(const String& s, const String& p) {
    File file = LittleFS.open("/config.json", "w");
    if (!file) {
        Serial.println("WiFi: Failed to open config.json");
        transitionFlags.fatal_error_trigger = true;
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






