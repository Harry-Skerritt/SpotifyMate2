//
// Created by Harry Skerritt on 06/01/2026.
//

#include "spotifyManager.h"
#include <LittleFS.h>
#include <WiFi.h>

#include "global_state.h"

Spotify::Auth spotify({ "", "" });

void SpotifyManager::init() {
    loadSpotifySecrets();

    if (spotifyAuth != nullptr) {
        // Is linked?
        if (refresh_token.length() > 0) {
            Serial.println("Spotify: Loaded existing refresh token.");
            networkState.spotify_linked = true;
            //networkState.show_success_trigger = true;
        } else {
            Serial.println("Spotify: No saved token. Starting Linking");

            String ip = WiFi.localIP().toString().c_str();
            String redirect = "http://" + ip + ":8888/callback";

            networkState.spotify_auth_url = spotifyAuth->createAuthoriseURL(redirect.c_str(),
                {
                    Spotify::Scope::UserReadPrivate,
                    Spotify::Scope::UserReadEmail,
                    Spotify::Scope::UserReadCurrentlyPlaying,
                    Spotify::Scope::UserReadPlaybackState,
                    Spotify::Scope::UserReadPlaybackPosition
                }
            ).c_str();
            networkState.spotify_linked = false;
            networkState.show_qr_trigger = true;
        }
    }
}

// Helpers
void SpotifyManager::loadSpotifySecrets() {
    if (!LittleFS.exists("/secrets.json")) {
        Serial.println("Spotify: secrets.json NOT FOUND!");
        deviceState.fatal_error_trigger = true;
        return;
    }

    File file = LittleFS.open("/secrets.json", "r");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("Spotify: Failed to parse secrets.json");
        deviceState.fatal_error_trigger = true;
        return;
    }

    String id = doc["spotify_client_id"].as<String>();
    String secret = doc["spotify_client_secret"].as<String>();

    // Check for a token
    refresh_token = doc["spotify_refresh_token"].as<String>();

    if (id.length() > 0 && secret.length() > 0) {
        // Dynamically create the Auth object
        spotifyAuth = new Spotify::Auth({id.c_str(), secret.c_str()});
        Serial.println("Spotify: Auth object ready with secrets.");
    }
}


