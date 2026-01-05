#include <Arduino.h>
#include "hal/display.h"
#include "network/wifiManager.h"
#include "global_state.h"


SystemState deviceState;
NetworkState networkState;

// Task Handlers
void TaskGraphics(void *pvParameters);
void TaskSystem(void *pvParameters);

bool last_wifi_state = false;

void setup() {
    Serial.begin(115200);

    halSetup();

    // UI Task (Core 1)
    xTaskCreatePinnedToCore(TaskGraphics, "Graphics", 8192, NULL, 3, NULL, 1);

    // Network Task (Core 0)
    xTaskCreatePinnedToCore(TaskSystem, "System", 8192, NULL, 1, NULL, 0);
}

// --- CORE 1: Handle Screen Updates ---
void TaskGraphics(void *pvParameters) {
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Initializing...");
    lv_obj_center(label);

    for (;;) {
        if (networkState.wifi_connected != last_wifi_state) {
            last_wifi_state = networkState.wifi_connected;

            if (networkState.wifi_connected) {
                lv_label_set_text_fmt(label, "Connected!\nIP: %s", networkState.ip.c_str());
            } else {
                lv_label_set_text(label, "Connecting to WiFi...");
            }
            lv_obj_center(label);
        }

        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
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