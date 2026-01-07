//
// Created by Harry Skerritt on 05/01/2026.
//

#ifndef GLOBAL_STATE_H
#define GLOBAL_STATE_H

#pragma once
#include <Arduino.h>
#include <vector>

// --- Command Bitmasks ---
#define CMD_WIFI_SCAN    (1 << 0)
#define CMD_WIFI_CONN    (1 << 1)
#define CMD_WIFI_RESET   (1 << 2)

// --- Task Handles ---
extern TaskHandle_t systemTaskHandle;

enum SpotifyStatus {
    SPOTIFY_IDLE,          // Not doing anything yet
    SPOTIFY_INITIALIZING,  // Checking if the saved token works
    SPOTIFY_NEED_LINK,     // Wi-Fi is up, but no token found
    SPOTIFY_LINKING,       // QR code is on screen, waiting for user
    SPOTIFY_AUTHENTICATING,// User scanned QR, ESP32 is exchanging code for token
    SPOTIFY_READY,         // Token validated, go to player
    SPOTIFY_ERROR          // Auth failed or Token revoked
};

struct SpotifyState {
    SpotifyStatus status = SPOTIFY_IDLE;
    String client_id;
    String client_secret;
    String refresh_token;

    String auth_url = "";
};


struct SystemState {
    bool setup_complete = false;
    bool spotify_linked = false;
};


enum WifiStatus {
    WIFI_IDLE,
    WIFI_SCANNING,
    WIFI_SCAN_RESULTS,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_ERROR
};

struct NetworkState {
    WifiStatus status = WIFI_IDLE;
    bool wifi_connected = false;
    String ip = "0.0.0.0";

    String selected_ssid = "";
    String selected_pass = "";
    std::vector<String> found_ssids;
};

extern SystemState systemState;
extern NetworkState networkState;
extern SpotifyState spotifyState;

#endif //GLOBAL_STATE_H
