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
    SPOTIFY_LINK_ERROR,    // For errors where re-linking is necessary
    SPOTIFY_ERROR          // Auth failed or Token revoked
};

struct SpotifyState {
    SpotifyStatus status = SPOTIFY_IDLE;
    String client_id;
    String client_secret;
    String refresh_token;
    String auth_url = "";

    String current_track_id = "";
    String current_track_title =  "Nothing Playing";
    String current_track_artist = "-";
    String current_track_url = "";
    String current_track_device_name = "No Device";
    int current_track_duration_ms = 0;
    int current_track_progress_ms = 0;
    bool is_playing = false;
    uint32_t album_background_cover  = 0x3F5C67;
    //uint32_t album_average_colour  = 0xB1A69D;

    bool needs_art_update = false;
    bool needs_text_update = false;
};


enum SystemStatus {
    SYSTEM_STATUS_IDLE,
    SYSTEM_STATUS_ACTIVE,
    SYSTEM_STATUS_SLEEP,
};

struct SystemState {
    bool setup_complete = false;
    bool spotify_linked = false;

    SystemStatus status = SYSTEM_STATUS_ACTIVE;
    unsigned long time_first_np = 0; // First time not playing is hit
};

/*
unsigned long const SLEEP_TIMEOUT_MS = (3 * 60000); // 3 Mins
unsigned long const PAUSE_SLEEP_TIMEOUT_MS = (5 * 60000); // 5 Mins
*/

// DEBUG VALUES
unsigned long const SLEEP_TIMEOUT_MS = (0.3 * 60000); // 20 Secs
unsigned long const PAUSE_SLEEP_TIMEOUT_MS = (0.5 * 60000); // 30 Secs

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
