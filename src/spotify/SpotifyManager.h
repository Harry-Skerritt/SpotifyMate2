//
// Created by Harry Skerritt on 06/01/2026.
//

#ifndef SPOTIFYMANAGER_H
#define SPOTIFYMANAGER_H

#include <spotify/spotify.hpp>
#include <lvgl.h>

extern  lv_img_dsc_t spotify_img_dsc;
extern uint8_t* compressed_buffer;

class SpotifyManager {
public:

    static SpotifyManager& getInstance() {
        static SpotifyManager instance;
        return instance;
    }

    void init();
    void update();

    void buildAuthURL();

    void loadAlbumArt(String& url, short target_size);

    bool getCurrentlyPlaying();


private:
    SpotifyManager() {}

    Spotify::Auth* sp_auth = nullptr;
    Spotify::Client* sp_client = nullptr;


    // Web Server + Getting Code
    bool isServerRunning = false;
    std::string temp_auth_code;
    void handleCodeWebServer();
    void handleAuthCodeExchange();

    void handleRefreshValidation();


    SpotifyManager(const SpotifyManager&) = delete;
    void operator=(const SpotifyManager&) = delete;
};



#endif //SPOTIFYMANAGER_H
