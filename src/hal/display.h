//
// Created by Harry Skerritt on 05/01/2026.
//

#ifndef DISPLAY_H
#define DISPLAY_H

#pragma once
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include <TAMC_GT911.h>
#include <Wire.h>


void halSetup();
void halSetBrightness(uint8_t brightness);



// --- Screen Size ---
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

// --- Touch Pins ---
#define TOUCH_SDA  8
#define TOUCH_SCL  9
#define TOUCH_INT  -1
#define TOUCH_RST  -1





#endif //DISPLAY_H
