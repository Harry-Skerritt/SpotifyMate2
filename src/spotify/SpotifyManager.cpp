//
// Created by Harry Skerritt on 06/01/2026.
//

#include "SpotifyManager.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "global_state.h"
#include "system/SystemManager.h"
#include "ui/UIManager.h"

String sanitizeString(String str) {
    str.replace("\xe2\x80\x99", "'");
    str.replace("&#39;", "'");
    return str;
}

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

        case SPOTIFY_READY:
        {
            static uint32_t lastPollTime = 0;
            uint32_t now = millis();

            if (now - lastPollTime > 2000) {
                lastPollTime = now;
                getCurrentlyPlaying();
            }
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

        sp_client = new Spotify::Client(*sp_auth);

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

void SpotifyManager::loadAlbumArt(String &url, short target_size) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setUserAgent("ESP32-Spotify-Mate");

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            int len = http.getSize();
            if (len <= 0) return;

            uint8_t* temp_jpg = (uint8_t*)ps_malloc(len);
            if (!temp_jpg) return;

            WiFiClient* stream = http.getStreamPtr();
            int downloaded = 0;
            unsigned long timeout = millis();

            // Loop until we have all the bytes or hit a timeout (5 seconds)
            while (http.connected() && downloaded < len && (millis() - timeout < 5000)) {
                size_t size = stream->available();
                if (size) {
                    int c = stream->readBytes(&temp_jpg[downloaded], size);
                    downloaded += c;
                    timeout = millis(); // Reset timeout on successful read
                }
                delay(1); // Let the network stack breathe
            }

            Serial.printf("Spotify: Downloaded %d of %d bytes\n", downloaded, len);

            if (downloaded == len) {
                // Only pass to UI if we have 100% of the file
                UIManager::getInstance().updateAlbumArt(temp_jpg, len, target_size);
            } else {
                Serial.println("Spotify: Download incomplete, discarding buffer");
                free(temp_jpg);
            }
        }
        http.end();
    }
}


bool SpotifyManager::getCurrentlyPlaying() {
    if (sp_client == nullptr) return false;

   // Serial.println("Spotify: Polling...");

    try {
        auto pb = sp_client->player().getPlaybackState();

        if (pb.has_value()) {
            spotifyState.current_track_device_name = sanitizeString(pb->device.name.c_str());
            spotifyState.current_track_progress_ms = pb->progress_ms;
            spotifyState.is_playing = pb->is_playing;

            auto track = pb->asTrack();
            if (track) {
                String newId = sanitizeString(track->id.c_str());
                String newUrl = track->album.images.at(0).url.c_str();

                bool trackChanged = (spotifyState.current_track_id != newId);
                bool urlChanged = (spotifyState.current_track_url != newUrl);

                if (trackChanged || urlChanged) {
                    Serial.println("Spotify: Change detected...");
                    spotifyState.current_track_id = newId;
                    spotifyState.current_track_title = sanitizeString(track->name.c_str());
                    spotifyState.current_track_artist = sanitizeString(track->artists.at(0).name.c_str());
                    spotifyState.current_track_duration_ms = track->duration_ms; // Total length

                    if (urlChanged) {
                        Serial.println("Spotify: New Album Art detected...");
                        try {
                            Spotify::Extensions::ImagePalette palette =
                                Spotify::Extensions::VisualAPI().getImagePalette(newUrl.c_str());

                            spotifyState.album_average_colour = calculateSmartBackground(palette);
                        } catch (...) {
                            Serial.println("Spotify: Image Palette Error - Couldn't get colour");
                            spotifyState.album_average_colour = 0x191414; // Fallback
                        }
                        spotifyState.current_track_url = newUrl;
                        spotifyState.needs_art_update = true;
                        spotifyState.needs_text_update = false;
                    } else {
                        spotifyState.needs_text_update = true;
                    }
                }
            }
            return true;

        } else {
            // Check if we were previously playing something.
            // Only update if we aren't already in the NOT_PLAYING state.
            if (spotifyState.current_track_id != "NOT_PLAYING") {
                Serial.println("Spotify: Nothing is playing");
                spotifyState.current_track_id = "NOT_PLAYING";
                spotifyState.current_track_title = "Nothing Playing";
                spotifyState.current_track_artist = "-";
                spotifyState.current_track_device_name = "No Device";

                spotifyState.current_track_url = "https://raw.githubusercontent.com/Harry-Skerritt/files/refs/heads/main/not_playing_album.jpg";
                spotifyState.album_average_colour = 0x13B94E;

                spotifyState.current_track_progress_ms = 0;
                spotifyState.current_track_duration_ms = 0;
                spotifyState.is_playing = false;

                spotifyState.needs_art_update = true;
                spotifyState.needs_text_update = false;
            }
            return true;
        }
    } catch (Spotify::Exception& e) {
        Serial.println("Spotify: Error getting playing state!");
        Serial.println(e.what());
    }
    return false;
}



// Helper
uint32_t SpotifyManager::calculateSmartBackground(const Spotify::Extensions::ImagePalette& palette) {

    float luma = (0.299f * palette.average.r) +
                 (0.587f * palette.average.g) +
                 (0.114f * palette.average.b);

    if (luma > 200) {
        // Too bright
        return palette.darker_1.to0x();
    }
    if (luma < 40) {
        // Too dark
        return palette.lighter_1.to0x();
    }

    // Fine
    return palette.average.to0x();
}

