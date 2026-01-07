//
// Created by Harry Skerritt on 06/01/2026.
//

#include "WifiManager.h"

#include <LittleFS.h>

#include "system/SystemManager.h"

#include <WiFi.h>

#include "global_state.h"
#include "spotify/SpotifyManager.h"

void WifiManager::update() {
    if (networkState.status == WIFI_CONNECTING) {
        handleConnecting();
    }
    else if (networkState.status == WIFI_SCANNING) {
        handleScanning();
    }
}


// --- Connect ---
void WifiManager::requestConnect(uint32_t timeoutMs) {
    ssid_to_connect = networkState.selected_ssid;
    pass_to_connect = networkState.selected_pass;
    connect_timeout = timeoutMs;

    if (systemTaskHandle != NULL) {
        xTaskNotify(systemTaskHandle, CMD_WIFI_CONN, eSetBits);
    }
}

void WifiManager::processConnect() {
    networkState.status = WIFI_CONNECTING;
    Serial.printf("Connecting to %s...\n", ssid_to_connect.c_str());

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid_to_connect.c_str(), pass_to_connect.c_str());

    connect_start_time = millis();
}

void WifiManager::handleConnecting() {
    if (WiFi.status() == WL_CONNECTED) {
        networkState.status = WIFI_CONNECTED;
        networkState.wifi_connected = true;
        networkState.ip = WiFi.localIP().toString();
        Serial.printf("Device IP: %s\n", networkState.ip.c_str());
        Serial.println("WiFi connected");

        // Save config here
        systemState.setup_complete = true;
        SystemManager::getInstance().writeConfig();

    } else if (millis() - connect_start_time > connect_timeout) {
        // Network timed out
        networkState.status = WIFI_ERROR;
        networkState.wifi_connected = false;
        WiFi.disconnect();
        Serial.println("WiFi Timeout!");
    }
}

// --- Scanning ---
void WifiManager::requestScan() {
    if (systemTaskHandle != NULL) {
        xTaskNotify(systemTaskHandle, CMD_WIFI_SCAN, eSetBits);
    }
}

void WifiManager::processScan() {
    Serial.println("WiFi Scanning...");

    WiFi.disconnect();
    networkState.status = WIFI_SCANNING;
    networkState.wifi_connected = false;


    int n = WiFi.scanNetworks(false, false);

    networkState.found_ssids.clear();
    for (int i = 0; i < n; ++i) {
        String currentSSID = WiFi.SSID(i);


        bool exists = false;
        for (const String& s : networkState.found_ssids) {
            if (s == currentSSID) { exists = true; break; }
        }

        if (!exists && currentSSID.length() > 0) {
            networkState.found_ssids.push_back(currentSSID);
        }
    }

    WiFi.scanDelete();
    networkState.status = WIFI_SCAN_RESULTS;
    Serial.println("WiFi: Scan complete");
}

void WifiManager::handleScanning() {

}

// --- Reset ---
void WifiManager::requestReset() {
    if (systemTaskHandle != NULL) {
        xTaskNotify(systemTaskHandle, CMD_WIFI_RESET, eSetBits);
    }
}

void WifiManager::processReset() {
    Serial.println("Wifi Reset Requested...");

    WiFi.disconnect(true, true);

    SystemManager::getInstance().resetConfig();

    networkState.selected_ssid = "";
    networkState.selected_pass = "";
    networkState.wifi_connected = false;
    networkState.status = WIFI_IDLE; // Should trigger onboarding

    Serial.println("WiFi Reset Complete");
}






