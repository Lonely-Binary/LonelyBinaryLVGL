/*
  13_Chart — plot values over time.

  YOU WILL LEARN
    - lv_chart: a line chart with two series that scrolls as data arrives
    - point count, axis range, grid lines
    - a bar chart from an array you already have
    - adding numbers beside a chart with lv_scale (charts have no axes)

  TOUCH: not needed.

  TO USE A DIFFERENT SCREEN, change the LB_* constant below.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_TFT_24);

static lv_obj_t *live;
static lv_chart_series_t *temp, *hum;

static void sample(lv_timer_t *) {
  // Stand-ins for two sensors. Replace with your readings.
  static float t = 0;
  t += 0.25f;
  lv_chart_set_next_value(live, temp, 22 + (int32_t)(6 * sinf(t)) + (int32_t)random(-1, 2));
  lv_chart_set_next_value(live, hum, 55 + (int32_t)(15 * sinf(t * 0.4f + 1)));
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

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();
  const bool small = w < 200 || h < 200;
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(scr, small ? 4 : 8, 0);
  lv_obj_set_style_pad_row(scr, 4, 0);

  // ── Live line chart ────────────────────────────────────────────────────
  // A legend: one caption per series, in the series' colour.
  lv_obj_t *legend = lv_obj_create(scr);
  lv_obj_remove_style_all(legend);
  lv_obj_set_size(legend, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(legend, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(legend, 10, 0);
  caption(legend, "LIVE");
  lv_obj_set_style_text_color(caption(legend, LV_SYMBOL_MINUS " TEMP"), LB_Style::warn, 0);
  lv_obj_set_style_text_color(caption(legend, LV_SYMBOL_MINUS " HUMIDITY"), LB_Style::accent, 0);

  // A row: numbers on the left (a vertical lv_scale), the chart on the right.
  lv_obj_t *row = lv_obj_create(scr);
  lv_obj_remove_style_all(row);
  lv_obj_set_width(row, lv_pct(100));
  lv_obj_set_flex_grow(row, 3);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(row, 4, 0);

  lv_obj_t *axis = lv_scale_create(row);
  lv_obj_set_size(axis, w < 200 ? 20 : 28, lv_pct(100));
  lv_scale_set_mode(axis, LV_SCALE_MODE_VERTICAL_LEFT);
  lv_scale_set_range(axis, 0, 100);
  lv_scale_set_total_tick_count(axis, 5);
  lv_scale_set_major_tick_every(axis, 1);
  lv_obj_set_style_text_font(axis, &lv_font_montserrat_10, 0);
  lv_obj_set_style_text_color(axis, LB_Style::muted, 0);
  lv_obj_set_style_line_opa(axis, LV_OPA_TRANSP, LV_PART_MAIN);   // numbers only
  lv_obj_set_style_line_color(axis, LB_Style::muted, LV_PART_INDICATOR);
  lv_obj_set_style_length(axis, 3, LV_PART_INDICATOR);
  // The scale's first and last labels sit on its ends; the chart's plot area
  // sits inside its padding. Pad the scale the same so they line up.
  lv_obj_set_style_pad_ver(axis, 6, 0);

  live = lv_chart_create(row);
  lv_obj_set_height(live, lv_pct(100));
  lv_obj_set_flex_grow(live, 1);
  lv_obj_add_style(live, &LB_Style::card, 0);
  lv_chart_set_type(live, LV_CHART_TYPE_LINE);
  lv_chart_set_point_count(live, 40);                       // 40 points on screen
  lv_chart_set_update_mode(live, LV_CHART_UPDATE_MODE_SHIFT); // new ones push old ones left
  lv_chart_set_axis_range(live, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
  lv_chart_set_div_line_count(live, 5, 8);
  lv_obj_set_style_pad_ver(live, 6, 0);
  lv_obj_set_style_size(live, 0, 0, LV_PART_INDICATOR);      // no dots, just lines
  lv_obj_set_style_line_width(live, 2, LV_PART_ITEMS);
  temp = lv_chart_add_series(live, LB_Style::warn, LV_CHART_AXIS_PRIMARY_Y);
  hum = lv_chart_add_series(live, LB_Style::accent, LV_CHART_AXIS_PRIMARY_Y);

  // ── Bar chart ──────────────────────────────────────────────────────────
  caption(scr, "RAIN THIS WEEK  (MM)");
  lv_obj_t *bars = lv_chart_create(scr);
  lv_obj_set_width(bars, lv_pct(100));
  lv_obj_set_flex_grow(bars, 2);
  lv_obj_add_style(bars, &LB_Style::card, 0);
  lv_chart_set_type(bars, LV_CHART_TYPE_BAR);
  lv_chart_set_point_count(bars, 7);
  lv_chart_set_axis_range(bars, LV_CHART_AXIS_PRIMARY_Y, 0, 30);
  lv_chart_set_div_line_count(bars, 0, 0);
  lv_obj_set_style_pad_column(bars, small ? 2 : 6, 0);       // gap between bars
  lv_chart_series_t *rain = lv_chart_add_series(bars, LB_Style::accent, LV_CHART_AXIS_PRIMARY_Y);
  static const int32_t mm[7] = {4, 12, 0, 22, 17, 3, 9};
  for (int i = 0; i < 7; i++) lv_chart_set_next_value(bars, rain, mm[i]);

  // Pre-fill the live chart so it does not start empty.
  for (int i = 0; i < 40; i++) sample(nullptr);
  lv_timer_create(sample, 300, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
