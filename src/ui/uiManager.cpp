//
// Created by Harry Skerritt on 05/01/2026.
//

#include "uiManager.h"
#include "global_state.h"

static lv_obj_t* current_screen = nullptr;

// Helper
void uiClear() {
    if (current_screen) {
        lv_obj_del(current_screen);
    }
    current_screen = lv_obj_create(NULL);
    lv_scr_load(current_screen);
}

// Helpers


void uiInit() {
    uiClear();

    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_center(cont);

    lv_obj_t* l1 = lv_label_create(cont);
    lv_label_set_text(l1, "Spotify");
    lv_obj_set_style_text_color(l1, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(l1, &font_gotham_medium_80, 0);

    lv_obj_t* l2 = lv_label_create(cont);
    lv_label_set_text(l2, "Mate");
    lv_obj_set_style_text_color(l2, SPOTIFY_GREEN, 0);
    lv_obj_set_style_text_font(l2, &font_gotham_medium_80, 0);
}

void uiWifiConnectionError() {
    uiClear();

    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    // Header Title
    lv_obj_t* title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "Oops! No network found");
    lv_obj_set_style_text_color(title, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(title, &font_gotham_medium_60, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    // Message
    lv_obj_t * body = lv_label_create(lv_scr_act());
    lv_label_set_text(body, "You don't appear to be\nconnected to the internet\n\nPlease check your connection\nand try again");
    lv_obj_set_style_text_color(body, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_text_font(body, &font_gotham_medium_40, 0);
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 30, 115);

    // Retry Button
    lv_obj_t* retry_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(retry_btn, 350, 70);
    lv_obj_align(retry_btn, LV_ALIGN_BOTTOM_LEFT, 38, -19);
    lv_obj_set_style_bg_color(retry_btn, SPOTIFY_GREEN, 0);
    lv_obj_set_style_bg_color(retry_btn, lv_color_hex(0x0D47A1), LV_STATE_PRESSED);
    lv_obj_set_style_radius(retry_btn, 50, 0);

    lv_obj_t* retry_btn_label = lv_label_create(retry_btn);
    lv_label_set_text(retry_btn_label, "Try Again");
    lv_obj_set_style_text_color(retry_btn_label, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(retry_btn_label, &font_gotham_medium_40, 0);
    lv_obj_center(retry_btn_label);

    // Reconnect Button
    lv_obj_t* reconnect_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(reconnect_btn, 350, 70);
    lv_obj_align(reconnect_btn, LV_ALIGN_BOTTOM_RIGHT, -38, -19);
    lv_obj_set_style_bg_color(reconnect_btn, BACKGROUND_GREY, 0);
    lv_obj_set_style_border_color(reconnect_btn, SPOTIFY_GREEN, 0);
    lv_obj_set_style_border_width(reconnect_btn, 3, 0);
    lv_obj_set_style_border_opa(reconnect_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(reconnect_btn, LV_BORDER_SIDE_FULL, 0);
    lv_obj_set_style_bg_color(reconnect_btn, lv_color_hex(0x0D47A1), LV_STATE_PRESSED);
    lv_obj_set_style_radius(reconnect_btn, 50, 0);

    lv_obj_t* reconnect_btn_label = lv_label_create(reconnect_btn);
    lv_label_set_text(reconnect_btn_label, "Reconnect");
    lv_obj_set_style_text_color(reconnect_btn_label, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(reconnect_btn_label, &font_gotham_medium_40, 0);
    lv_obj_center(reconnect_btn_label);


}