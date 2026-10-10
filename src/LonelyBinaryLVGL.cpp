#include "LonelyBinaryLVGL.h"

LB_LVGL_Class LB_LVGL;

// The flush callback is a plain C function, so the display it should draw to
// is kept here. One display per sketch is the only case this library supports.
static LB_Display *s_display = nullptr;

static uint32_t lb_tick_cb() { return millis(); }

// Rotation (docs: LB_LVGL.begin(display, lines, 90)). LVGL draws into `s_rot`, a portrait frame; the finished frame is
// turned into whichever panel buffer is not on show and handed to present().
static uint16_t  s_rotation = 0;
static uint16_t *s_rot = nullptr;
static uint16_t *s_front = nullptr;
static int32_t   s_lw = 0, s_lh = 0;     // LVGL's (logical) size
static int32_t   s_pw = 0, s_ph = 0;     // the panel's

// 90: logical (x, y) lands at panel (ph-1-y ... ) - written as the inverse, so every panel pixel is written once, in order.
static void lb_rotate(const uint16_t *src, uint16_t *dst) {
  const int T = 16;   // tiles keep the strided reads inside a few cache lines
  for (int32_t y0 = 0; y0 < s_ph; y0 += T)
    for (int32_t x0 = 0; x0 < s_pw; x0 += T) {
      const int32_t y1 = y0 + T < s_ph ? y0 + T : s_ph, x1 = x0 + T < s_pw ? x0 + T : s_pw;
      for (int32_t y = y0; y < y1; y++) {
        uint16_t *d = dst + (size_t)y * s_pw;
        if (s_rotation == 90) {
          for (int32_t x = x0; x < x1; x++) d[x] = src[(size_t)(s_pw - 1 - x) * s_lw + y];
        } else {
          for (int32_t x = x0; x < x1; x++) d[x] = src[(size_t)x * s_lw + (s_lw - 1 - y)];
        }
      }
    }
}

static void lb_flush_cb(lv_display_t *disp, const lv_area_t *area,
                        uint8_t *px_map) {
  if (s_display && s_rotation) {
    if (lv_display_flush_is_last(disp)) {
      uint16_t *back = s_front == s_display->framebuffer() ? s_display->framebuffer2() : s_display->framebuffer();
      const uint32_t t0 = micros();
      lb_rotate((const uint16_t *)px_map, back);
      const uint32_t t1 = micros();
      s_display->present(back);
      s_front = back;
#ifdef LB_LVGL_ROTATE_LOG
      static uint32_t n = 0;
      if (n++ < 5 || n % 30 == 0) Serial.printf("[LB_LVGL] rotate %lu us, present %lu us\n", (unsigned long)(t1 - t0), (unsigned long)(micros() - t1));
#endif
    }
  } else if (s_display) {
    if (s_display->framebuffer2()) {
      // Double buffered (RGB boards): px_map is the buffer LVGL just finished.
      // Show it at the next vertical blank; LVGL then draws into the other one
      // and copies the changed areas across itself.
      if (lv_display_flush_is_last(disp)) s_display->present((const uint16_t *)px_map);
    } else if (s_display->hasCanvas()) {
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
    // The panel's point, turned into LVGL's: 90 shows the frame turned clockwise, 270 anticlockwise.
    if (s_rotation == 90) { const int16_t px = x; x = y; y = (int16_t)(s_pw - 1 - px); }
    else if (s_rotation == 270) { const int16_t px = x; x = (int16_t)(s_ph - 1 - y); y = px; }
    data->point.x = x;
    data->point.y = y;
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    // LVGL keeps the last point on release, which is what it wants: the
    // click lands where the finger left.
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

bool LB_LVGL_Class::begin(LB_Display &display, uint16_t partialLines) { return begin(display, partialLines, 0); }
uint16_t LB_LVGL_Class::rotation() const { return s_rotation; }
const uint16_t *LB_LVGL_Class::shown() const { return s_rotation ? s_front : nullptr; }

bool LB_LVGL_Class::begin(LB_Display &display, uint16_t partialLines, uint16_t rotation) {
  if (_disp) return true;
  if (!display.begun()) {
    Serial.println(F("[LB_LVGL] call display.begin() before LB_LVGL.begin()."));
    return false;
  }
  s_display = &display;

  lv_init();
  lv_tick_set_cb(lb_tick_cb);

  int32_t w = display.width();
  int32_t h = display.height();
  s_pw = w; s_ph = h;
  s_rotation = 0;
  uint16_t *fb = display.framebuffer();
  uint16_t *fb2 = display.framebuffer2();
  if ((rotation == 90 || rotation == 270) && fb && fb2) {
    // A portrait frame for LVGL, in PSRAM; the panel's two buffers stay as they are.
    s_rot = (uint16_t *)heap_caps_malloc((size_t)w * h * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (s_rot) { memset(s_rot, 0, (size_t)w * h * 2); s_rotation = rotation; s_front = fb; const int32_t t = w; w = h; h = t; }
    else Serial.println(F("[LB_LVGL] no room for a rotated frame: showing the screen upright."));
  } else if (rotation) {
    Serial.println(F("[LB_LVGL] rotation needs a double-buffered RGB panel: showing the screen upright."));
  }
  s_lw = w; s_lh = h;

  _disp = lv_display_create(w, h);
  if (!_disp) return false;
  lv_display_set_flush_cb(_disp, lb_flush_cb);

  if (s_rotation) {
    _direct = true;
    lv_display_set_buffers(_disp, s_rot, nullptr, (uint32_t)w * h * 2, LV_DISPLAY_RENDER_MODE_DIRECT);
  } else if (fb && fb2) {
    // Two buffers, swapped at the vertical blank: no half-drawn frames.
    _direct = true;
    lv_display_set_buffers(_disp, fb, fb2, (uint32_t)w * h * 2, LV_DISPLAY_RENDER_MODE_DIRECT);
  } else if (fb) {
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
