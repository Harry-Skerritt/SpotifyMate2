//
// Created by Harry Skerritt on 05/01/2026.
//

#include "display.h"



// --- Hardware Objects ---
static Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    5, 3, 46, 7,
    14, 38, 18, 17, 10,
    39, 0, 45, 48, 47, 21,
    1, 2, 42, 41, 40,
    1,
    40,
    48,
    40,
    1,
    13,
    3,
    32,
    0,
    16000000L);

static Arduino_GFX *gfx = new Arduino_RGB_Display(SCREEN_WIDTH, SCREEN_HEIGHT, bus, true);
static TAMC_GT911 ts = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, SCREEN_WIDTH, SCREEN_HEIGHT);

// --- Callbacks ---
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    ts.read();
    if (ts.isTouched && ts.touches > 0) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = ts.points[0].x;
        data->point.y = ts.points[0].y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}


#define BUF_LINES 120

// --- Setup ---
void halSetup() {
    Serial.println("Starting I2C");
    // 1. Start I2C
    Wire.begin(8, 9, 100000); // Specify 100kHz speed for stability
    delay(100);


    // 2. Wake up CH422G IO Expander
    // Address 0x24: System Config. 0x01 = Enable I/O
    Serial.println("Waking up CH422G");
    Wire.beginTransmission(0x24);
    Wire.write(0x01); // SYS_CFG
    Wire.write(0x01);
    Wire.endTransmission();
    delay(5);

    // Address 0x38: Set OC pins.
    // 0x0F usually turns on Backlight and pulls LCD_RST high
    Serial.println("Set OC Pins");
    Wire.beginTransmission(0x23);
    //Wire.write(0x03); // OUTPUT_CFG
    Wire.write(0x0F);
    Wire.endTransmission();
    delay(5);

    Wire.beginTransmission(0x33);
    Wire.write(0xFF);
    Wire.endTransmission();
    delay(5);

    delay(500);


    if (!gfx) {
        Serial.println("CRITICAL: GFX Object creation failed!");
        return;
    }

    Serial.println("Starting GFX");
    if (!gfx->begin()) {
        Serial.println("CRITICAL: GFX begin() failed!");
        return;
    }

    delay(200);


    //Serial.println("Setting screen black");
    //if (gfx) gfx->fillScreen(BLACK); // Force the screen to turn black immediately
    //delay(100);

    //Serial.println("Setting screen blue");
    //if (gfx) gfx->fillScreen(BLUE);

    Serial.println("GFX initialization passed without fillScreen");
    Serial.println("Starting touch");
    //ts.begin();
    //ts.setRotation(ROTATION_NORMAL);

    // LVGL Memory
    Serial.println("Starting LVGL");
    lv_init();


    Serial.println("Allocating PSRAM");
    // Allocate 40 lines of screen height in PSRAM
    lv_color_t *buf1 = (lv_color_t *)ps_malloc(SCREEN_WIDTH * BUF_LINES * sizeof(lv_color_t));
    lv_color_t *buf2 = (lv_color_t *)ps_malloc(SCREEN_WIDTH * BUF_LINES * sizeof(lv_color_t));


    Serial.println("Draw Buffer stuff");
    if (!buf1 || !buf2)
    {
        Serial.println("Failed to allocate display buffer");
        lv_disp_draw_buf_t draw_buf;
        Serial.println("PSRAM Buffer Allocation Failed!");
        // Fallback to internal if PSRAM fails
        buf1 = (lv_color_t *)malloc(SCREEN_WIDTH * 20 * sizeof(lv_color_t));
        buf2 = (lv_color_t *)malloc(SCREEN_WIDTH * 20 * sizeof(lv_color_t));
    }

    lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, SCREEN_WIDTH * BUF_LINES);


    Serial.println("Registering Display Driver");
    // Register Display Driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);


    Serial.println("Registering Touch Driver");
    // Register Touch Driver
   // static lv_indev_drv_t indev_drv;
    //lv_indev_drv_init(&indev_drv);
    //indev_drv.type = LV_INDEV_TYPE_POINTER;
    //indev_drv.read_cb = my_touchpad_read;
   // lv_indev_drv_register(&indev_drv);

    //gfx->fillScreen(RED);

}
