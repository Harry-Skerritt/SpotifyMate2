#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include <Wire.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

// Exact Timing & Pin mapping from your Waveshare documentation
Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    5  /* DE */, 3  /* VSYNC */, 46 /* HSYNC */, 7 /* PCLK */,
    14 /* R0 */, 38 /* R1 */, 18 /* R2 */, 17 /* R3 */, 10 /* R4 */,
    39 /* G0 */, 0  /* G1 */, 45 /* G2 */, 48 /* G3 */, 47 /* G4 */, 21 /* G5 */,
    1  /* B0 */, 2  /* B1 */, 42 /* B2 */, 41 /* B3 */, 40 /* B4 */,
    1  /* hsync_p */, 8 /* hsync_fp */, 4 /* hsync_pw */, 8 /* hsync_bp */,
    1  /* vsync_p */, 8 /* vsync_fp */, 4 /* vsync_pw */, 8 /* vsync_bp */,
    1  /* pclk_active_neg */, 16000000L /* 16MHz */);

Arduino_GFX *gfx = new Arduino_RGB_Display(
    SCREEN_WIDTH, SCREEN_HEIGHT, bus, 0, true);

// Initialize CH422G Expander to turn on Backlight and release Reset
void setup_waveshare_hw() {
    Wire.begin(8, 9);
    // Command 0x01: Enable Output mode
    Wire.beginTransmission(0x24); Wire.write(0x01); Wire.write(0x01); Wire.endTransmission();
    // Command 0x38: Set EXIO2 (Backlight) and EXIO3 (Reset) HIGH (Binary 1100 = 0x0C)
    Wire.beginTransmission(0x38); Wire.write(0x0C); Wire.endTransmission();
}

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;

// LVGL Display Flush Callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    lv_disp_flush_ready(disp);
}

void setup() {
    Serial.begin(115200);
    setup_waveshare_hw();

    if (!psramInit()) {
        Serial.println("CRITICAL: PSRAM NOT DETECTED!");
    }

    if (!gfx->begin()) {
        Serial.println("GFX Begin Failed!");
    }
    gfx->fillScreen(BLACK);

    // LVGL Initialization
    lv_init();

    // Allocate dual buffer in PSRAM for smooth 800x480 performance
    disp_draw_buf = (lv_color_t *)ps_malloc(SCREEN_WIDTH * 40 * sizeof(lv_color_t));
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, SCREEN_WIDTH * 40);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Test UI: Center Label
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Harry Skerritt");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    Serial.println("Setup Complete.");
}

void loop() {
    lv_timer_handler(); // Update LVGL
    delay(5);
}