#include "LonelyBinaryLVGL.h"

LB_LVGL_Class LB_LVGL;

// The flush callback is a plain C function, so the display it should draw to
// is kept here. One display per sketch is the only case this library supports.
static LB_Display *s_display = nullptr;

static uint32_t lb_tick_cb() { return millis(); }

static void lb_flush_cb(lv_display_t *disp, const lv_area_t *area,
                        uint8_t *px_map) {
  if (s_display) {
    if (s_display->hasCanvas()) {
      // Direct mode: LVGL has already written into the panel's own
      // framebuffer, so there is nothing to copy — we only have to push the
      // finished frame. Doing that on every sub-area would send the whole
      // screen several times per refresh, hence the is-last check.
      if (lv_display_flush_is_last(disp)) s_display->flush();
    } else {
      s_display->pushImage(area->x1, area->y1, lv_area_get_width(area),
                           lv_area_get_height(area), (const uint16_t *)px_map);
    }
  }
  lv_display_flush_ready(disp);
}

bool LB_LVGL_Class::begin(LB_Display &display, uint16_t partialLines) {
  if (_disp) return true;
  if (!display.begun()) {
    Serial.println(F("[LB_LVGL] call display.begin() before LB_LVGL.begin()."));
    return false;
  }
  s_display = &display;

  lv_init();
  lv_tick_set_cb(lb_tick_cb);

  const int32_t w = display.width();
  const int32_t h = display.height();

  _disp = lv_display_create(w, h);
  if (!_disp) return false;
  lv_display_set_flush_cb(_disp, lb_flush_cb);

  uint16_t *fb = display.framebuffer();
  if (fb) {
    // Zero-copy: LVGL's draw buffer IS the panel's framebuffer.
    _direct = true;
    lv_display_set_buffers(_disp, fb, nullptr, (uint32_t)w * h * 2,
                           LV_DISPLAY_RENDER_MODE_DIRECT);
  } else {
    _direct = false;
    const size_t bytes = (size_t)w * partialLines * 2;
    _buf = heap_caps_malloc(bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!_buf) _buf = heap_caps_malloc(bytes, MALLOC_CAP_8BIT);
    if (!_buf) {
      Serial.println(F("[LB_LVGL] draw buffer allocation failed — lower "
                       "partialLines, or use display.begin(true) with PSRAM."));
      return false;
    }
    lv_display_set_buffers(_disp, _buf, nullptr, bytes,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
  }

  LB_Style::begin();
  return true;
}

void LB_LVGL_Class::loop() {
  lv_timer_handler();
}
