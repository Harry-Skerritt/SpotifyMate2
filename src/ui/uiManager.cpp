//
// Created by Harry Skerritt on 05/01/2026.
//

#include "uiManager.h"
#include "global_state.h"



// --- Button Callbacks ---
static lv_obj_t* wifi_error_retry_btn_ptr;
static lv_obj_t* wifi_error_reconnect_btn_ptr;
static lv_obj_t* error_restart_btn_ptr;
static void errorEventHandler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e); // What happened?
    lv_obj_t * btn = lv_event_get_target(e);     // Which button was it?

    if(code == LV_EVENT_CLICKED) {
        Serial.println("Button Clicked!");
        if (btn == wifi_error_retry_btn_ptr) {
            lv_obj_t * label = lv_obj_get_child(btn, 0);
            lv_label_set_text(label, "Retrying...");
        }
        else if (btn == wifi_error_reconnect_btn_ptr) {
            lv_obj_t * label = lv_obj_get_child(btn, 0);
            lv_label_set_text(label, "Connecting...");
        }
        else if (btn == error_restart_btn_ptr) {
            lv_obj_t * label = lv_obj_get_child(btn, 0);
            lv_label_set_text(label, "Restarting...");
        }
    }
}



// --- Singleton ---
void UIManager::init() {
    initStyles();
}

void UIManager::initStyles() {
    // Transition
    static constexpr lv_style_prop_t props[] = {LV_STYLE_BG_COLOR, LV_STYLE_TRANSFORM_ZOOM, static_cast<lv_style_prop_t>(0)};
    lv_style_transition_dsc_init(&trans_btn, props, lv_anim_path_linear, 100, 0, NULL);

    // Base Button
    lv_style_init(&style_btn_base);
    lv_style_set_width(&style_btn_base, 350);
    lv_style_set_height(&style_btn_base, 70);
    lv_style_set_radius(&style_btn_base, 50);
    lv_style_set_transition(&style_btn_base, &trans_btn);

    // Shadow
    lv_style_set_shadow_color(&style_btn_base, lv_color_hex(0x000000)); // Color
    lv_style_set_shadow_opa(&style_btn_base, LV_OPA_50); // Opacity
    lv_style_set_shadow_width(&style_btn_base, 20); // Width
    lv_style_set_shadow_ofs_x(&style_btn_base, 0); // Offset X
    lv_style_set_shadow_ofs_y(&style_btn_base, 13); // Offset Y
    lv_style_set_shadow_spread(&style_btn_base, 2); // Spread

    // Spotify Green
    lv_style_init(&style_btn_green);
    lv_style_set_bg_color(&style_btn_green, SPOTIFY_GREEN);
    lv_style_set_bg_opa(&style_btn_green, LV_OPA_COVER);
    lv_style_set_text_color(&style_btn_green, SPOTIFY_WHITE);

    // Spotify Outline
    lv_style_init(&style_btn_outline);
    lv_style_set_bg_opa(&style_btn_outline, 0);
    lv_style_set_border_width(&style_btn_outline, 3);
    lv_style_set_border_color(&style_btn_outline, SPOTIFY_GREEN);
    lv_style_set_text_color(&style_btn_outline, SPOTIFY_WHITE);
}


// Helper
void UIManager::clearScreen() {
    lv_obj_t* old_scr = lv_scr_act();
    current_screen = lv_obj_create(NULL);

    if(current_screen) {
        lv_scr_load(current_screen);
        // Async deletion is the secret! It waits for the click to finish
        // before destroying the memory of the previous screen.
        if(old_scr) lv_obj_del_async(old_scr);
    }
}

// Button Helper
lv_obj_t* UIManager::createSpotifyBtn(lv_obj_t* parent, const lv_event_cb_t cb, const char* text,
    const lv_align_t align, const int x, const int y, const bool is_green)
{
    lv_obj_t* btn = lv_btn_create(parent);

    // Styles
    lv_obj_add_style(btn, &style_btn_base, 0);

    if (is_green) {
        lv_obj_add_style(btn, &style_btn_green, 0);
        lv_obj_set_style_bg_color(btn, SPOTIFY_GREEN, LV_STATE_PRESSED);
    } else {
        lv_obj_add_style(btn, &style_btn_outline, 0);
        lv_obj_set_style_bg_color(btn, SPOTIFY_GREEN_DARKER, LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(btn, LV_OPA_30, LV_STATE_PRESSED);
    }

    // Label
    lv_obj_align(btn, align, x, y);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &font_gotham_medium_40, 0);
    lv_obj_center(label);

    lv_obj_add_event_cb(btn, cb, LV_EVENT_ALL, NULL);

    return btn;
}

lv_obj_t *UIManager::createLogo(lv_obj_t *parent, const lv_font_t *font, const lv_align_t align, const int x, const int y) {

    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    // Flex Container
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(cont, 0, 0); // This makes words touch
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_bg_opa(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);

    lv_obj_set_style_flex_cross_place(cont, LV_FLEX_ALIGN_CENTER, 0);

    // Spotify
    lv_obj_t* l1 = lv_label_create(cont);
    lv_label_set_text(l1, "Spotify");
    lv_obj_set_style_text_color(l1, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(l1, font, 0);

    // Mate
    lv_obj_t* l2 = lv_label_create(cont);
    lv_label_set_text(l2, "Mate");
    lv_obj_set_style_text_color(l2, SPOTIFY_GREEN, 0);
    lv_obj_set_style_text_font(l2, font, 0);

    lv_obj_align(cont, align, x, y);

    return cont;
}


// --- Screens ---
void UIManager::showFailure() {
    clearScreen();

    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    createLogo(current_screen, &font_gotham_medium_80, LV_ALIGN_TOP_MID, 0, 35);

    lv_obj_t* label = lv_label_create(current_screen);
    lv_label_set_text(label, "Something went wrong!");
    lv_obj_set_style_text_color(label, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(label, &font_gotham_medium_60, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    error_restart_btn_ptr = createSpotifyBtn(
        current_screen, errorEventHandler, "Restart", LV_ALIGN_BOTTOM_MID, 0, -31, true);

}


void UIManager::showSplashScreen() {
    clearScreen();

    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    createLogo(current_screen, &font_gotham_medium_80, LV_ALIGN_CENTER, 0, 0);
}



void UIManager::showWifiConnectionError() {
    clearScreen();

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
    wifi_error_retry_btn_ptr = createSpotifyBtn(
        current_screen, errorEventHandler, "Try Again", LV_ALIGN_BOTTOM_LEFT, 38, -19, true);

    // Reconnect Button
    wifi_error_reconnect_btn_ptr = createSpotifyBtn(
        current_screen, errorEventHandler, "Reconnect", LV_ALIGN_BOTTOM_RIGHT, -38, -19, false);

}