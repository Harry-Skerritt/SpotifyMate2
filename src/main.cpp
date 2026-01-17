#include <Arduino.h>
#include <LittleFS.h>
#include "hal/display.h"
#include "global_state.h"
#include "network/WifiManager.h"
#include "spotify/SpotifyManager.h"
#include "system/SystemManager.h"
#include "ui/UIManager.h"

SystemState systemState;
NetworkState networkState;
SpotifyState spotifyState;


// Task Handlers
void TaskGraphics(void *pvParameters);
void TaskSystem(void *pvParameters);

// Tasks
TaskHandle_t systemTaskHandle = NULL;

void setup() {
    Serial.begin(115200);
    delay(2000);
    halSetup();

#if LV_USE_SJPG == 0
    Serial.println("WARNING: LV_USE_SJPG is DISABLED in lv_conf.h!");
#else
    Serial.println("SUCCESS: JPEG Decoder is ENABLED.");
#endif

    UIManager::getInstance().init();

    if (!LittleFS.begin(true)) UIManager::getInstance().showFailure();
    delay(100);

    SystemManager::getInstance().init();
    delay(500);

    SpotifyManager::getInstance().init();

    // UI Task (Core 1)
    xTaskCreatePinnedToCore(TaskGraphics, "Graphics", 32768, NULL, 3, NULL, 1);

    // Network Task (Core 0)
    xTaskCreatePinnedToCore(TaskSystem, "System", 32768, NULL, 1, &systemTaskHandle, 0);
}

void handleHardResetCheck() {
    pinMode(0, INPUT_PULLUP);
    uint32_t start_time = millis();

    while(millis() - start_time < 3000) {
        if (digitalRead(0) == LOW) {
            Serial.println("DEBUG: Reset Triggered!");
            LittleFS.remove("/config.json");
            LittleFS.remove("/tokens.json");
            systemState.setup_complete = false;
            systemState.spotify_linked = false;
            UIManager::getInstance().showContextScreen("Resetting...");
            lv_timer_handler();
            vTaskDelay(pdMS_TO_TICKS(2000));
            ESP.restart();
        }
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}


// --- CORE 1: Handle Screen Updates ---
void TaskGraphics(void *pvParameters) {

    UIManager::getInstance().showSplashScreen();

    handleHardResetCheck();

    for (;;) {
        if (systemState.status != SYSTEM_STATUS_SLEEP) {

            if (spotifyState.status == SPOTIFY_READY) {
                UIManager::getInstance().setTrackProgress(
                    spotifyState.current_track_progress_ms,
                    spotifyState.current_track_duration_ms
                );
            }


            UIManager::getInstance().update();

            lv_timer_handler();
            vTaskDelay(pdMS_TO_TICKS(25));
        }
        else {
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}


// --- CORE 0: Handle Wi-Fi and API Logic ---
void TaskSystem(void *pvParameters) {
    uint32_t ulTaskNotifiedValue;

    for (;;) {

        if (xTaskNotifyWait(0, ULONG_MAX, &ulTaskNotifiedValue, pdMS_TO_TICKS(100)) == pdPASS) {

            if (ulTaskNotifiedValue & CMD_WIFI_SCAN) {
                WifiManager::getInstance().processScan();
            }

            if (ulTaskNotifiedValue & CMD_WIFI_CONN) {
                WifiManager::getInstance().processConnect();
            }

            if (ulTaskNotifiedValue & CMD_WIFI_RESET) {
                WifiManager::getInstance().processReset();
            }
        }

        // Non-Command Logic (e.g. Checking Wi-Fi status)
        WifiManager::getInstance().update();
        SystemManager::getInstance().update();


        // Only update if on WiFi
        if (networkState.wifi_connected) {
            // Handle sleeping
            if (systemState.status == SYSTEM_STATUS_IDLE) {
                Serial.println("SLEEP DEBUG: IN IDLE STATE");
                unsigned long idleTime = millis() - systemState.time_first_np;
                unsigned long currentTimeout = (spotifyState.current_track_id == "NOT_PLAYING")
                                           ? SLEEP_TIMEOUT_MS
                                           : PAUSE_SLEEP_TIMEOUT_MS;

                if (idleTime >= currentTimeout) {
                    Serial.println("SLEEP DEBUG: TIMEOUT ELAPSED - SLEEPING...");
                    systemState.status = SYSTEM_STATUS_SLEEP;
                    SystemManager::getInstance().enterSleepMode();
                }
            }

            SpotifyManager::getInstance().update();

            if (systemState.status != SYSTEM_STATUS_ACTIVE && spotifyState.is_playing) {
                systemState.status = SYSTEM_STATUS_ACTIVE;
                SystemManager::getInstance().exitSleepMode();
            }
        }

        int delayTime = (systemState.status == SYSTEM_STATUS_ACTIVE) ? 100 : 2000;
        vTaskDelay(pdMS_TO_TICKS(delayTime));
    }
}

void loop() {
    // In FreeRTOS projects, loop() is not used.
    vTaskDelete(NULL);
}