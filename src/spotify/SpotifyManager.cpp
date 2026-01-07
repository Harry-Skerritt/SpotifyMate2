//
// Created by Harry Skerritt on 06/01/2026.
//

#include "SpotifyManager.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <TJpg_Decoder.h>

#include "global_state.h"
#include "system/SystemManager.h"
#include "ui/UIManager.h"

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
            handleAuthCodeExchange();
            break;


        case SPOTIFY_NEED_LINK:
            if (!isServerRunning) {
                handleCodeWebServer();
            }
            break;


        default:
            break;

    }
}

void SpotifyManager::buildAuthURL() {

    String proxyUrl = "https://spotify-proxy-6cuziwrfx-harry-skerritts-projects.vercel.app/api/callback";
    String localIP = networkState.ip.c_str();

    std::vector<Spotify::Scope> scopes = {
        Spotify::Scope::UserReadEmail,
        Spotify::Scope::UserReadPrivate,
        Spotify::Scope::UserReadPlaybackState,
        Spotify::Scope::UserReadCurrentlyPlaying,
        Spotify::Scope::UserModifyPlaybackState,
        Spotify::Scope::UserReadRecentlyPlayed,
        Spotify::Scope::UserLibraryModify
    };

    spotifyState.auth_url = sp_auth->createAuthoriseURL(
    proxyUrl.c_str(),
        scopes,
    std::string(localIP.c_str())
    ).c_str();
}



void SpotifyManager::handleRefreshValidation() {
    Serial.println("Spotify: Attempting to refresh saved token...");

    if (sp_auth->begin(spotifyState.refresh_token.c_str())) {
        spotifyState.refresh_token = sp_auth->getRefreshToken().c_str();
        Serial.println("Spotify: Refresh successful!");
        spotifyState.status = SPOTIFY_READY;
    } else {
        Serial.println("Spotify: Refresh failed (Token expired or revoked)");
        spotifyState.status = SPOTIFY_LINK_ERROR;
    }
}



void SpotifyManager::handleCodeWebServer() {
    isServerRunning = true;

    xTaskCreatePinnedToCore(
        [](void* pvParameters) {
            SpotifyManager* manager = (SpotifyManager*)pvParameters;

            Serial.println("Spotify: Auth Server Task Started");
            String url = "http://" + networkState.ip;

            std::string code = Spotify::AuthServer::waitForCode(
                url.c_str(),
                8888
            );

            if (!code.empty()) {
                Serial.println("Spotify: Code received! Authing...");
                manager->temp_auth_code = code;
                spotifyState.status = SPOTIFY_AUTHENTICATING;
            } else {
                Serial.print("Spotify: Auth Server times out of failed to get code.");
            }

            manager->isServerRunning = false;
            Serial.println("Spotify: Auth Server Task Finished and Deleting");
            vTaskDelete(NULL);
        },
        "SpotAuth",
        8192,
        this,
        1,
        NULL,
        0
    );
}

void SpotifyManager::handleAuthCodeExchange() {
    Serial.println("Spotify: Exchanging code for tokens...");

    try {
        sp_auth->exchangeCode(temp_auth_code);
        sp_client = new Spotify::Client(*sp_auth);

        // Successful Exchange
        spotifyState.refresh_token = sp_auth->getRefreshToken().c_str();

        SystemManager::getInstance().writeSpotifyTokens();
        systemState.spotify_linked = true;
        SystemManager::getInstance().writeConfig();



        spotifyState.status = SPOTIFY_READY;
        Serial.println("Spotify: Login Successful!");
    } catch (Spotify::Exception& e) {
        Serial.println("Spotify: Login Failed!");
        Serial.println(e.what());
        spotifyState.status = SPOTIFY_ERROR;
    }

}

void SpotifyManager::loadAlbumArt(String &url) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    http.setUserAgent("ESP32-Spotify-Mate"); // Some servers block empty UAs

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            int len = http.getSize();
            if (len <= 0) return;

            uint8_t* temp_jpg = (uint8_t*)ps_malloc(len);
            if (!temp_jpg) return;

            // Use readBytesFully to ensure we get the whole file
            int downloaded = http.getStream().readBytes(temp_jpg, len);

            Serial.printf("Spotify: Downloaded %d of %d bytes\n", downloaded, len);

            if (downloaded > 0) {
                UIManager::getInstance().updateAlbumArt(temp_jpg, len);
            }

            free(temp_jpg);
        } else {
            Serial.printf("Spotify: HTTP Error %d\n", httpCode);
        }
        http.end();
    }
}


