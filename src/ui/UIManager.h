//
// Created by Harry Skerritt on 06/01/2026.
//

#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <Arduino.h>
#include <lvgl.h>


// --- Global Colours ---
#define SPOTIFY_GREEN lv_color_hex(0x1ED760)
#define SPOTIFY_GREEN_DARKER lv_color_hex(0x1DB954)
#define BACKGROUND_GREY lv_color_hex(0x1E1E1E)
#define SPOTIFY_WHITE lv_color_hex(0xFFFFFF)
#define SPOTIFY_GREY lv_color_hex(0xCCCCCC)

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
    void update();

    void showSplashScreen();
    void showSpinner(const String& msg);
    void showFailure();
    void showContextScreen(const String &msg);

    // WiFi Screens
    void showOnboarding();
    void showNetworkList(const std::vector<String>& networks);
    void showPasswordEntry(const String& ssid);
    void showWifiError();

    // Spotify Screens
    void showSpotifyLinkError();
    void showSpotifyLinking(const char* auth_url);


private:
    UIManager() {}

    // Screen Management
    lv_obj_t* current_screen;
    void clearScreen();

    // Styles
    void initStyles();
    lv_style_t style_btn_base;
    lv_style_t style_btn_green;
    lv_style_t style_btn_outline;
    lv_style_transition_dsc_t trans_btn;

    lv_style_t style_network_card;


    // Internal Helpers
    lv_obj_t* createSpotifyBtn(lv_obj_t* parent, lv_event_cb_t cb, const char* text, lv_align_t align, int x, int y, bool is_green);
    lv_obj_t* createLogo(lv_obj_t* parent, const lv_font_t* font, lv_align_t align, int x, int y);
    lv_obj_t* createNetworkItem(lv_obj_t* parent, const char* ssid);
    lv_obj_t* createCustomQRCode(lv_obj_t* parent, const char* url, int size);

    void populateWifiList(lv_obj_t* list_cont, const std::vector<String>& networks);



    UIManager(const UIManager&) = delete;
    void operator=(const UIManager&) = delete;

};



#endif //UIMANAGER_H
