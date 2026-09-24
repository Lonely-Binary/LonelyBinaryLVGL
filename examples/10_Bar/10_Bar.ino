/*
  10_Bar — show a level: progress, a tank, a range.

  YOU WILL LEARN
    - lv_bar: range, value, and animating to a new value
    - horizontal or vertical is simply which way the bar is longer
    - LV_BAR_MODE_RANGE: a bar from a start value to an end value
    - colouring the filled part (LV_PART_INDICATOR) by level

  TOUCH: not needed.

  TO USE A DIFFERENT SCREEN, change the LB_* constant below.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_TFT_24);

static lv_obj_t *progress, *progressText, *tank, *tankText, *band;

static void step(lv_timer_t *) {
  // Progress: 0 -> 100, then start again.
  static int p = 0;
  p = p >= 100 ? 0 : p + 5;
  lv_bar_set_value(progress, p, LV_ANIM_ON);   // slides rather than jumps
  lv_label_set_text_fmt(progressText, "Downloading  %d%%", p);

  // Tank: a slow random walk; colour follows the level.
  static int level = 60;
  level = LV_CLAMP(5, level + (int)random(-9, 10), 100);
  lv_bar_set_value(tank, level, LV_ANIM_ON);
  lv_label_set_text_fmt(tankText, "%d%%", level);
  lv_obj_set_style_bg_color(tank, level < 20 ? LB_Style::crit : level < 40 ? LB_Style::warn : LB_Style::ok,
                            LV_PART_INDICATOR);

  // Range: today's low and high drift a little.
  static int lo = 14, hi = 27;
  lo = LV_CLAMP(5, lo + (int)random(-1, 2), 20);
  hi = LV_CLAMP(22, hi + (int)random(-1, 2), 38);
  lv_bar_set_start_value(band, lo, LV_ANIM_ON);
  lv_bar_set_value(band, hi, LV_ANIM_ON);
}

static lv_obj_t *caption(lv_obj_t *parent, const char *t) {
  lv_obj_t *l = lv_label_create(parent);
  lv_obj_add_style(l, &LB_Style::label, 0);
  lv_label_set_text(l, t);
  return l;
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  const int32_t w = display.width(), h = display.height();
  const int32_t pad = (w < 200 || h < 200) ? 6 : 12;
  lv_obj_t *scr = lv_screen_active();

  // Left: a column of horizontal bars. Right: one tall vertical bar.
  lv_obj_t *left = lv_obj_create(scr);
  lv_obj_remove_style_all(left);
  lv_obj_set_size(left, w * 7 / 10 - pad, h - 2 * pad);
  lv_obj_align(left, LV_ALIGN_LEFT_MID, pad, 0);
  lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(left, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

  // 1. Progress. Wider than tall, so it fills left to right.
  progressText = caption(left, "");
  progress = lv_bar_create(left);
  lv_obj_set_size(progress, lv_pct(100), 12);
  lv_bar_set_range(progress, 0, 100);
  // How long LV_ANIM_ON takes to slide to a new value.
  lv_obj_set_style_anim_duration(progress, 400, 0);

  // 2. A plain level, set once. No animation, no timer.
  caption(left, "SIGNAL");
  lv_obj_t *signal = lv_bar_create(left);
  lv_obj_set_size(signal, lv_pct(100), 12);
  lv_bar_set_value(signal, 72, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(signal, LB_Style::ok, LV_PART_INDICATOR);

  // 3. Range mode: the filled part runs from a start value to the value.
  //    Here, today's temperature band on a -10..40 C scale.
  caption(left, "TODAY  LOW - HIGH");
  band = lv_bar_create(left);
  lv_obj_set_size(band, lv_pct(100), 12);
  lv_bar_set_mode(band, LV_BAR_MODE_RANGE);
  lv_bar_set_range(band, -10, 40);
  lv_obj_set_style_bg_color(band, LB_Style::warn, LV_PART_INDICATOR);
  lv_obj_set_style_anim_duration(band, 400, 0);

  // 4. Vertical: taller than wide, so it fills bottom to top. A water tank.
  tank = lv_bar_create(scr);
  lv_obj_set_size(tank, LV_MAX(w * 3 / 10 - 3 * pad, 16), h - 2 * pad - 20);
  lv_obj_align(tank, LV_ALIGN_TOP_RIGHT, -pad, pad);
  lv_obj_set_style_radius(tank, 4, 0);
  lv_obj_set_style_radius(tank, 4, LV_PART_INDICATOR);
  lv_obj_set_style_anim_duration(tank, 600, 0);
  tankText = lv_label_create(scr);
  lv_obj_align_to(tankText, tank, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

  step(nullptr);
  lv_timer_create(step, 700, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
