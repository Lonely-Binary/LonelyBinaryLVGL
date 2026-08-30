/**
 * lv_conf.h — LVGL configuration for Lonely Binary displays.
 *
 * YOU DO NOT NEED TO COPY OR EDIT THIS FILE. That is the whole point of this
 * library: LVGL normally makes you copy lv_conf_template.h into your libraries
 * folder and rename it, which is the same trap as TFT_eSPI's User_Setup.h.
 * A known-good config ships here instead, and LVGL finds it because Arduino
 * puts this src/ folder on the include path (see the __has_include("lv_conf.h")
 * branch in lvgl/src/lv_conf_internal.h).
 *
 * This file only OVERRIDES what we care about. Everything else falls through
 * to LVGL's own defaults in lv_conf_internal.h, so a new LVGL release does not
 * leave a stale 1500-line copy behind — re-run tools/vendor_lvgl.sh and this
 * file usually needs no change at all.
 *
 * The vendored LVGL version is recorded in src/lvgl/VENDORED_VERSION.
 */
#ifndef LV_CONF_H
#define LV_CONF_H

/*====================
   COLOR
 *====================*/

/* All Lonely Binary panels are RGB565. Monochrome e-paper will use I1 through
 * a second display object rather than by changing this. */
#define LV_COLOR_DEPTH 16

/*====================
   MEMORY
 *====================*/

/* Use the C library allocator rather than LVGL's fixed pool. On ESP32 that
 * means allocations are served by the normal heap, which spills into PSRAM on
 * boards that have it — so an N16R8 gets room to breathe without this file
 * having to know whether PSRAM is fitted. */
#define LV_USE_STDLIB_MALLOC  LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING  LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB

/*====================
   HAL
 *====================*/

#define LV_DEF_REFR_PERIOD 16   /* ~60 fps ceiling; the panel is the real limit */
#define LV_DPI_DEF 130          /* typical for a 2-3" SPI panel */
#define LV_USE_OS LV_OS_NONE    /* driven from loop(), not a FreeRTOS task */

/*====================
   RENDERING
 *====================*/

/* Anti-aliased arcs, rounded corners and shadows. Required for lv_arc to look
 * like anything other than a staircase, which matters because the gauge is the
 * most-photographed widget on a round panel. */
#define LV_DRAW_SW_COMPLEX 1
#define LV_DRAW_SW_SUPPORT_RGB565 1
#define LV_DRAW_SW_SUPPORT_RGB565_SWAPPED 1
#define LV_DRAW_SW_SUPPORT_L8 1
#define LV_DRAW_SW_SUPPORT_I1 1   /* keep: monochrome e-paper needs it */

/*====================
   DEBUG
 *====================*/

/* Off by default — a beginner's Serial output should be theirs, not LVGL's.
 * Set LV_USE_LOG to 1 and LV_LOG_LEVEL to LV_LOG_LEVEL_WARN while debugging. */
#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_OBJ 0

/*====================
   FONTS
 *====================*/

/* Every built-in Montserrat size the vendored tree still carries. A disabled
 * font compiles to nothing, so this list costs flash only for the ones you
 * actually reference. */
#define LV_FONT_MONTSERRAT_8  1
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_30 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_48 1

#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Chinese is available but off — enabling it costs about 1 MB of flash, so it
 * has to be a deliberate choice rather than a default.
 *   #define LV_FONT_SIMSUN_16_CJK 1   (lv_font_source_han_sans_sc_16_cjk) */

/*====================
   WIDGETS AND EXTRAS
 *====================*/

/* Widgets are all on: they are small, and turning one off only shows up as a
 * link error in somebody else's example six months later. */

#define LV_USE_QRCODE 1   /* device provisioning screens; ~3 KB */

/* Deleted from the vendored tree by tools/vendor_lvgl.sh — declared 0 here so
 * a stray enable produces a clear "not vendored" failure rather than a
 * confusing missing-symbol link error. */
#define LV_USE_THORVG_INTERNAL 0
#define LV_USE_LODEPNG 0
#define LV_USE_LIBPNG 0
#define LV_USE_BMP 0
#define LV_USE_TJPGD 0
#define LV_USE_LIBJPEG_TURBO 0
#define LV_USE_GIF 0
#define LV_USE_RLOTTIE 0
#define LV_USE_FFMPEG 0
#define LV_USE_SVG 0
#define LV_USE_BARCODE 0
#define LV_USE_TINY_TTF 0
#define LV_USE_FREETYPE 0

/*====================
   THEME
 *====================*/

/* LVGL's default theme is all gradients, shadows and rounded corners. The
 * Lonely Binary look is the opposite — flat colour, large numerals, high
 * contrast — and LB_Styles.h applies it in one place. The stock theme stays
 * compiled in so a customer who wants the standard LVGL look can have it. */
#define LV_USE_THEME_DEFAULT 1
#define LV_USE_THEME_SIMPLE 1

#endif /* LV_CONF_H */
