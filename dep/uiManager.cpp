//
// Created by Harry Skerritt on 05/01/2026.
//

#include "../src/ui/uiManager.h"
#include "global_state.h"
#include <vector>
#include <LittleFS.h>





/*
// Spotify Linking
void UIManager::showSpotifyLinking(const char* auth_url) {
    clearScreen();

    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    // Header Title
    lv_obj_t* title = lv_label_create(current_screen);
    lv_label_set_text(title, "Link with Spotify");
    lv_obj_set_style_text_color(title, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(title, &font_gotham_medium_60, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    // Message
    lv_obj_t * body = lv_label_create(current_screen);
    lv_label_set_text(body, "Scan the QR code and follow the \ninstructions to link your Spotify!");
    lv_obj_set_style_text_color(body, SPOTIFY_GREY, 0);
    lv_obj_set_style_text_font(body, &font_gotham_medium_40, 0);
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 30, 115);

    // QR Code
    lv_obj_t* auth_qr = createCustomQRCode(current_screen, auth_url, 215);
    lv_obj_align(auth_qr, LV_ALIGN_BOTTOM_MID, 0, -27);

}


// Main Player
void UIManager::showMainPlayer() {
    clearScreen();

    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    // Header Title
    lv_obj_t* title = lv_label_create(current_screen);
    lv_label_set_text(title, "Player");
    lv_obj_set_style_text_color(title, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(title, &font_gotham_medium_60, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

}

*/
