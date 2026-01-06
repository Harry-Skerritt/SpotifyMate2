//
// Created by Harry Skerritt on 05/01/2026.
//

#ifndef GLOBAL_STATE_H
#define GLOBAL_STATE_H

#pragma once
#include <Arduino.h>
#include <vector>

#include "spotify/models/AuthModels.hpp"

struct NetworkState {
    // Current Wifi Status
    bool wifi_connected = false;
    String ip = "0.0.0.0";
    bool setup_complete = false;

    // Spotify Auth Flags
    bool spotify_linked = false; // From Config.json
    String spotify_auth_url = "";
    volatile bool show_qr_trigger = false;

    // Scanning Flags
    volatile bool start_scan_trigger = false;
    volatile bool scan_complete = false;
    std::vector<String> found_ssids;

    // Connection Flags
    volatile bool start_connect_trigger = false;
    String selected_ssid;
    String selected_pass;

    // WiFi Status Flags
    volatile bool failed_to_connect_trigger = false;
    volatile bool show_success_trigger = false;
    uint32_t success_shown_at = 0;
};

struct SystemState {

    volatile bool fatal_error_trigger = false;

    String current_track = "Idle";
    String artist = "Waiting...";
    int volume = 50;
    bool is_playing = false;
};



// Declare a global instance that all files can see
extern SystemState deviceState;
extern NetworkState networkState;
extern Spotify::AuthResponse spotifyAuth;
#endif //GLOBAL_STATE_H
