//
// Created by Harry Skerritt on 06/01/2026.
//

#ifndef SPOTIFYMANAGER_H
#define SPOTIFYMANAGER_H

#include <spotify/spotify.hpp>


class SpotifyManager {
public:

    static SpotifyManager& getInstance() {
        static SpotifyManager instance;
        return instance;
    }

    void init();
    void update();

private:

    SpotifyManager() {}

    Spotify::Auth* sp_auth = nullptr;

    void handleRefreshValidation();


    SpotifyManager(const SpotifyManager&) = delete;
    void operator=(const SpotifyManager&) = delete;
};



#endif //SPOTIFYMANAGER_H
