//
// Created by Harry Skerritt on 05/01/2026.
//

#include "uiManager.h"
#include "global_state.h"

// Styles
static lv_style_t style_btn_green_main;
static lv_style_t style_btn_green_pressed;
static lv_style_transition_dsc_t trans_btn;

static lv_obj_t* current_screen = nullptr;

// Helper
void uiClear() {
    if (current_screen) {
        lv_obj_del(current_screen);
    }
    current_screen = lv_obj_create(NULL);
    lv_scr_load(current_screen);
}


void uiStylesInit() {
    // ----------------------
    // --- Spotify Button ---
    // ----------------------
    // 1. Prepare the Transition (for smooth fading)
    static const lv_style_prop_t props[] = {LV_STYLE_BG_COLOR, LV_STYLE_TRANSFORM_ZOOM, (lv_style_prop_t)0};
    lv_style_transition_dsc_init(&trans_btn, props, lv_anim_path_linear, 100, 0, NULL);

    // 2. The Main Style (Normal State)
    lv_style_init(&style_btn_green_main);
    lv_style_set_bg_color(&style_btn_green_main, lv_color_hex(0x1DB954));
    lv_style_set_bg_opa(&style_btn_green_main, LV_OPA_COVER);
    lv_style_set_radius(&style_btn_green_main, 50);
    lv_style_set_text_color(&style_btn_green_main, lv_color_hex(0xFFFFFF));
    lv_style_set_transition(&style_btn_green_main, &trans_btn); // Apply fade here

    // 3. The Pressed Style (The changes that happen on touch)
    lv_style_init(&style_btn_green_pressed);
    lv_style_set_bg_color(&style_btn_green_pressed, lv_color_hex(0x1ED760)); // Brighter green
    lv_style_set_transform_zoom(&style_btn_green_pressed, 245);
}

// --- Button Handler for Error WiFi ---
static lv_obj_t* retry_btn_ptr;
static lv_obj_t* reconnect_btn_ptr;
static void wifi_error_event_handler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e); // What happened?
    lv_obj_t * btn = lv_event_get_target(e);     // Which button was it?

    if(code == LV_EVENT_CLICKED) {
        Serial.println("Button Clicked!");
        if (btn == retry_btn_ptr) {
            lv_obj_t * label = lv_obj_get_child(btn, 0);
            lv_label_set_text(label, "Retrying...");
        }
        else if (btn == reconnect_btn_ptr) {
            lv_obj_t * label = lv_obj_get_child(btn, 0);
            lv_label_set_text(label, "Connecting...");
        }
    }
}

void uiSplashScreen() {
    uiClear();

    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    lv_obj_t * cont = lv_obj_create(current_screen);
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
    lv_obj_t* title = lv_label_create(current_screen);
    lv_label_set_text(title, "Oops! No network found");
    lv_obj_set_style_text_color(title, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(title, &font_gotham_medium_60, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    // Message
    lv_obj_t * body = lv_label_create(current_screen);
    lv_label_set_text(body, "You don't appear to be\nconnected to the internet\n\nPlease check your connection\nand try again");
    lv_obj_set_style_text_color(body, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_text_font(body, &font_gotham_medium_40, 0);
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 30, 115);

    // Retry Button
    retry_btn_ptr = lv_btn_create(current_screen);
    lv_obj_set_size(retry_btn_ptr, 350, 70);
    lv_obj_align(retry_btn_ptr, LV_ALIGN_BOTTOM_LEFT, 38, -19);
    lv_obj_add_style(retry_btn_ptr, &style_btn_green_main, LV_STATE_DEFAULT);
    lv_obj_add_style(retry_btn_ptr, &style_btn_green_pressed, LV_STATE_PRESSED);
    lv_obj_add_event_cb(retry_btn_ptr, wifi_error_event_handler, LV_EVENT_ALL, NULL);

    lv_obj_t* retry_btn_label = lv_label_create(retry_btn_ptr);
    lv_label_set_text(retry_btn_label, "Try Again");
    lv_obj_set_style_text_color(retry_btn_label, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(retry_btn_label, &font_gotham_medium_40, 0);
    lv_obj_center(retry_btn_label);

    // Reconnect Button
    reconnect_btn_ptr = lv_btn_create(current_screen);
    lv_obj_set_size(reconnect_btn_ptr, 350, 70);
    lv_obj_align(reconnect_btn_ptr, LV_ALIGN_BOTTOM_RIGHT, -38, -19);
    lv_obj_set_style_bg_color(reconnect_btn_ptr, BACKGROUND_GREY, 0);
    lv_obj_set_style_border_color(reconnect_btn_ptr, SPOTIFY_GREEN, 0);
    lv_obj_set_style_border_width(reconnect_btn_ptr, 3, 0);
    lv_obj_set_style_border_opa(reconnect_btn_ptr, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(reconnect_btn_ptr, LV_BORDER_SIDE_FULL, 0);
    lv_obj_set_style_border_color(reconnect_btn_ptr, lv_color_hex(0x1ED760), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(reconnect_btn_ptr, LV_OPA_30, LV_STATE_PRESSED);
    lv_obj_set_style_radius(reconnect_btn_ptr, 50, 0);

    lv_obj_t* reconnect_btn_label = lv_label_create(reconnect_btn_ptr);
    lv_label_set_text(reconnect_btn_label, "Reconnect");
    lv_obj_set_style_text_color(reconnect_btn_label, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(reconnect_btn_label, &font_gotham_medium_40, 0);
    lv_obj_center(reconnect_btn_label);
}