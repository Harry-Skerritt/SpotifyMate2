//
// Created by Harry Skerritt on 05/01/2026.
//

#ifndef UIMANAGER_H
#define UIMANAGER_H


#pragma once
#include <lvgl.h>

// Global Colours
#define SPOTIFY_GREEN lv_color_hex(0x1ED760)
#define BACKGROUND_GREY lv_color_hex(0x1E1E1E)
#define SPOTIFY_WHITE lv_color_hex(0xFFFFFF)

// Global fonts
LV_FONT_DECLARE(font_gotham_medium_20);
LV_FONT_DECLARE(font_gotham_medium_80);

// Different screens
void uiInit();
void uiWifiOnboarding();
void uiWifiConnectionError();
void ui_show_player();
void ui_update_player();


#endif //UIMANAGER_H
