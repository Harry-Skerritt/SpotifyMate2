//
// Created by Harry Skerritt on 06/01/2026.
//

#include "SystemManager.h"

#include <Wire.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "global_state.h"
#include "../../../../../../.platformio/packages/toolchain-riscv32-esp/riscv32-esp-elf/include/c++/8.4.0/set"
#include "network/WifiManager.h"
#include "ui/UIManager.h"


void SystemManager::init() {
    systemState.status = SYSTEM_STATUS_ACTIVE;

    // --- Loading ---
    // Handling Config.json
    if (!loadConfig()) {
        //  Device definitely hasn't been set up
        Serial.println("Failed to load config");
        networkState.status = WIFI_IDLE;
    }

    // Handling Secret.json
    if (!loadSpotifySecrets()) {
        Serial.println("Failed to load spotify secrets");
        UIManager::getInstance().showFailure();
        return;
    }

    // Handling tokens.json
    if (!loadSpotifyTokens()) {
        Serial.println("Failed to load spotify tokens");
        // This isn't critical but will mean spotify needs relinking
        spotifyState.status = SPOTIFY_NEED_LINK;
        spotifyState.refresh_token = "";
    }

    // --- Spotify Check ---
    if (spotifyState.client_id.length() == 0)
    {
        Serial.println("Spotify Credentials are missing - aborting!");
        UIManager::getInstance().showFailure();
        return;
    }


    // --- WiFi Check ---
    // Device could've been set up
    if (systemState.setup_complete) {
        // System has been set up before
        if (networkState.selected_ssid.length() > 0 &&
            networkState.selected_pass.length() > 0)
        {
            networkState.status = WIFI_CONNECTING; // Preempt the connection and hopefully avoid errors

            //WifiManager::getInstance().requestConnect();
            connectionRequested = true;
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

void SystemManager::update() {
    if (connectionRequested) {
        WifiManager::getInstance().requestConnect();
        connectionRequested = false;
    }
}



// --- CONFIG ---
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
        Serial.println("System: Failed to open config.json");
        return;
    }

    JsonDocument doc;
    doc["ssid"] = networkState.selected_ssid;
    doc["password"] = networkState.selected_pass;
    doc["setup_complete"] = systemState.setup_complete;
    doc["spotify_linked"] = systemState.spotify_linked;

    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to file - config");
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

// --- SECRET ---
bool SystemManager::loadSpotifySecrets() {
    if (!LittleFS.exists("/secrets.json")) return false;

    File file = LittleFS.open("/secrets.json", "r");
    if (!file) return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("System: Secrets is corrupt");
        return false;
    }

    spotifyState.client_id = doc["spotify_client_id"].as<String>();
    spotifyState.client_secret = doc["spotify_client_secret"].as<String>();

    return true;
}


// --- TOKEN ---
bool SystemManager::loadSpotifyTokens() {
    if (!LittleFS.exists("/tokens.json")) return false;

    File file = LittleFS.open("/tokens.json", "r");
    if (!file) return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("System: Tokens is corrupt");
        return false;
    }

    spotifyState.refresh_token = doc["spotify_refresh_token"].as<String>();
    //spotifyState.a = doc["spotify_client_secret"].as<String>();

    return true;
}

void SystemManager::writeSpotifyTokens() {
    File file = LittleFS.open("/tokens.json", "w");
    if (!file) {
        Serial.println("System: Failed to open tokens.json");
        return;
    }

    JsonDocument doc;
    doc["spotify_refresh_token"] = spotifyState.refresh_token;

    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to file - tokens");
        file.close();
        return;
    }

    file.close();
    Serial.println("Tokens Saved Successfully");
}

void SystemManager::resetSpotifyTokens() {
    LittleFS.remove("/tokens.json");
    spotifyState.refresh_token = "";
}


// --- Helper ---
void SystemManager::setBacklight(bool on) {
    Wire.beginTransmission(0x38);

    // 0x0F usually turns it on/full brightness, 0x00 usually turns it off
    Wire.write(on ? 0x0F : 0x00);

    Wire.endTransmission();
    Serial.printf("System: Backlight %s\n", on ? "ON" : "OFF");
}



// --- Sleep Mode ---
void SystemManager::enterSleepMode() {
    Serial.println("System: Entering sleep mode");

    // Dim Backlight
    setBacklight(false);

    // Show Sleep Screen
    UIManager::getInstance().showContextScreen("Sleeping...");
}

void SystemManager::exitSleepMode() {

    // Turn Backlight on
    setBacklight(true);

    UIManager::getInstance().showMainPlayer();

    // Refresh UI
    spotifyState.needs_art_update = true;
    spotifyState.needs_text_update = true;

    Serial.println("System: Exited sleep mode");
}
