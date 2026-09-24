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

static void lb_touch_cb(lv_indev_t *, lv_indev_data_t *data) {
  int16_t x, y;
  LB_Touch *t = s_display ? s_display->touch() : nullptr;
  if (t && t->getTouch(&x, &y)) {
    data->point.x = x;
    data->point.y = y;
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    // LVGL keeps the last point on release, which is what it wants: the
    // click lands where the finger left.
    data->state = LV_INDEV_STATE_RELEASED;
  }
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

  if (display.touch()) {
    _indev = lv_indev_create();
    lv_indev_set_type(_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(_indev, lb_touch_cb);
    lv_indev_set_display(_indev, _disp);
  }

  LB_Style::begin();

  // LVGL's default theme is light. On the house dark background every widget
  // the sketch does not style itself - buttons, sliders, switches, lists -
  // came out as light boxes with dark text on a dark page. The same theme in
  // dark mode, with the house accent as its primary colour, makes every stock
  // widget match LB_Style without any per-widget styling.
  lv_theme_t *th = lv_theme_default_init(_disp, LB_Style::accent, LB_Style::warn,
                                         true, LV_FONT_DEFAULT);
  lv_display_set_theme(_disp, th);
  lv_obj_set_style_bg_color(lv_screen_active(), LB_Style::bg, 0);
  // The top layer (status bars, toasts, anything above every screen) is not
  // themed - applying the theme would make it opaque and hide the screen - so
  // it has no text colour of its own and labels on it came out black.
  lv_obj_set_style_text_color(lv_layer_top(), LB_Style::text, 0);
  return true;
}

void LB_LVGL_Class::loop() {
  lv_timer_handler();
}
