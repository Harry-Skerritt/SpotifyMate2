//
// Created by Harry Skerritt on 06/01/2026.
//

#include "UIManager.h"

#include "global_state.h"
#include "network/WifiManager.h"
#include "spotify/SpotifyManager.h"
#include "system/SystemManager.h"


void UIManager::init() {
    initStyles();
}

void UIManager::update() {
    static bool first_run = true;
    static WifiStatus last_wifi_status = WIFI_IDLE;
    static SpotifyStatus last_spotify_status = SPOTIFY_IDLE;
    static uint32_t connectedStartTime = 0;

    // --- WIFI ----
    if (networkState.status != last_wifi_status || first_run) {
        switch (networkState.status) {
            case WIFI_IDLE:
                showOnboarding();
                break;

            case WIFI_CONNECTING:
                showSpinner("Connecting to " + networkState.selected_ssid);
                break;

            case WIFI_SCANNING:
                showSpinner("Searching for networks...");
                break;

            case WIFI_SCAN_RESULTS:
                showNetworkList(networkState.found_ssids);
                break;

            case WIFI_CONNECTED:
                showContextScreen("WiFi Connected!");
                break;

            case WIFI_ERROR:
                showWifiError();
                break;
        }

        if (networkState.status == WIFI_CONNECTED) {
            connectedStartTime = millis();
        } else {
            connectedStartTime = 0;
        }

        last_wifi_status = networkState.status;
    }

    // --- TRANSITION ---
    bool wifi_ready_for_spotify = false;
    if (networkState.status == WIFI_CONNECTED && connectedStartTime != 0) {
        if (millis() - connectedStartTime > 1500) {
            wifi_ready_for_spotify = true;
            SpotifyManager::getInstance().buildAuthURL();

            if (spotifyState.status == SPOTIFY_IDLE) {
                if (spotifyState.refresh_token.length() > 0) spotifyState.status = SPOTIFY_INITIALIZING;
                else if (systemState.spotify_linked && spotifyState.refresh_token.length() == 0) spotifyState.status = SPOTIFY_LINK_ERROR;
                else spotifyState.status = SPOTIFY_NEED_LINK;
            }
        }
    }

    // --- SPOTIFY ---
    if (wifi_ready_for_spotify && (spotifyState.status != last_spotify_status)) {
        switch (spotifyState.status) {
            case SPOTIFY_NEED_LINK:
                showSpotifyLinking(spotifyState.auth_url.c_str());
                // Needs to start a web server and listen
                break;

            case SPOTIFY_AUTHENTICATING:
                showSpinner("Authenticating with Spotify...");
                break;

            case SPOTIFY_INITIALIZING:
                showSpinner("Resuming Spotify Session...");

            case SPOTIFY_READY:
                showContextScreen("Main Player!");
                //showMainPlayer();
                break;

            case SPOTIFY_LINK_ERROR:
                showSpotifyLinkError();
                break;

            case SPOTIFY_ERROR:
                //showFailure(); // Or a specific Spotify error screen
                showContextScreen("Spotify Error");
                break;

            default: break;
        }
        last_spotify_status = spotifyState.status;
    }

    first_run = false;
}

// --- Callbacks - Errors ----
static lv_obj_t* wifi_error_retry_btn_ptr;
static lv_obj_t* wifi_error_reconnect_btn_ptr;
static lv_obj_t* error_restart_btn_ptr;
static lv_obj_t* spotify_link_error_btn_ptr;

static void errorEventHandler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e); // What happened?
    lv_obj_t * btn = lv_event_get_target(e);     // Which button was it?

    if(code == LV_EVENT_CLICKED) {
        if (btn == wifi_error_retry_btn_ptr) {
            // Retry with longer timeout
            WifiManager::getInstance().requestConnect(30000); // Double timeout

        }
        else if (btn == wifi_error_reconnect_btn_ptr) {
            // Restart WiFi onboarding
            WifiManager::getInstance().requestReset();
        }

        else if (btn == error_restart_btn_ptr) {
            // Restart Device
            ESP.restart();
        }
        else if (btn == spotify_link_error_btn_ptr) {
            // Re link spotify
            SystemManager::getInstance().resetSpotifyTokens(); // Wipe saved tokens
            spotifyState.status = SPOTIFY_NEED_LINK; // Re onboard
            spotifyState.refresh_token = ""; // Clear local token
        }
    }
}

