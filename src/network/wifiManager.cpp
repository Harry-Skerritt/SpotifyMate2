//
// Created by Harry Skerritt on 05/01/2026.
//

#include "wifiManager.h"


String ssid, pass;

void loadWifiConfig() {
    if (!LittleFS.begin(true)) { // 'true' forces format if mount fails
        Serial.println("LittleFS Mount Failed");
        return;
    }

    File file = LittleFS.open("/config.json", "r");

    if (file) {
        JsonDocument doc;
        deserializeJson(doc, file);
        ssid = doc["ssid"].as<String>();
        pass = doc["password"].as<String>();
        file.close();
    }

}

void wifiInit() {
    loadWifiConfig();


    if (ssid.length() > 0 && pass.length() > 0) {
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        WiFi.begin(ssid.c_str(), pass.c_str());
    }


}

