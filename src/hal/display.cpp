//
// Created by Harry Skerritt on 05/01/2026.
//

#include "display.h"



// --- Hardware Objects ---
static Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    5, 3, 46, 7, 14, 38, 18, 17, 10, 39, 0, 45, 48, 47, 21, 1, 2, 42, 41, 40,
    1,
    40,
    48,
    40,
    1,
    13,
    1,
    31,
    1,
    14000000L);

static Arduino_GFX *gfx = new Arduino_RGB_Display(SCREEN_WIDTH, SCREEN_HEIGHT, bus, 0, true);
static TAMC_GT911 ts = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, 43, -1, SCREEN_WIDTH, SCREEN_HEIGHT);

#define DISP_BUF_SIZE 80

// --- Callbacks ---
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    // Only read every 20ms
    static uint32_t last_read = 0;
    if(millis() - last_read < 20) return;
    last_read = millis();

    ts.read();
    if (ts.isTouched && ts.touches > 0) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = ts.points[0].x;
        data->point.y = ts.points[0].y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

#define TOUCH_GT911_ADDRESS 0x5D

// --- Setup ---
void halSetup() {
    Serial.println("Initializing GFX + Touch...");

    pinMode(43, INPUT_PULLUP);

    // CH422G I/O Expander Initialization
    Wire.begin(8, 9, 100000);
    Wire.beginTransmission(0x24); Wire.write(0x01); Wire.endTransmission();
    Wire.beginTransmission(0x38); Wire.write(0x0F); Wire.endTransmission();
    delay(200);

    // GFX
    gfx->begin();

    Serial.println("Starting touch");
    ts.begin();
    ts.setRotation(ROTATION_INVERTED);

    // LVGL Memory
    lv_init();
    // Allocate 40 lines of screen height in PSRAM
    static lv_color_t *disp_draw_buf = (lv_color_t *)ps_malloc(SCREEN_WIDTH * DISP_BUF_SIZE * sizeof(lv_color_t));
    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, SCREEN_WIDTH * DISP_BUF_SIZE);

    // Register Display Driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Register Touch Driver
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

}
