//
// Created by Harry Skerritt on 05/01/2026.
//

#ifndef GLOBAL_STATE_H
#define GLOBAL_STATE_H

#pragma once
#include <Arduino.h>
#include <vector>

struct NetworkState {
    // Current Wifi Status
    bool wifi_connected = false;
    String ip = "0.0.0.0";

    bool has_been_connected = false;

    // Scanning Flags
    volatile bool start_scan_trigger = false;
    volatile bool scan_complete = false;
    std::vector<String> found_ssids;

    // Connection Flags
    volatile bool start_connect_trigger = false;
    String selected_ssid;
    String selected_pass;

    // Failed Flags
    volatile bool failed_to_connect_trigger = false;
};

struct SystemState {
    String current_track = "Idle";
    String artist = "Waiting...";
    int volume = 50;
    bool is_playing = false;
};

// Declare a global instance that all files can see
extern SystemState deviceState;
extern NetworkState networkState;

#endif //GLOBAL_STATE_H
