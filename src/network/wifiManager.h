//
// Created by Harry Skerritt on 05/01/2026.
//

#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>


void loadWifiConfig();
void wifiInit();


extern String ssid;
extern String pass;




#endif //WIFIMANAGER_H
