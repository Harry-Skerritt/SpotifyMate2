//
// Created by Harry Skerritt on 06/01/2026.
//

#ifndef SPOTIFYMANAGER_H
#define SPOTIFYMANAGER_H

#include <Arduino.h>
#include <spotify/spotify.hpp>
#include <ArduinoJson.h>

class SpotifyManager {

public:
    static SpotifyManager& getInstance() {
        static SpotifyManager instance;
        return instance;
    }


    void init();

private:
    Spotify::Auth* spotifyAuth = nullptr;
    String refresh_token;
    String access_token;

    void loadSpotifySecrets();
};



#endif //SPOTIFYMANAGER_H
