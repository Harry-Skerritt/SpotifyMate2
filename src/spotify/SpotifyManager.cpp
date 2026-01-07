//
// Created by Harry Skerritt on 06/01/2026.
//

#include "SpotifyManager.h"

#include "global_state.h"


void SpotifyManager::init() {

    // Create the spotify auth object
    Spotify::ClientCredentials credentials;

    if (spotifyState.client_id.length() != 0) {
        credentials.client_id = spotifyState.client_id.c_str();
        credentials.client_secret = spotifyState.client_secret.c_str();
    } else {
        Serial.println("This might just be a timing error, or it could be fatal");
    }

    sp_auth = new Spotify::Auth(credentials);

}

void SpotifyManager::update() {

    switch (spotifyState.status) {

        case SPOTIFY_INITIALIZING:
            // We have a token in storage, but need to see if it's still valid
            handleRefreshValidation();
            break;

        case SPOTIFY_AUTHENTICATING:
            // The local webserver has received a 'code' and we are exchanging it
            //handleAuthCodeExchange();
            break;

        default:
            break;

    }
}


void SpotifyManager::handleRefreshValidation() {

}
