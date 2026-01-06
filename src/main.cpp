#include <Arduino.h>
#include "hal/display.h"
#include "network/wifiManager.h"
#include "global_state.h"
#include "ui/uiManager.h"


SystemState deviceState;
NetworkState networkState;
TransitionFlags transitionFlags;

// Task Handlers
void TaskGraphics(void *pvParameters);
void TaskSystem(void *pvParameters);



void setup() {
    Serial.begin(9600);
    halSetup();
    UIManager::getInstance().init();

    if (!LittleFS.begin(true)) transitionFlags.fatal_error_trigger = true;

    // UI Task (Core 1)
    xTaskCreatePinnedToCore(TaskGraphics, "Graphics", 32768, NULL, 3, NULL, 1);

    // Network Task (Core 0)
    xTaskCreatePinnedToCore(TaskSystem, "System", 16384, NULL, 1, NULL, 0);
}

void handleHardResetCheck() {
    pinMode(0, INPUT_PULLUP);
    uint32_t start_time = millis();

    while(millis() - start_time < 3000) {
        if (digitalRead(0) == LOW) {
            Serial.println("DEBUG: Reset Triggered!");
            LittleFS.remove("/config.json");
            //LittleFS.remove("/secrets.json"); // Wipe Spotify too
            deviceState.setup_complete = false;
            deviceState.spotify_linked = false;
            UIManager::getInstance().showContextScreen("Reseting...");
            lv_timer_handler();
            vTaskDelay(pdMS_TO_TICKS(2000));
            ESP.restart(); // Cleanest way to reset after wipe
        }
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}


// --- CORE 1: Handle Screen Updates ---
void TaskGraphics(void *pvParameters) {
    UIManager::getInstance().showSplashScreen();
    lv_timer_handler();

    handleHardResetCheck();

    if (!deviceState.setup_complete) {
        Serial.println("System: No config found, forcing onboarding UI");
        UIManager::getInstance().showWifiOnboarding();
    }

    for (;;) {
        UIManager::getInstance().update(networkState, deviceState, transitionFlags);

        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(25));
    }
}


// --- CORE 0: Handle Wi-Fi and API Logic ---
void TaskSystem(void *pvParameters) {
   WifiManager::getInstance().init();

    for (;;) {
        WifiManager::getInstance().update();

        if (networkState.wifi_connected) {
            // Update Spotify Manager
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void loop() {
    // In FreeRTOS projects, loop() is not used.
    vTaskDelete(NULL);
}