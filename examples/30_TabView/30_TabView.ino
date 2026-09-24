/*
  30_TabView — several pages behind a row of tabs.

  YOU WILL LEARN
    - lv_tabview: add tabs, each tab is a page you fill like a screen
    - tab bar at the bottom, where a thumb reaches it
    - swiping between tabs, or tapping the tab bar
    - which tab is showing: LV_EVENT_VALUE_CHANGED + lv_tabview_get_tab_active

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *reading;
static lv_obj_t *chart;
static lv_chart_series_t *series;

static void onTab(lv_event_t *e) {
  lv_obj_t *tv = (lv_obj_t *)lv_event_get_target(e);
  Serial.printf("tab %lu\n", (unsigned long)lv_tabview_get_tab_active(tv));
}

static void sample(lv_timer_t *) {
  static float t = 0;
  t += 0.3f;
  const int32_t v = 22 + (int32_t)(4 * sinf(t));
  lv_label_set_text_fmt(reading, "%ld", (long)v);
  lv_chart_set_next_value(chart, series, v);
}

static void buildHome(lv_obj_t *tab) {
  lv_obj_t *cap = lv_label_create(tab);
  lv_obj_add_style(cap, &LB_Style::label, 0);
  lv_label_set_text(cap, "LIVING ROOM");
  lv_obj_align(cap, LV_ALIGN_TOP_MID, 0, 0);
  reading = lv_label_create(tab);
  lv_obj_add_style(reading, &LB_Style::value, 0);
  lv_obj_set_style_text_font(reading, &lv_font_montserrat_48, 0);
  lv_obj_center(reading);
  lv_obj_t *unit = lv_label_create(tab);
  lv_obj_add_style(unit, &LB_Style::unit, 0);
  lv_label_set_text(unit, "degrees C");
  lv_obj_align(unit, LV_ALIGN_CENTER, 0, 40);
}

static void buildStats(lv_obj_t *tab) {
  chart = lv_chart_create(tab);
  lv_obj_set_size(chart, lv_pct(100), lv_pct(100));
  lv_obj_add_style(chart, &LB_Style::card, 0);
  lv_chart_set_point_count(chart, 30);
  lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
  lv_chart_set_axis_range(chart, LV_CHART_AXIS_PRIMARY_Y, 15, 30);
  lv_obj_set_style_size(chart, 0, 0, LV_PART_INDICATOR);
  series = lv_chart_add_series(chart, LB_Style::accent, LV_CHART_AXIS_PRIMARY_Y);
}

static void buildSettings(lv_obj_t *tab) {
  lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(tab, 12, 0);
  static const char *names[] = {"Night mode", "Sound", "Auto update"};
  for (int i = 0; i < 3; i++) {
    lv_obj_t *row = lv_obj_create(tab);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_label_set_text(lv_label_create(row), names[i]);
    lv_obj_t *sw = lv_switch_create(row);
    if (i) lv_obj_add_state(sw, LV_STATE_CHECKED);
  }
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  lv_obj_t *tv = lv_tabview_create(lv_screen_active());
  lv_tabview_set_tab_bar_position(tv, LV_DIR_BOTTOM);
  lv_tabview_set_tab_bar_size(tv, 48);
  lv_obj_set_style_bg_color(tv, LB_Style::bg, 0);

  // Tab names can carry an icon; LV_SYMBOL_... is just text.
  lv_obj_t *home = lv_tabview_add_tab(tv, LV_SYMBOL_HOME " Home");
  lv_obj_t *stats = lv_tabview_add_tab(tv, LV_SYMBOL_LIST " Stats");
  lv_obj_t *settings = lv_tabview_add_tab(tv, LV_SYMBOL_SETTINGS);
  buildHome(home);
  buildStats(stats);
  buildSettings(settings);

  // The tab bar is an ordinary object: style it like one.
  lv_obj_t *bar = lv_tabview_get_tab_bar(tv);
  lv_obj_set_style_bg_color(bar, LB_Style::surface, 0);
  lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);

  lv_obj_add_event_cb(tv, onTab, LV_EVENT_VALUE_CHANGED, nullptr);

  for (int i = 0; i < 30; i++) sample(nullptr);
  lv_timer_create(sample, 1000, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
