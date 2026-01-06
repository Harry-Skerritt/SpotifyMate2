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

    if (!LittleFS.begin(true)) { // 'true' forces format if mount fails
        Serial.println("LittleFS Mount Failed");
        deviceState.fatal_error_trigger = true;
    }

    // UI Task (Core 1)
    xTaskCreatePinnedToCore(TaskGraphics, "Graphics", 32768, NULL, 3, NULL, 1);

    // Network Task (Core 0)
    xTaskCreatePinnedToCore(TaskSystem, "System", 8192, NULL, 1, NULL, 0);
}


bool is_scanning_ui_active = false;
// --- CORE 1: Handle Screen Updates ---
void TaskGraphics(void *pvParameters) {
    pinMode(0, INPUT_PULLUP);

    UIManager::getInstance().showSplashScreen();


    // Reset Logic
    uint32_t start_time = millis();
    bool reset_triggered = false;

    while(millis() - start_time < 3000) {
        if (digitalRead(0) == LOW && !reset_triggered) {
            Serial.println("DEBUG: Reset Button Held! Wiping Config...");

            if (LittleFS.remove("/config.json")) {
                networkState.setup_complete = false;
                UIManager::getInstance().showContextScreen("Resetting...");
                reset_triggered = true; // Mark that we are done
            } else {
                Serial.println("File delete failed - might not exist");
                UIManager::getInstance().showContextScreen("No config to reset!");
                reset_triggered = true;
            }
        }

        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(15));

        // Exit the 3-second window early if the user triggered a reset
        if (reset_triggered) {
            vTaskDelay(pdMS_TO_TICKS(1500)); // Brief pause so they can actually read the "Resetting" message
            break;
        }
    }

    WifiManager::getInstance().init(); // Init here for flags

    if (networkState.setup_complete) {
        // Have saved creds so connect
        UIManager::getInstance().showWifiConnecting();
    } else {
        // Fresh - needs onboarding
        UIManager::getInstance().showWifiOnboarding();
    }


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

        // Handle successful connection
        if (networkState.show_success_trigger) {
            UIManager::getInstance().showContextScreen("Connected!");
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


unsigned long connect_start_time = 0;
bool is_connecting = false;
// --- CORE 0: Handle Wi-Fi and API Logic ---
void TaskSystem(void *pvParameters) {
    // If init() starts a connection, mark us as connecting
    if (networkState.setup_complete) {
        is_connecting = true;
        connect_start_time = millis();
    }

    for (;;) {
        WifiManager::getInstance().handleAsyncScan();

        // Handle User-Triggered Connection (from Keyboard)
        if (networkState.start_connect_trigger) {
            Serial.println("Sytem: Attemping to connect...");
            WiFi.begin(networkState.selected_ssid.c_str(), networkState.selected_pass.c_str());
            networkState.start_connect_trigger = false;
            is_connecting = true;
            connect_start_time = millis();
        }

        wl_status_t status = WiFi.status();

        if (status== WL_CONNECTED) {
            if (!networkState.wifi_connected) {
                networkState.wifi_connected = true;
                is_connecting = false;
                networkState.ip = WiFi.localIP().toString();
                Serial.println("Network Ready");

                if (networkState.selected_ssid.length() > 0) {
                    WifiManager::getInstance().saveWifiConfig(networkState.selected_ssid.c_str(), networkState.selected_pass.c_str());
                }

                // Success!
                networkState.show_success_trigger = true;
            }
        } else if (is_connecting && (millis() - connect_start_time > 15000)) {
            // Trigger failure is trying for 15 seconds
            networkState.wifi_connected = false;
            networkState.failed_to_connect_trigger = true;
            is_connecting = false;
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