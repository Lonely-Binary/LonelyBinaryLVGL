/*
  21_Switch_Checkbox — on/off choices.

  YOU WILL LEARN
    - lv_switch for a setting that takes effect at once
    - lv_checkbox for picking any number of things
    - radio buttons: checkboxes where picking one un-picks the others
    - reading state with lv_obj_has_state(obj, LV_STATE_CHECKED)
    - disabling a control (LV_STATE_DISABLED) when it does not apply

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *wifi, *autoMode;
static lv_obj_t *days[7];
static lv_obj_t *speeds[3];
static lv_obj_t *summary;

static void refresh() {
  static const char *dayNames[7] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
  static const char *speedNames[3] = {"Low", "Mid", "High"};
  char picked[32] = "";
  for (int i = 0; i < 7; i++)
    if (lv_obj_has_state(days[i], LV_STATE_CHECKED)) { strcat(picked, dayNames[i]); strcat(picked, " "); }
  int speed = 0;
  for (int i = 0; i < 3; i++)
    if (lv_obj_has_state(speeds[i], LV_STATE_CHECKED)) speed = i;
  lv_label_set_text_fmt(summary, "Wi-Fi %s\nRuns on: %s\nFan: %s",
                        lv_obj_has_state(wifi, LV_STATE_CHECKED) ? "on" : "off",
                        picked[0] ? picked : "never", speedNames[speed]);
}

static void onChange(lv_event_t *) {
  // Auto mode picks the days itself, so the day boxes do not apply.
  const bool autoOn = lv_obj_has_state(autoMode, LV_STATE_CHECKED);
  for (auto d : days) lv_obj_set_state(d, LV_STATE_DISABLED, autoOn);
  refresh();
}

// Radio behaviour: whichever was clicked becomes the only one checked.
static void onSpeed(lv_event_t *e) {
  lv_obj_t *hit = (lv_obj_t *)lv_event_get_target(e);
  for (auto s : speeds) lv_obj_set_state(s, LV_STATE_CHECKED, s == hit);
  refresh();
}

static lv_obj_t *settingRow(lv_obj_t *parent, const char *name) {
  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_t *l = lv_label_create(row);
  lv_label_set_text(l, name);
  lv_obj_t *sw = lv_switch_create(row);
  lv_obj_add_event_cb(sw, onChange, LV_EVENT_VALUE_CHANGED, nullptr);
  return sw;
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
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(scr, 12, 0);
  lv_obj_set_style_pad_row(scr, 8, 0);

  // Switches: a setting that applies immediately.
  wifi = settingRow(scr, LV_SYMBOL_WIFI "  Wi-Fi");
  lv_obj_add_state(wifi, LV_STATE_CHECKED);
  autoMode = settingRow(scr, LV_SYMBOL_REFRESH "  Auto schedule");

  // Checkboxes: any number of days.
  caption(scr, "RUN ON");
  lv_obj_t *dayRow = lv_obj_create(scr);
  lv_obj_remove_style_all(dayRow);
  lv_obj_set_size(dayRow, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(dayRow, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_pad_gap(dayRow, 8, 0);
  static const char *d[7] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
  for (int i = 0; i < 7; i++) {
    days[i] = lv_checkbox_create(dayRow);
    lv_checkbox_set_text(days[i], d[i]);
    if (i < 5) lv_obj_add_state(days[i], LV_STATE_CHECKED);
    lv_obj_add_event_cb(days[i], onChange, LV_EVENT_VALUE_CHANGED, nullptr);
  }

  // Radio buttons: checkboxes, made round, and only one may be checked.
  caption(scr, "FAN SPEED");
  lv_obj_t *speedRow = lv_obj_create(scr);
  lv_obj_remove_style_all(speedRow);
  lv_obj_set_size(speedRow, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(speedRow, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(speedRow, 14, 0);
  static const char *s[3] = {"Low", "Mid", "High"};
  for (int i = 0; i < 3; i++) {
    speeds[i] = lv_checkbox_create(speedRow);
    lv_checkbox_set_text(speeds[i], s[i]);
    lv_obj_set_style_radius(speeds[i], LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_add_event_cb(speeds[i], onSpeed, LV_EVENT_CLICKED, nullptr);
  }
  lv_obj_add_state(speeds[1], LV_STATE_CHECKED);

  summary = lv_label_create(scr);
  lv_obj_set_style_text_color(summary, LB_Style::muted, 0);
  refresh();
}

void loop() {
  LB_LVGL.loop();
}
