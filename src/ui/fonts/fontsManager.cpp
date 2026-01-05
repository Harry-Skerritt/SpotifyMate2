//
// Created by Harry Skerritt on 05/01/2026.
//

#include "fontsManager.h"
#include <Arduino.h>

extern "C" {
    extern const uint8_t gotham_font[];
    extern const int gotham_font_size;
}



lv_font_t * font_20;
lv_font_t * font_35;
lv_font_t * font_40;
lv_font_t * font_50;
lv_font_t * font_60;
lv_font_t * font_80;



void initCustomFonts() {
    delay(200);

    Serial.printf("Font in flash at: %p size=%d\n", gotham_font, gotham_font_size);

    uint8_t* font_psram = (uint8_t*)ps_malloc(gotham_font_size);
    if (!font_psram) {
        Serial.println("FAILED to allocate PSRAM for font");
        return;
    }

    memcpy(font_psram, gotham_font, gotham_font_size);
    Serial.printf("Font copied to PSRAM at: %p\n", font_psram);

    font_20 = lv_tiny_ttf_create_data(font_psram, gotham_font_size, 20);
    font_60 = lv_tiny_ttf_create_data(font_psram, gotham_font_size, 60);
    font_80 = lv_tiny_ttf_create_data(font_psram, gotham_font_size, 80);

    if (!font_20 || !font_60 || !font_80) {
        Serial.println("Font creation failed");
        return;
    }

    Serial.printf("Fonts OK. Free PSRAM: %u\n", ESP.getFreePsram());
}