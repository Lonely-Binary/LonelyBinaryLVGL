/*
  41_Dashboard — a live, multi-sensor dashboard.

  PUTS TOGETHER
    - a header row with a clock and icons (03_Layout, Flex)
    - tiles on a grid that adapt to any screen shape (03_Layout, Grid)
    - a sparkline per tile (13_Chart), colour by threshold (02_Styles)
    - an alert banner that appears only when something is wrong
      (15_LED_Spinner, LV_OBJ_FLAG_HIDDEN)
    - tap a tile for its min / max / average (28_MsgBox)

  One struct describes each metric - name, unit, thresholds - and the rest is
  built from it. Add a metric by adding one line to METRICS[].

  The readings are simulated. Put your sensor reads in readMetric().

  TOUCH: optional (tap a tile for its statistics). Runs on every panel.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

struct Metric {
  const char *name, *unit, *fmt;
  float warn, crit;          // above warn: amber; above crit: red
  float base, swing;         // for the simulation only
};
static const Metric METRICS[] = {
  {"TEMP", "C", "%.1f", 28, 32, 24, 6},
  {"HUMIDITY", "%", "%.0f", 70, 85, 55, 25},
  {"CO2", "ppm", "%.0f", 1000, 1400, 800, 500},
  {"POWER", "W", "%.0f", 1800, 2400, 900, 1100},
};
static const int N = sizeof METRICS / sizeof METRICS[0];
static const int HISTORY = 30;

struct Tile {
  lv_obj_t *value, *chart;
  lv_chart_series_t *series;
  float history[HISTORY];
  int count;
};
static Tile tiles[N];
static lv_obj_t *clockLabel, *banner;

static float readMetric(int i, float t) {
  const Metric &m = METRICS[i];
  return m.base + m.swing * sinf(t * (0.3f + 0.17f * i) + i) + m.swing * 0.1f * (random(-10, 11) / 10.0f);
}

static lv_color_t levelColor(const Metric &m, float v) {
  return v > m.crit ? LB_Style::crit : v > m.warn ? LB_Style::warn : LB_Style::accent;
}

static void update(lv_timer_t *) {
  static float t = 0;
  t += 0.2f;
  bool alarm = false;
  for (int i = 0; i < N; i++) {
    Tile &tile = tiles[i];
    const Metric &m = METRICS[i];
    const float v = readMetric(i, t);
    memmove(tile.history, tile.history + 1, sizeof(float) * (HISTORY - 1));
    tile.history[HISTORY - 1] = v;
    if (tile.count < HISTORY) tile.count++;

    char buf[16];
    snprintf(buf, sizeof buf, m.fmt, v);
    lv_label_set_text(tile.value, buf);
    LB_Style::setLevel(tile.value, v > m.crit ? LB_Style::LEVEL_CRIT : v > m.warn ? LB_Style::LEVEL_WARN
                                                                                : LB_Style::LEVEL_OK);
    // Sparkline scaled to its own range, so small swings are still visible.
    lv_chart_set_axis_range(tile.chart, LV_CHART_AXIS_PRIMARY_Y, (int32_t)(m.base - m.swing * 1.2f),
                            (int32_t)(m.base + m.swing * 1.2f));
    lv_chart_set_next_value(tile.chart, tile.series, (int32_t)v);
    lv_chart_set_series_color(tile.chart, tile.series, levelColor(m, v));
    alarm |= v > m.crit;
  }
  if (alarm) lv_obj_remove_flag(banner, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(banner, LV_OBJ_FLAG_HIDDEN);

  const uint32_t s = millis() / 1000;
  lv_label_set_text_fmt(clockLabel, "%02lu:%02lu:%02lu", (unsigned long)(s / 3600 % 24),
                        (unsigned long)(s / 60 % 60), (unsigned long)(s % 60));
}

static void onTile(lv_event_t *e) {
  const int i = (int)(intptr_t)lv_event_get_user_data(e);
  const Tile &tile = tiles[i];
  if (!tile.count) return;
  float lo = 1e9f, hi = -1e9f, sum = 0;
  for (int k = HISTORY - tile.count; k < HISTORY; k++) {
    lo = LV_MIN(lo, tile.history[k]);
    hi = LV_MAX(hi, tile.history[k]);
    sum += tile.history[k];
  }
  char a[16], b[16], c[16];
  snprintf(a, sizeof a, METRICS[i].fmt, lo);
  snprintf(b, sizeof b, METRICS[i].fmt, hi);
  snprintf(c, sizeof c, METRICS[i].fmt, sum / tile.count);
  lv_obj_t *box = lv_msgbox_create(nullptr);
  lv_msgbox_add_title(box, METRICS[i].name);
  lv_msgbox_add_text_fmt(box, "Last %d readings (%s)\n\nMin  %s\nMax  %s\nAverage  %s", tile.count,
                         METRICS[i].unit, a, b, c);
  lv_msgbox_add_close_button(box);
}

static lv_obj_t *makeTile(lv_obj_t *parent, int i, bool compact) {
  const Metric &m = METRICS[i];
  Tile &tile = tiles[i];
  lv_obj_t *card = lv_obj_create(parent);
  lv_obj_add_style(card, &LB_Style::card, 0);
  lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(card, 2, 0);
  lv_obj_add_event_cb(card, onTile, LV_EVENT_CLICKED, (void *)(intptr_t)i);

  lv_obj_t *cap = lv_label_create(card);
  lv_obj_add_style(cap, &LB_Style::label, 0);
  lv_label_set_text_fmt(cap, "%s  %s", m.name, m.unit);

  tile.value = lv_label_create(card);
  lv_obj_add_style(tile.value, &LB_Style::value, 0);
  lv_obj_set_style_text_font(tile.value, compact ? &lv_font_montserrat_20 : &lv_font_montserrat_28, 0);

  // The sparkline: no grid, no dots, no border - just the line.
  tile.chart = lv_chart_create(card);
  // remove_style_all FIRST: sizes and flex_grow are style properties too, so
  // setting them before it would be silently wiped (the chart then keeps its
  // default 150+ px height and runs off the bottom of the tile).
  lv_obj_remove_style_all(tile.chart);
  lv_obj_set_width(tile.chart, lv_pct(100));
  lv_obj_set_flex_grow(tile.chart, 1);
  lv_chart_set_div_line_count(tile.chart, 0, 0);
  lv_chart_set_point_count(tile.chart, HISTORY);
  lv_chart_set_update_mode(tile.chart, LV_CHART_UPDATE_MODE_SHIFT);
  lv_obj_set_style_line_width(tile.chart, 2, LV_PART_ITEMS);
  lv_obj_set_style_size(tile.chart, 0, 0, LV_PART_INDICATOR);
  tile.series = lv_chart_add_series(tile.chart, LB_Style::accent, LV_CHART_AXIS_PRIMARY_Y);
  lv_obj_remove_flag(tile.chart, LV_OBJ_FLAG_CLICKABLE);   // taps go to the card
  return card;
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();
  const bool compact = w < 240 || h < 240;
  const int32_t gap = compact ? 4 : 8;
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(scr, gap, 0);
  lv_obj_set_style_pad_row(scr, gap, 0);

  // Header: title, clock pushed to the right.
  lv_obj_t *header = lv_obj_create(scr);
  lv_obj_remove_style_all(header);
  lv_obj_set_size(header, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_t *title = lv_label_create(header);
  lv_label_set_text(title, LV_SYMBOL_HOME "  Workshop");
  clockLabel = lv_label_create(header);
  lv_obj_set_style_text_color(clockLabel, LB_Style::muted, 0);

  // Tiles on a grid: 2 x 2, or 4 x 1 on a strip display.
  lv_obj_t *grid = lv_obj_create(scr);
  lv_obj_remove_style_all(grid);
  lv_obj_set_width(grid, lv_pct(100));
  lv_obj_set_flex_grow(grid, 1);
  lv_obj_set_style_pad_gap(grid, gap, 0);
  const bool strip = w > 2 * h;
  static const int32_t c2[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static const int32_t c4[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static const int32_t r2[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static const int32_t r1[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(grid, strip ? c4 : c2, strip ? r1 : r2);
  for (int i = 0; i < N; i++) {
    lv_obj_t *t = makeTile(grid, i, compact);
    lv_obj_set_grid_cell(t, LV_GRID_ALIGN_STRETCH, strip ? i : i % 2, 1, LV_GRID_ALIGN_STRETCH,
                         strip ? 0 : i / 2, 1);
  }

  // Alert banner: on the top layer so it covers the tiles, hidden until needed.
  banner = lv_label_create(lv_layer_top());
  lv_label_set_text(banner, LV_SYMBOL_WARNING "  Check the red reading");
  lv_obj_set_style_bg_opa(banner, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(banner, LB_Style::crit, 0);
  lv_obj_set_style_text_color(banner, LB_Style::bg, 0);
  lv_obj_set_style_pad_all(banner, 6, 0);
  lv_obj_set_style_radius(banner, 4, 0);
  lv_obj_align(banner, LV_ALIGN_BOTTOM_MID, 0, -gap);
  lv_obj_add_flag(banner, LV_OBJ_FLAG_HIDDEN);

  for (int i = 0; i < HISTORY; i++) update(nullptr);
  lv_timer_create(update, 1000, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
