#include <Arduino.h>
#include "hal/display.h"
#include "network/wifiManager.h"
#include "global_state.h"
#include "ui/uiManager.h"


SystemState deviceState;
NetworkState networkState;

// Task Handlers
void TaskGraphics(void *pvParameters);
void TaskSystem(void *pvParameters);

bool last_wifi_state = false;

void setup() {
    Serial.begin(9600);

    halSetup();
    UIManager::getInstance().init();

    // UI Task (Core 1)
    xTaskCreatePinnedToCore(TaskGraphics, "Graphics", 32768, NULL, 3, NULL, 1);

    // Network Task (Core 0)
    xTaskCreatePinnedToCore(TaskSystem, "System", 8192, NULL, 1, NULL, 0);
}


bool is_scanning_ui_active = false;
// --- CORE 1: Handle Screen Updates ---
void TaskGraphics(void *pvParameters) {
    UIManager::getInstance().showSplashScreen();

    uint32_t start_time = millis();
    while(millis() - start_time < 1000) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(15));
    }

    UIManager::getInstance().showWifiOnboarding();

    for (;;) {

        // Handle Network Scanning
        if (networkState.start_scan_trigger && !is_scanning_ui_active) {
            is_scanning_ui_active = true;
            UIManager::getInstance().showWifiScanning();
        }

        // Handle showing connections
        if (networkState.scan_complete) {
            networkState.scan_complete = false;
            is_scanning_ui_active = false;
            UIManager::getInstance().showWifiConnections(networkState.found_ssids);
        }

        // Handle connecting
        if (networkState.start_connect_trigger) {
            UIManager::getInstance().showWifiConnecting();
        }

        // Handle connection error

        if (networkState.failed_to_connect_trigger) {
            UIManager::getInstance().showWifiConnectionError();
            networkState.failed_to_connect_trigger = false;
        }


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
        vTaskDelay(pdMS_TO_TICKS(25));
    }
}

// --- CORE 0: Handle Wi-Fi and API Logic ---
void TaskSystem(void *pvParameters) {
    //WifiManager::getInstance().init();

    for (;;) {

        WifiManager::getInstance().handleAsyncScan();

        if (networkState.start_connect_trigger) {
            Serial.println("Sytem: Attemping to connect...");
            WiFi.begin(networkState.selected_ssid.c_str(), networkState.selected_pass.c_str());
            networkState.has_been_connected = true;
            networkState.start_connect_trigger = false;
        }

        if (WiFi.status() == WL_CONNECTED) {
            if (!networkState.wifi_connected) {
                networkState.wifi_connected = true;
                networkState.ip = WiFi.localIP().toString();
                Serial.println("Network Ready");

                // If on wifi then this will transition somewhere else
                UIManager::getInstance().showContextScreen("Connected!");
            }
        } else if (networkState.has_been_connected) {
            // If not connected, but has ben previously then go to the failed screen
            networkState.wifi_connected = false;
            networkState.failed_to_connect_trigger = true;
        } else {
            Serial.println("WiFi not connected and never has been");
        }

        if (networkState.wifi_connected) {
            // Spotify API polling will live here
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void loop() {
    // In FreeRTOS projects, loop() is not used.
    vTaskDelete(NULL);
}