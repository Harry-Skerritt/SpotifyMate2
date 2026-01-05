#include <Arduino.h>
#include "hal/display.h"
#include "network/wifiManager.h"
#include "global_state.h"
#include "ui/uiManager.h"
#include "ui/fonts/fontsManager.h"


SystemState deviceState;
NetworkState networkState;

// Task Handlers
void TaskGraphics(void *pvParameters);
void TaskSystem(void *pvParameters);

bool last_wifi_state = false;

void setup() {
    Serial.begin(9600);
    // Give the USB CDC a moment to connect
    delay(2000);

    Serial.println("\n--- HARDWARE DIAGNOSTIC ---");

    if(psramInit()){
        Serial.printf("PSRAM: FOUND! Size: %u bytes\n", ESP.getPsramSize());
    } else {
        Serial.println("PSRAM: NOT FOUND. Hardware is not initializing.");
    }

    Serial.printf("Internal RAM Free: %u bytes\n", ESP.getFreeHeap());
    Serial.println("---------------------------\n");


    halSetup();
    Serial.println("Creating fonts...");
    initCustomFonts();

    Serial.println("Building UI...");
    uiInit();

    lv_timer_handler();
    Serial.println("UI Built.");

    // UI Task (Core 1)
    xTaskCreatePinnedToCore(TaskGraphics, "Graphics", 20480, NULL, 3, NULL, 1);

    // Network Task (Core 0)
    xTaskCreatePinnedToCore(TaskSystem, "System", 8192, NULL, 1, NULL, 0);
}

// --- CORE 1: Handle Screen Updates ---
void TaskGraphics(void *pvParameters) {
    //uiInit();

    for (;;) {
        if (networkState.wifi_connected != last_wifi_state) {
            last_wifi_state = networkState.wifi_connected;

            if (networkState.wifi_connected) {
                String ipStr = "Connected!\nIP: " + networkState.ip;
                Serial.println(ipStr.c_str());
            } else {
                Serial.println("Connecting to WiFi...");
            }
        }

        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// --- CORE 0: Handle Wi-Fi and API Logic ---
void TaskSystem(void *pvParameters) {
    wifiInit();

    for (;;) {
        if (WiFi.status() == WL_CONNECTED) {
            if (!networkState.wifi_connected) {
                networkState.wifi_connected = true;
                networkState.ip = WiFi.localIP().toString();
                Serial.println("Network Ready");
            }
        } else {
            networkState.wifi_connected = false;
        }


        if (networkState.wifi_connected) {
            // Spotify API polling will live here
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void loop() {
    // In FreeRTOS projects, loop() is not used.
    vTaskDelete(NULL);
}