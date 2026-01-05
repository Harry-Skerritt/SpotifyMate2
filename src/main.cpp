#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include <Wire.h>
#include <TAMC_GT911.h>
#include <WiFi.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

// --- Touch Pins ---
#define TOUCH_SDA  8
#define TOUCH_SCL  9
#define TOUCH_INT  -1
#define TOUCH_RST  -1

// --- Global LVGL Objects ---
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;

// --- Hardware Objects ---
TAMC_GT911 ts = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, SCREEN_WIDTH, SCREEN_HEIGHT);

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    5, 3, 46, 7, 14, 38, 18, 17, 10, 39, 0, 45, 48, 47, 21, 1, 2, 42, 41, 40,
    1, 8, 4, 8, 1, 8, 4, 8, 0, 12000000L);

Arduino_GFX *gfx = new Arduino_RGB_Display(SCREEN_WIDTH, SCREEN_HEIGHT, bus, 0, true);

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

const char* ssid = "Mustang";
const char* password = "";

void setup_wifi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    setup_wifi();

    // 1. Initialize PSRAM FIRST (Before anything else)
    if (!psramInit()) {
        Serial.println("PSRAM Fail! Check your platformio.ini settings.");
    } else {
        Serial.printf("PSRAM Total: %d bytes\n", ESP.getPsramSize());
        Serial.printf("PSRAM Free: %d bytes\n", ESP.getFreePsram());
    }

    // 2. Initialize I2C with a short timeout
    Serial.println("Starting I2C...");
    Wire.begin(TOUCH_SDA, TOUCH_SCL, 100000); // Use 100kHz for higher stability
    Wire.setTimeOut(100); // 100ms timeout prevents the Watchdog from biting on hangs

    // 3. Wake up hardware via CH422G
    Serial.println("Configuring CH422G...");
    Wire.beginTransmission(0x24);
    Wire.write(0x01);
    if (Wire.endTransmission() != 0) Serial.println("Expander 0x24 NACK");

    Wire.beginTransmission(0x38);
    Wire.write(0x0E);
    if (Wire.endTransmission() != 0) Serial.println("Expander 0x38 NACK");

    delay(200);

    // 4. Initialize GFX
    Serial.println("Initializing GFX...");
    if (!gfx->begin()) {
        Serial.println("GFX Init Failed!");
    }
    gfx->fillScreen(BLACK);

    // 5. Initialize LVGL
    Serial.println("Starting LVGL...");
    lv_init();

    disp_draw_buf = (lv_color_t *)ps_malloc(SCREEN_WIDTH * 80 * sizeof(lv_color_t));
    if (!disp_draw_buf) {
        Serial.println("CRITICAL: LVGL Buffer Allocation Failed!");
        while(1) delay(1000);
    }

    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, SCREEN_WIDTH * 40);

    // Register Display
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // 6. Touch setup
    Serial.println("Starting Touch...");

    // Manually set INT pin as input before starting
    pinMode(4, INPUT);

    // Pass the I2C address explicitly to ts.begin()
    // Waveshare boards usually use 0x5D
    ts.begin(0x5D);
    ts.setRotation(ROTATION_NORMAL);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    // 7. Simple Test UI
    // 7. Create a robust Touch Test UI
    lv_obj_t *btn = lv_btn_create(lv_scr_act());     // Create button
    lv_obj_set_size(btn, 400, 200);                  // Make it huge (easy to hit)
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);        // Center it
    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_RED), 0);

    lv_obj_t *label = lv_label_create(btn);          // Put label inside button
    lv_label_set_text(label, "Press Me");
    lv_obj_center(label);

    static int click_count = 0;

    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
        // 2. Increment the counter
        click_count++;

        // 3. Get the button and the label
        lv_obj_t *b = lv_event_get_target(e);
        lv_obj_t *l = lv_obj_get_child(b, 0);

        // 4. Use lv_label_set_text_fmt to handle the number
        lv_label_set_text_fmt(l, "Clicks: %d", click_count);

        // Optional: Visual feedback
        lv_obj_set_style_bg_color(b, lv_palette_main(LV_PALETTE_GREEN), 0);
        Serial.printf("Touch Registered. Total Clicks: %d\n", click_count);
    }, LV_EVENT_CLICKED, NULL);

    // 1. Create a second label object on the current screen
    lv_obj_t * label2 = lv_label_create(lv_scr_act());

    // 2. Set the text
    lv_label_set_text_fmt(label2, "Connected!\nIP: %s", WiFi.localIP().toString().c_str());
    // 3. Position it
    // You can use alignments like LV_ALIGN_TOP_MID, LV_ALIGN_BOTTOM_LEFT, etc.
    lv_obj_align(label2, LV_ALIGN_TOP_MID, 0, 20); // Top middle, 20 pixels down

    // 4. Style it (Optional: Make it look different)
    lv_obj_set_style_text_color(label2, lv_palette_main(LV_PALETTE_ORANGE), 0);

    Serial.println("Setup Complete.");
}

void loop() {
    lv_timer_handler();
    delay(5);
}