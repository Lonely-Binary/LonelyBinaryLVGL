/*
  22_Slider — drag to set a value.

  YOU WILL LEARN
    - lv_slider: range, value, and VALUE_CHANGED while dragging
    - driving real hardware from it: this one sets the backlight
    - range mode: two knobs for a low and a high limit
    - a vertical slider (taller than wide)
    - RELEASED: act once when the finger lifts, not on every pixel

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *brightText, *bandText, *volText;

static void onBrightness(lv_event_t *e) {
  lv_obj_t *s = (lv_obj_t *)lv_event_get_target(e);
  const int32_t v = lv_slider_get_value(s);
  lv_label_set_text_fmt(brightText, LV_SYMBOL_IMAGE "  Brightness  %ld%%", (long)v);
  // Real hardware: the panel's backlight follows the finger.
  display.backlight(v * 255 / 100);
}

static void onBand(lv_event_t *e) {
  lv_obj_t *s = (lv_obj_t *)lv_event_get_target(e);
  lv_label_set_text_fmt(bandText, "Keep between %ld and %ld C", (long)lv_slider_get_left_value(s),
                        (long)lv_slider_get_value(s));
}

static void onVolume(lv_event_t *e) {
  lv_obj_t *s = (lv_obj_t *)lv_event_get_target(e);
  lv_label_set_text_fmt(volText, "%ld", (long)lv_slider_get_value(s));
}

// RELEASED fires once when the finger lifts - the place to save a setting,
// rather than writing flash on every step of the drag.
static void onBandDone(lv_event_t *e) {
  lv_obj_t *s = (lv_obj_t *)lv_event_get_target(e);
  Serial.printf("saved: %ld..%ld\n", (long)lv_slider_get_left_value(s), (long)lv_slider_get_value(s));
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();

  // Left: two horizontal sliders. Right: a vertical one.
  lv_obj_t *left = lv_obj_create(scr);
  lv_obj_remove_style_all(left);
  lv_obj_set_size(left, w - 80, h - 20);
  lv_obj_align(left, LV_ALIGN_LEFT_MID, 10, 0);
  lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(left, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_hor(left, 10, 0);   // room for the knobs at the ends

  brightText = lv_label_create(left);
  lv_obj_set_width(brightText, lv_pct(100));
  lv_obj_t *bright = lv_slider_create(left);
  lv_obj_set_width(bright, lv_pct(100));
  lv_slider_set_range(bright, 5, 100);     // never fully dark: you could not find it
  lv_slider_set_value(bright, 100, LV_ANIM_OFF);
  lv_obj_add_event_cb(bright, onBrightness, LV_EVENT_VALUE_CHANGED, nullptr);
  lv_obj_send_event(bright, LV_EVENT_VALUE_CHANGED, nullptr);

  bandText = lv_label_create(left);
  lv_obj_set_width(bandText, lv_pct(100));   // wraps on a narrow screen
  lv_obj_t *band = lv_slider_create(left);
  lv_obj_set_width(band, lv_pct(100));
  lv_slider_set_mode(band, LV_SLIDER_MODE_RANGE);
  lv_slider_set_range(band, 10, 35);
  // End value first: the start is clamped to it, so setting 18 while the end
  // is still at its default would quietly give you 10.
  lv_slider_set_value(band, 24, LV_ANIM_OFF);
  lv_slider_set_start_value(band, 18, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(band, LB_Style::warn, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(band, LB_Style::warn, LV_PART_KNOB);
  lv_obj_add_event_cb(band, onBand, LV_EVENT_VALUE_CHANGED, nullptr);
  lv_obj_add_event_cb(band, onBandDone, LV_EVENT_RELEASED, nullptr);
  lv_obj_send_event(band, LV_EVENT_VALUE_CHANGED, nullptr);

  // Vertical: make it taller than wide and it slides up and down.
  lv_obj_t *vol = lv_slider_create(scr);
  lv_obj_set_size(vol, 14, h - 110);
  lv_obj_align(vol, LV_ALIGN_TOP_RIGHT, -28, 40);
  lv_slider_set_value(vol, 60, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(vol, LB_Style::ok, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(vol, LB_Style::ok, LV_PART_KNOB);
  volText = lv_label_create(scr);
  lv_obj_align_to(volText, vol, LV_ALIGN_OUT_BOTTOM_MID, 0, 14);
  lv_obj_t *icon = lv_label_create(scr);
  lv_label_set_text(icon, LV_SYMBOL_VOLUME_MAX);
  lv_obj_align_to(icon, vol, LV_ALIGN_OUT_TOP_MID, 0, -8);
  lv_obj_add_event_cb(vol, onVolume, LV_EVENT_VALUE_CHANGED, nullptr);
  lv_obj_send_event(vol, LV_EVENT_VALUE_CHANGED, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
