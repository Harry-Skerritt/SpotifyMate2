//
// Created by Harry Skerritt on 05/01/2026.
//

#include "uiManager.h"
#include "global_state.h"
#include "fonts/fontsManager.h"

static lv_obj_t* current_screen = nullptr;

// Helper
void uiClear() {
    if (current_screen != nullptr) {
        lv_obj_del(current_screen);
        current_screen = nullptr;
    }
    current_screen = lv_obj_create(NULL);

    if (current_screen == nullptr) {
        Serial.println("CRITICAL: Failed to create new screen object!");
        return;
    }

    lv_scr_load(current_screen);
}

// Helpers


void uiInit() {
    if (font_80 == nullptr || font_60 == nullptr || font_20 == nullptr) {
        Serial.println("UI Error: Splash screen fonts not loaded!");
        return;
    }

    Serial.println("Starting uiInit...");

    uiClear();

    // Safety 3: If uiClear failed, don't proceed
    if (current_screen == nullptr) return;

    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    lv_obj_t * cont = lv_obj_create(current_screen);
    if (!cont) { Serial.println("Failed to create container"); return; }

    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_center(cont);

    lv_obj_t* l1 = lv_label_create(cont);
    lv_label_set_text(l1, "Spotify");
    lv_obj_set_style_text_color(l1, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(l1, font_20, 0);

    lv_obj_t* l2 = lv_label_create(cont);
    lv_label_set_text(l2, "Mate");
    lv_obj_set_style_text_color(l2, SPOTIFY_GREEN, 0);
    lv_obj_set_style_text_font(l2, font_20, 0);

    Serial.println("uiInit finished successfully!");
}

void uiWifiConnectionError() {
    uiClear();

    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    lv_obj_t* title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "Oops! No network found");
    lv_obj_set_style_text_color(title, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(title, font_20, 0);
    lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, 0);
}