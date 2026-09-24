/*
  20_Button — make something happen when you tap.

  YOU WILL LEARN
    - lv_button, and putting a label on it
    - events: lv_obj_add_event_cb(obj, function, WHICH_EVENT, user_data)
    - CLICKED (tap), LONG_PRESSED (hold), VALUE_CHANGED (a toggle flipped)
    - a toggle button: LV_OBJ_FLAG_CHECKABLE and LV_STATE_CHECKED
    - user_data: one callback serving several buttons

  This is the most important example in the series: every touch widget after
  this one talks to your code the same way, through events.

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *countLabel, *lampLabel;
static int count = 0;

static lv_obj_t *button(lv_obj_t *parent, const char *text) {
  lv_obj_t *b = lv_button_create(parent);
  lv_obj_set_width(b, lv_pct(100));
  lv_obj_t *l = lv_label_create(b);
  lv_label_set_text(l, text);
  lv_obj_center(l);
  return b;
}

// ── Event callbacks ─────────────────────────────────────────────────────────
// LVGL calls these; they run inside LB_LVGL.loop(). Keep them short.

static void onTap(lv_event_t *e) {
  // user_data is the number we passed to lv_obj_add_event_cb: +1 or -1.
  count += (int)(intptr_t)lv_event_get_user_data(e);
  lv_label_set_text_fmt(countLabel, "%d", count);
}

static void onHold(lv_event_t *) {
  count = 0;
  lv_label_set_text(countLabel, "0");
}

static void onToggle(lv_event_t *e) {
  lv_obj_t *b = (lv_obj_t *)lv_event_get_target(e);
  const bool on = lv_obj_has_state(b, LV_STATE_CHECKED);
  lv_label_set_text(lampLabel, on ? LV_SYMBOL_POWER "  Lamp ON" : LV_SYMBOL_POWER "  Lamp OFF");
  // This is where you would switch a relay: digitalWrite(RELAY_PIN, on);
  Serial.printf("lamp %s\n", on ? "on" : "off");
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  lv_obj_t *scr = lv_screen_active();
  const bool small = display.width() < 200 || display.height() < 200;
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(scr, small ? 6 : 16, 0);

  countLabel = lv_label_create(scr);
  lv_obj_set_style_text_font(countLabel, small ? &lv_font_montserrat_24 : &lv_font_montserrat_48, 0);
  lv_label_set_text(countLabel, "0");

  // Two buttons in a row, one callback: user_data tells them apart.
  lv_obj_t *row = lv_obj_create(scr);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(row, 10, 0);
  lv_obj_t *minus = button(row, LV_SYMBOL_MINUS);
  lv_obj_t *plus = button(row, LV_SYMBOL_PLUS);
  lv_obj_set_flex_grow(minus, 1);
  lv_obj_set_flex_grow(plus, 1);
  lv_obj_add_event_cb(minus, onTap, LV_EVENT_CLICKED, (void *)(intptr_t)-1);
  lv_obj_add_event_cb(plus, onTap, LV_EVENT_CLICKED, (void *)(intptr_t)+1);

  // Hold to reset: LONG_PRESSED fires after about 400 ms of holding.
  lv_obj_t *reset = button(scr, "Hold to reset");
  lv_obj_set_style_bg_color(reset, LB_Style::surface, 0);
  lv_obj_add_event_cb(reset, onHold, LV_EVENT_LONG_PRESSED, nullptr);

  // A toggle: CHECKABLE makes each tap flip LV_STATE_CHECKED, and the theme
  // draws the checked state in the accent colour. VALUE_CHANGED fires on flip.
  lv_obj_t *lamp = button(scr, "");
  lampLabel = lv_obj_get_child(lamp, 0);
  lv_obj_add_flag(lamp, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_style_bg_color(lamp, LB_Style::surface, 0);
  lv_obj_add_event_cb(lamp, onToggle, LV_EVENT_VALUE_CHANGED, nullptr);
  lv_obj_send_event(lamp, LV_EVENT_VALUE_CHANGED, nullptr);   // set the label
}

void loop() {
  LB_LVGL.loop();
}
