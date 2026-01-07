#include <Arduino.h>
#include <LittleFS.h>
#include "hal/display.h"
#include "global_state.h"
#include "network/WifiManager.h"
#include "system/SystemManager.h"
#include "ui/UIManager.h"

SystemState systemState;
NetworkState networkState;


// Task Handlers
void TaskGraphics(void *pvParameters);
void TaskSystem(void *pvParameters);

// Tasks
TaskHandle_t systemTaskHandle = NULL;

void setup() {
    Serial.begin(115200);
    delay(2000);
    halSetup();

    UIManager::getInstance().init();

    if (!LittleFS.begin(true)) UIManager::getInstance().showFailure();
    delay(100);

    SystemManager::getInstance().init();

    // UI Task (Core 1)
    xTaskCreatePinnedToCore(TaskGraphics, "Graphics", 32768, NULL, 3, NULL, 1);

    // Network Task (Core 0)
    xTaskCreatePinnedToCore(TaskSystem, "System", 16384, NULL, 1, &systemTaskHandle, 0);
}

void handleHardResetCheck() {
    pinMode(0, INPUT_PULLUP);
    uint32_t start_time = millis();

    while(millis() - start_time < 3000) {
        if (digitalRead(0) == LOW) {
            Serial.println("DEBUG: Reset Triggered!");
            LittleFS.remove("/config.json");
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

        UIManager::getInstance().update();

        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(25));
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

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void loop() {
    // In FreeRTOS projects, loop() is not used.
    vTaskDelete(NULL);
}