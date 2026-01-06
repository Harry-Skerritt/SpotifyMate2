//
// Created by Harry Skerritt on 05/01/2026.
//

#ifndef GLOBAL_STATE_H
#define GLOBAL_STATE_H

#pragma once
#include <Arduino.h>
#include <vector>

struct NetworkState {
    bool wifi_connected = false;
    String ip = "0.0.0.0";
    String selected_ssid;
    String selected_pass;
    std::vector<String> found_ssids;
};

struct TransitionFlags {
    // Scanning Flags
    volatile bool start_scan_trigger = false;
    volatile bool scan_complete = false;

    // Connection Flags
    volatile bool is_connecting = false;
    volatile bool start_connect_trigger = false;
    volatile bool failed_to_connect_trigger = false;
    volatile bool show_success_trigger = false;

    // Error Flags
    volatile bool fatal_error_trigger = false;

    uint32_t success_shown_at = 0;
};

struct SystemState {
    bool setup_complete = false;
    bool spotify_linked = false;
    String spotify_auth_url = "";

    // Player Data
    String current_track = "Idle";
    String artist = "Waiting...";
    int volume = 50;
    bool is_playing = false;
};

// Declare a global instance that all files can see
extern SystemState deviceState;
extern NetworkState networkState;
extern TransitionFlags transitionFlags;

#endif //GLOBAL_STATE_H
