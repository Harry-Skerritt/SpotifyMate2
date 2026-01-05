#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*--- Disable massive extras ---*/
#define LV_USE_FREETYPE 0
#define LV_USE_FFMPEG  0
#define LV_USE_GIF     0
#define LV_USE_PNG     0
#define LV_USE_SJPG    0
#define LV_USE_RLOTTIE 0

/*--- Disable SDL & desktop backends ---*/
#define LV_USE_GPU_SDL 0

/*--- Disable built-in fonts (you use TinyTTF) ---*/
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 0
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/* UI Settings */
#define LV_COLOR_DEPTH 16

#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (512 * 1024)

#define LV_TICK_CUSTOM 1  /* Simplifies timing for Arduino */
#define LV_COLOR_16_SWAP 1

/* Widgets for SpotifyMate */
#define LV_USE_LABEL 1
#define LV_USE_BTN 1
#define LV_USE_IMG 1
#define LV_USE_SLIDER 1
#define LV_USE_ARC 1

#define LV_USE_TINY_TTF 1
#define LV_TINY_TTF_FILE_SUPPORT 0

#endif