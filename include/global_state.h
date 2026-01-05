//
// Created by Harry Skerritt on 05/01/2026.
//

#ifndef GLOBAL_STATE_H
#define GLOBAL_STATE_H

#pragma once
#include <Arduino.h>

struct NetworkState {
    bool wifi_connected = false;
    String ip = "0.0.0.0";
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
