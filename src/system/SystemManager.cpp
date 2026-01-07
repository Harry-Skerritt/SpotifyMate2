//
// Created by Harry Skerritt on 06/01/2026.
//

#include "SystemManager.h"

#include <LittleFS.h>
#include <ArduinoJson.h>

#include "global_state.h"
#include "network/WifiManager.h"
#include "ui/UIManager.h"

void SystemManager::init() {
    if (!loadConfig()) {
        //  Device definitely hasn't been set up
        Serial.println("Failed to load config");
        networkState.status = WIFI_IDLE;
    }

    // Device could've been set up
    if (systemState.setup_complete) {
        // System has been set up before
        if (networkState.selected_ssid.length() > 0 &&
            networkState.selected_pass.length() > 0)
        {
            networkState.status = WIFI_CONNECTING; // Preempt the connection and hopefully avoid errors

            WifiManager::getInstance().requestConnect();
            String msg = "Connecting to " + networkState.selected_ssid;
            UIManager::getInstance().showSpinner(msg.c_str());
        }
        else {
            // Has been set up but wi-fi failed
            networkState.status = WIFI_ERROR;
        }
    } else {
        // Device has not been set up - initiate onboarding
        networkState.status = WIFI_IDLE;
    }

}


bool SystemManager::loadConfig() {
    if (!LittleFS.exists("/config.json")) return false;

    File file = LittleFS.open("/config.json", "r");
    if (!file) return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("System: Config is corrupt");
        return false;
    }

    networkState.selected_ssid = doc["ssid"].as<String>();
    networkState.selected_pass = doc["password"].as<String>();

    systemState.setup_complete = doc["setup_complete"].as<bool>();
    systemState.spotify_linked = doc["spotify_linked"].as<bool>();

    return true;
}

void SystemManager::writeConfig() {
    File file = LittleFS.open("/config.json", "w");
    if (!file) {
        Serial.println("WiFi: Failed to open config.json");
        return;
    }

    JsonDocument doc;
    doc["ssid"] = networkState.selected_ssid;
    doc["password"] = networkState.selected_pass;
    doc["setup_complete"] = systemState.setup_complete;
    doc["spotify_linked"] = systemState.spotify_linked;

    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to file");
        file.close();
        return;
    }

    file.close();
    Serial.println("Config Saved Successfully");
}

void SystemManager::resetConfig() {
    LittleFS.remove("/config.json");

    systemState.setup_complete = false;
    systemState.spotify_linked = false; // Todo: Need this?
}




