/*
  12_Scale — a real instrument dial with ticks, numbers and a needle.

  YOU WILL LEARN
    - lv_scale: ticks, major ticks every N, labels, the angle it spans
    - sections: colour part of the scale (a red zone)
    - a line needle that points at a value
    - the same widget as a straight ruler (horizontal mode)

  TOUCH: not needed.

  TO USE A DIFFERENT SCREEN, change the LB_* constant below.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *gauge, *needle, *readout, *ruler, *marker;
static lv_style_t redTicks, redArc;
static int32_t needleLen;

static void setRpm(int32_t rpm) {
  lv_scale_set_line_needle_value(gauge, needle, needleLen, rpm);
  lv_label_set_text_fmt(readout, "%ld00\nRPM", (long)rpm);
}

static void tick(lv_timer_t *) {
  // An engine revving up and settling back, forever.
  static float t = 0;
  t += 0.08f;
  const int32_t rpm = 8 + (int32_t)(52 * (0.5f - 0.5f * cosf(t)));
  setRpm(rpm);
  // The ruler's marker follows the same value along the bottom.
  lv_obj_set_x(marker, lv_obj_get_x(ruler) + lv_obj_get_width(ruler) * rpm / 70 - 4);
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();
  const int32_t d = LV_MIN(w, h - 40) - 16;       // leave room for the ruler

  // ── The round dial ─────────────────────────────────────────────────────
  gauge = lv_scale_create(scr);
  lv_obj_set_size(gauge, d, d);
  lv_obj_align(gauge, LV_ALIGN_TOP_MID, 0, 6);
  lv_scale_set_mode(gauge, LV_SCALE_MODE_ROUND_INNER);   // numbers inside the ring
  lv_scale_set_range(gauge, 0, 70);                      // x100 RPM
  lv_scale_set_total_tick_count(gauge, 36);              // 2 per step of 10... ish
  lv_scale_set_major_tick_every(gauge, 5);               // a numbered tick every 10
  lv_scale_set_label_show(gauge, true);
  lv_scale_set_angle_range(gauge, 240);
  lv_scale_set_rotation(gauge, 150);                     // gap at the bottom

  // Tick lengths and colours: minor ticks are ITEMS, major ones INDICATOR.
  lv_obj_set_style_length(gauge, d / 30, LV_PART_ITEMS);
  lv_obj_set_style_length(gauge, d / 16, LV_PART_INDICATOR);
  lv_obj_set_style_line_color(gauge, LB_Style::muted, LV_PART_ITEMS);
  lv_obj_set_style_line_color(gauge, LB_Style::text, LV_PART_INDICATOR);
  lv_obj_set_style_text_color(gauge, LB_Style::muted, LV_PART_INDICATOR);
  lv_obj_set_style_text_font(gauge, d > 200 ? &lv_font_montserrat_14 : &lv_font_montserrat_10, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(gauge, LB_Style::muted, LV_PART_MAIN);
  lv_obj_set_style_arc_width(gauge, 2, LV_PART_MAIN);

  // The red zone: 60..70. A section carries its own styles for the ticks
  // and the arc that fall inside it.
  lv_style_init(&redTicks);
  lv_style_set_line_color(&redTicks, LB_Style::crit);
  lv_style_set_text_color(&redTicks, LB_Style::crit);
  lv_style_init(&redArc);
  lv_style_set_arc_color(&redArc, LB_Style::crit);
  lv_style_set_arc_width(&redArc, 4);
  lv_scale_section_t *zone = lv_scale_add_section(gauge);
  lv_scale_set_section_range(gauge, zone, 60, 70);
  lv_scale_set_section_style_indicator(gauge, zone, &redTicks);
  lv_scale_set_section_style_items(gauge, zone, &redTicks);
  lv_scale_set_section_style_main(gauge, zone, &redArc);

  // The needle is an lv_line inside the scale; the scale points it.
  needle = lv_line_create(gauge);
  lv_obj_set_style_line_width(needle, LV_MAX(d / 60, 2), 0);
  lv_obj_set_style_line_color(needle, LB_Style::accent, 0);
  lv_obj_set_style_line_rounded(needle, true, 0);
  needleLen = d / 2 - d / 8;

  readout = lv_label_create(gauge);
  lv_obj_set_style_text_align(readout, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_font(readout, d > 200 ? &lv_font_montserrat_20 : &lv_font_montserrat_12, 0);
  lv_obj_align(readout, LV_ALIGN_CENTER, 0, d / 4);

  // ── A straight ruler along the bottom ──────────────────────────────────
  ruler = lv_scale_create(scr);
  lv_obj_set_size(ruler, w * 8 / 10, 24);
  lv_obj_align(ruler, LV_ALIGN_BOTTOM_MID, 0, -4);
  lv_scale_set_mode(ruler, LV_SCALE_MODE_HORIZONTAL_BOTTOM);
  lv_scale_set_range(ruler, 0, 70);
  lv_scale_set_total_tick_count(ruler, 15);
  lv_scale_set_major_tick_every(ruler, 2);
  lv_scale_set_label_show(ruler, false);
  lv_obj_set_style_line_color(ruler, LB_Style::muted, LV_PART_ITEMS);
  lv_obj_set_style_line_color(ruler, LB_Style::text, LV_PART_INDICATOR);
  lv_obj_set_style_length(ruler, 5, LV_PART_ITEMS);
  lv_obj_set_style_length(ruler, 10, LV_PART_INDICATOR);

  marker = lv_obj_create(scr);
  lv_obj_remove_style_all(marker);
  lv_obj_set_size(marker, 8, 8);
  lv_obj_set_style_radius(marker, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(marker, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(marker, LB_Style::accent, 0);
  lv_obj_align_to(marker, ruler, LV_ALIGN_OUT_TOP_LEFT, 0, 0);
  lv_obj_update_layout(scr);                             // positions are final now

  setRpm(8);
  lv_timer_create(tick, 50, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
