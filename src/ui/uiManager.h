//
// Created by Harry Skerritt on 05/01/2026.
//

#ifndef UIMANAGER_H
#define UIMANAGER_H


#pragma once
#include <lvgl.h>


// --- Global Colours ---
#define SPOTIFY_GREEN lv_color_hex(0x1ED760)
#define SPOTIFY_GREEN_DARKER lv_color_hex(0x1DB954)
#define BACKGROUND_GREY lv_color_hex(0x1E1E1E)
#define SPOTIFY_WHITE lv_color_hex(0xFFFFFF)

// --- Global fonts ---
LV_FONT_DECLARE(font_gotham_medium_20);
LV_FONT_DECLARE(font_gotham_medium_40);
LV_FONT_DECLARE(font_gotham_medium_60);
LV_FONT_DECLARE(font_gotham_medium_80);


class UIManager {
public:

    static UIManager& getInstance() {
        static UIManager instance;
        return instance;
    }


    void init();

    void showFailure();
    void showSplashScreen();
    void showWifiConnectionError();

private:
    // Constructor for singleton
    UIManager() : current_screen(nullptr) {}

    // Screen Management
    lv_obj_t* current_screen;
    void clearScreen();

    // Styles
    lv_style_t style_btn_base;
    lv_style_t style_btn_green;
    lv_style_t style_btn_outline;
    lv_style_transition_dsc_t trans_btn;

    // Internal Helpers
    void initStyles();
    lv_obj_t* createSpotifyBtn(lv_obj_t* parent, lv_event_cb_t cb, const char* text, lv_align_t align, int x, int y, bool is_green);
    lv_obj_t* createLogo(lv_obj_t* parent, const lv_font_t* font, lv_align_t align, int x, int y);

    // Prevent copying
    UIManager(const UIManager&) = delete;
    void operator=(const UIManager&) = delete;
};

#endif //UIMANAGER_H
