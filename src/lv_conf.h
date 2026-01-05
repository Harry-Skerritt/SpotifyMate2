#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>


/* UI Settings */
#define LV_COLOR_DEPTH 16
#define LV_MEM_CUSTOM 1   /* Use malloc() which uses our 8MB PSRAM */
#define LV_TICK_CUSTOM 1  /* Simplifies timing for Arduino */
#define LV_COLOR_16_SWAP 1

/* Widgets for SpotifyMate */
#define LV_USE_LABEL 1
#define LV_USE_BTN 1
#define LV_USE_IMG 1
#define LV_USE_SLIDER 1
#define LV_USE_ARC 1

#endif