// --- Callbacks - Onboarding ---
static lv_obj_t* get_connected_btn_ptr;
static void onboardingEventHandler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e); // What happened?
    lv_obj_t * btn = lv_event_get_target(e);     // Which button was it?

    if(code == LV_EVENT_CLICKED) {
        Serial.println("Button Clicked!");

        if (btn == get_connected_btn_ptr) {
            lv_obj_t * label = lv_obj_get_child(btn, 0);
            lv_label_set_text(label, "Connecting...");

            // Trigger a scan
            WifiManager::getInstance().requestScan();
        }
    }
}

// --- Callbacks - Wifi Join ---
static void wifiJoinEventHandler(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * card = lv_obj_get_parent(btn);

    // Find the SSID label
    lv_obj_t * label = lv_obj_get_child(card, 0);
    const char * ssid = lv_label_get_text(label);

    Serial.printf("UI: Attempting to join: %s\n", ssid);
    networkState.selected_ssid = String(ssid);

    UIManager::getInstance().showPasswordEntry(ssid);
}

// --- Screen Calls - Core ---
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

void UIManager::showSpinner(const String &msg) {
    clearScreen();
    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    lv_obj_t* spinner = lv_spinner_create(current_screen, 1000, 60);
    lv_obj_set_size(spinner, 80, 80);
    lv_obj_center(spinner);

    lv_obj_set_style_arc_color(spinner, SPOTIFY_GREEN, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(spinner, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 8, LV_PART_INDICATOR);

    lv_obj_t* label = lv_label_create(current_screen);
    lv_label_set_text(label, msg.c_str());
    lv_obj_set_style_text_font(label, &font_gotham_medium_40, 0);
    lv_obj_set_style_text_color(label, SPOTIFY_WHITE, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 80);
}

void UIManager::showContextScreen(const String &msg) {
    clearScreen();
    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    createLogo(current_screen, &font_gotham_medium_80, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* context = lv_label_create(current_screen);
    lv_label_set_text(context, msg.c_str());
    lv_obj_set_style_text_font(context, &font_gotham_medium_20, 0);
    lv_obj_set_style_text_color(context, SPOTIFY_WHITE, 0);
    lv_obj_align(context, LV_ALIGN_BOTTOM_MID, 0, -18);
}



// --- Screen Calls - WiFi ---
void UIManager::showOnboarding() {
    clearScreen();

    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    // Header Title
    lv_obj_t* title = lv_label_create(current_screen);
    lv_label_set_text(title, "Let's get connected!");
    lv_obj_set_style_text_color(title, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(title, &font_gotham_medium_60, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    // Message
    lv_obj_t * body = lv_label_create(current_screen);
    lv_label_set_text(body, "Press the buton below to search for \nlocal networks. \n\nFind yours and connect!");
    lv_obj_set_style_text_color(body, SPOTIFY_GREY, 0);
    lv_obj_set_style_text_font(body, &font_gotham_medium_40, 0);
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 30, 115);

    get_connected_btn_ptr = createSpotifyBtn(current_screen, onboardingEventHandler, "Get Connected!", LV_ALIGN_BOTTOM_MID, 0, -32, true);
}

void UIManager::showNetworkList(const std::vector<String>& networks) {
    clearScreen();
    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    // Header Title
    lv_obj_t* title = lv_label_create(current_screen);

    char buf[32];
    sprintf(buf, "%d Networks found", networks.size());
    lv_label_set_text(title, buf);
    lv_obj_set_style_text_color(title, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(title, &font_gotham_medium_60, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    // Create a scrolling container for the list
    lv_obj_t* list_cont = lv_obj_create(current_screen);
    lv_obj_set_size(list_cont, 800, 350);
    lv_obj_align(list_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(list_cont, LV_FLEX_FLOW_COLUMN); // Stack cards vertically
    lv_obj_set_style_bg_opa(list_cont, 0, 0);
    lv_obj_set_style_border_width(list_cont, 0, 0);
    lv_obj_set_style_pad_gap(list_cont, 10, 0); // Space between cards

    // Network list
    populateWifiList(list_cont, networks);
}

void UIManager::showWifiError() {
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
    lv_obj_set_style_text_color(body, SPOTIFY_GREY, 0);
    lv_obj_set_style_text_font(body, &font_gotham_medium_40, 0);
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 30, 115);

    // Retry Button
    wifi_error_retry_btn_ptr = createSpotifyBtn(
        current_screen, errorEventHandler, "Try Again", LV_ALIGN_BOTTOM_LEFT, 38, -19, true);

    // Reconnect Button
    wifi_error_reconnect_btn_ptr = createSpotifyBtn(
        current_screen, errorEventHandler, "Reconnect", LV_ALIGN_BOTTOM_RIGHT, -38, -19, false);

}

void UIManager::showPasswordEntry(const String &ssid) {
    clearScreen();
    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    // Header
    lv_obj_t* title = lv_label_create(current_screen);
    lv_label_set_text_fmt(title, "Connect to %s", ssid);
    lv_obj_set_style_text_font(title, &font_gotham_medium_40, 0);
    lv_obj_set_style_text_color(title, SPOTIFY_WHITE, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);

    // Text Area
    lv_obj_t* ta = lv_textarea_create(current_screen);
    lv_textarea_set_password_mode(ta, true);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_placeholder_text(ta, "Enter Password");
    lv_obj_set_size(ta, 500, 60);
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 100);
    lv_obj_set_style_text_font(ta, &font_gotham_medium_20, 0);

    // Spotify Style
    lv_obj_set_style_bg_color(ta, lv_color_hex(0x333333), 0);
    lv_obj_set_style_text_color(ta, SPOTIFY_WHITE, 0);
    lv_obj_set_style_border_width(ta, 0, 0);

    // Keyboard
    lv_obj_t* kb = lv_keyboard_create(current_screen);
    lv_obj_set_size(kb, 800, 240);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_keyboard_set_textarea(kb, ta);

    lv_obj_set_style_bg_color(kb, BACKGROUND_GREY, 0);
    lv_obj_set_style_bg_color(kb, SPOTIFY_GREEN, LV_PART_ITEMS | LV_STATE_CHECKED);

    // Keyboard Callback
    lv_obj_add_event_cb(kb, [](lv_event_t* e) {
        lv_event_code_t code = lv_event_get_code(e);
        lv_obj_t* kb = lv_event_get_target(e);
        lv_obj_t* ta = lv_keyboard_get_textarea(kb);

        if(code == LV_EVENT_READY) { // Ready = Checkmark/OK clicked
            const char* pwd = lv_textarea_get_text(ta);

            networkState.selected_pass = pwd;

            networkState.status = WIFI_CONNECTING;
            WifiManager::getInstance().requestConnect();

        } else if(code == LV_EVENT_CANCEL) {

            getInstance().showOnboarding(); // Go back

        }
    }, LV_EVENT_ALL, NULL);
}



// --- Spotify ---
void UIManager::showSpotifyLinkError() {
    clearScreen();

    lv_obj_set_style_bg_color(current_screen, BACKGROUND_GREY, 0);

    // Header Title
    lv_obj_t* title = lv_label_create(current_screen);
    lv_label_set_text(title, "Can't connect to Spotify");
    lv_obj_set_style_text_color(title, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(title, &font_gotham_medium_60, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    // Message
    lv_obj_t * body = lv_label_create(current_screen);
    lv_label_set_text(body, "Something has gone wrong! \nPress the button below and follow the instructions to re-link your Spotify account");
    lv_obj_set_style_text_color(body, SPOTIFY_GREY, 0);
    lv_obj_set_style_text_font(body, &font_gotham_medium_40, 0);
    lv_obj_align(body, LV_ALIGN_TOP_LEFT, 30, 115);

    spotify_link_error_btn_ptr = createSpotifyBtn(current_screen, errorEventHandler, "Re-link", LV_ALIGN_BOTTOM_MID, 0, -32, true);
}


void UIManager::showSpotifyLinking(const char *auth_url) {
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


// --- Main Functionality ---
void UIManager::showMainPlayer() {
    clearScreen();

    lv_obj_set_style_bg_color(current_screen, rgbToBGRHex(0x942219), 0);

    // Song Title
    lv_obj_t* song_title = lv_label_create(current_screen);
    lv_label_set_text(song_title, "Rock Believer");
    lv_obj_set_style_text_color(song_title, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(song_title, &font_gotham_medium_50, 0);
    lv_obj_align(song_title, LV_ALIGN_BOTTOM_MID, 8, -155);


    // Song Artist
    lv_obj_t* song_artist = lv_label_create(current_screen);
    lv_label_set_text(song_artist, "Scorpions");
    lv_obj_set_style_text_color(song_artist, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(song_artist, &font_gotham_medium_30, 0);
    lv_obj_align(song_artist, LV_ALIGN_BOTTOM_MID, 8, -105);

    // Device Icon


    // Device Name
    lv_obj_t* device_name = lv_label_create(current_screen);
    lv_label_set_text(device_name, "Harry's Mac Mini");
    lv_obj_set_style_text_color(device_name, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(device_name, &font_gotham_medium_20, 0);
    lv_obj_align(device_name, LV_ALIGN_BOTTOM_MID, 50, -190);
}




// --- PRIVATE ---
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

    // Network card
    lv_style_init(&style_network_card);
    lv_style_set_bg_color(&style_network_card, lv_color_hex(0x444444));
    lv_style_set_bg_opa(&style_network_card, LV_OPA_COVER);
    lv_style_set_radius(&style_network_card, 12);
    lv_style_set_pad_all(&style_network_card, 15);
    lv_style_set_width(&style_network_card, 750); // Standard width for 800px screen
    lv_style_set_height(&style_network_card, LV_SIZE_CONTENT);
}

void UIManager::clearScreen() {
    lv_obj_t* old_scr = lv_scr_act();
    current_screen = lv_obj_create(NULL);

    if(current_screen) {
        lv_scr_load(current_screen);
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

lv_obj_t *UIManager::createNetworkItem(lv_obj_t *parent, const char *ssid) {
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_add_style(card, &style_network_card, 0);

    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Padding
    lv_obj_set_style_pad_top(card, 10, 0);
    lv_obj_set_style_pad_bottom(card, 10, 0);
    lv_obj_set_style_pad_left(card, 20, 0);
    lv_obj_set_style_pad_right(card, 20, 0);

    // SSID (Left)
    lv_obj_t* label = lv_label_create(card);
    lv_label_set_text(label, ssid);
    lv_obj_set_style_text_color(label, SPOTIFY_WHITE, 0);
    lv_obj_set_style_text_font(label, &font_gotham_medium_40, 0);

    // Truncation
    lv_obj_set_flex_grow(label, 1);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);


    // Connect Button (Right)
    lv_obj_t* btn = lv_btn_create(card);
    lv_obj_set_size(btn, 170, 42);
    lv_obj_add_style(btn, &style_btn_green, 0); // Reuse your green style
    lv_obj_set_style_radius(btn, 20, 0);
    lv_obj_add_event_cb(btn, wifiJoinEventHandler, LV_EVENT_CLICKED, NULL);

    lv_obj_t* btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Connect");
    lv_obj_set_style_text_font(btn_label, &font_gotham_medium_20, 0);
    lv_obj_set_style_text_color(btn_label, SPOTIFY_WHITE, 0);
    lv_obj_center(btn_label);

    return card;
}

lv_obj_t *UIManager::createCustomQRCode(lv_obj_t *parent, const char *url, int size) {
    lv_obj_t* frame = lv_obj_create(parent);
    lv_obj_set_size(frame, size + 10, size + 10);
    lv_obj_set_style_radius(frame, 15, 0);
    lv_obj_set_style_bg_color(frame, lv_color_hex(0x121212), 0);
    lv_obj_set_style_border_color(frame, SPOTIFY_GREEN, 0);
    lv_obj_set_style_border_width(frame, 2, 0);
    lv_obj_set_style_clip_corner(frame, true, 0);
    lv_obj_set_style_pad_all(frame, 5, 0);
    lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);


    lv_obj_t* qr = lv_qrcode_create(frame, size, SPOTIFY_GREEN, lv_color_hex(0x121212));
    lv_qrcode_update(qr, url, strlen(url));

    lv_obj_align(qr, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(qr, LV_OBJ_FLAG_IGNORE_LAYOUT);

    return frame;
}

void UIManager::populateWifiList(lv_obj_t* list_cont, const std::vector<String>& networks) {
    lv_obj_clean(list_cont);

    for (const String& ssid : networks) {
        createNetworkItem(list_cont, ssid.c_str());
    }

    lv_obj_t* spacer = lv_obj_create(list_cont);
    lv_obj_set_size(spacer, LV_PCT(100), 40);
    lv_obj_set_style_bg_opa(spacer, 0, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);

    lv_obj_scroll_to_y(list_cont, 0, LV_ANIM_OFF);
}

lv_color_t UIManager::rgbToBGRHex(uint32_t c) {

    uint32_t r = (c & 0xFF0000) >> 16; // Red
    uint32_t g = (c & 0x00FF00);       // Green
    uint32_t b = (c & 0x0000FF);       // Blue

    uint32_t bgr = (b << 16) | g | r;

    return lv_color_hex(bgr);

}


