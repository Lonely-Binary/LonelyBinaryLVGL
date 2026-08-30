/*
  LonelyBinaryLVGL — LVGL on a Lonely Binary display, with nothing to configure.

      #include <LonelyBinaryDisplay.h>
      #include <LonelyBinaryLVGL.h>

      LB_Display display(LB_TFT_24);

      void setup() {
        display.begin(true);        // true = PSRAM framebuffer (recommended)
        LB_LVGL.begin(display);

        lv_obj_t *label = lv_label_create(lv_screen_active());
        lv_obj_add_style(label, &LB_Style::value, 0);
        lv_label_set_text(label, "24.6");
        lv_obj_center(label);
      }

      void loop() {
        LB_LVGL.loop();             // must be called often
      }

  WHAT THIS SAVES YOU:
    • lv_conf.h. LVGL normally makes you copy lv_conf_template.h into your
      libraries folder and rename it. A known-good one ships in this library
      and LVGL finds it on the include path. Nothing to copy, nothing to edit.
    • The flush callback, the draw buffers, the tick source and the refresh
      loop — all wired to the panel you already chose with LB_Display.
    • A flat, high-contrast style sheet (LB_Style) instead of LVGL's stock
      gradients and shadows.

  LVGL is vendored under src/lvgl (version in src/lvgl/VENDORED_VERSION),
  MIT-licensed, unmodified apart from removing platform back-ends and image
  decoders that cannot apply here. See tools/vendor_lvgl.sh.

  MIT License · Lonely Binary
*/
#ifndef LONELY_BINARY_LVGL_H
#define LONELY_BINARY_LVGL_H

#include <Arduino.h>
#include <LonelyBinaryDisplay.h>

#include "lvgl/lvgl.h"
#include "LB_Styles.h"

class LB_LVGL_Class {
 public:
  // Bring up LVGL against an already-begun LB_Display.
  //
  // Call display.begin(true) first if you can: with a canvas, LVGL renders
  // straight into the panel's framebuffer and the flush callback copies
  // nothing at all. Without one, LVGL falls back to a partial buffer of
  // `partialLines` rows, which still works but pushes every band over SPI.
  bool begin(LB_Display &display, uint16_t partialLines = 40);

  // Run LVGL's timers and redraw. Call this every time round loop(); it
  // returns quickly when there is nothing to do.
  void loop();

  lv_display_t *display() const { return _disp; }
  bool usingFramebuffer() const { return _direct; }

 private:
  lv_display_t *_disp = nullptr;
  void         *_buf  = nullptr;
  bool          _direct = false;
};

extern LB_LVGL_Class LB_LVGL;

#endif  // LONELY_BINARY_LVGL_H
