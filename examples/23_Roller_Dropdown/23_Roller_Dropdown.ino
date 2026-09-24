/*
  23_Roller_Dropdown — pick one from a list.

  YOU WILL LEARN
    - lv_roller: a spinning wheel; here two of them make a time picker
    - options as one string, one per line ("00\n01\n02...")
    - LV_ROLLER_MODE_INFINITE: 59 wraps round to 00
    - lv_dropdown: a list that opens when tapped and closes when chosen
    - reading the choice: index (get_selected) or text (get_selected_str)

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *hours, *minutes, *mode, *result;

static void onChange(lv_event_t *) {
  char h[4], m[4], md[16];
  lv_roller_get_selected_str(hours, h, sizeof h);
  lv_roller_get_selected_str(minutes, m, sizeof m);
  lv_dropdown_get_selected_str(mode, md, sizeof md);
  lv_label_set_text_fmt(result, LV_SYMBOL_BELL "  %s at %s:%s", md, h, m);
}

static lv_obj_t *roller(lv_obj_t *parent, const char *options, int visible) {
  lv_obj_t *r = lv_roller_create(parent);
  lv_roller_set_options(r, options, LV_ROLLER_MODE_INFINITE);
  lv_roller_set_visible_row_count(r, visible);
  lv_obj_add_event_cb(r, onChange, LV_EVENT_VALUE_CHANGED, nullptr);
  return r;
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  lv_obj_t *scr = lv_screen_active();
  const int32_t h = display.height();
  const int visible = h >= 280 ? 5 : 3;

  // Build "00\n01\n...\n23" and "00\n...\n59" once.
  static char hourOpts[24 * 3], minOpts[60 * 3];
  char *p = hourOpts;
  for (int i = 0; i < 24; i++) p += sprintf(p, i ? "\n%02d" : "%02d", i);
  p = minOpts;
  for (int i = 0; i < 60; i++) p += sprintf(p, i ? "\n%02d" : "%02d", i);

  // The dropdown sits at the top; its list opens downwards over the rest.
  mode = lv_dropdown_create(scr);
  lv_dropdown_set_options(mode, "Wake up\nMedicine\nWater plants\nFeed the cat");
  lv_obj_set_width(mode, lv_pct(80));
  lv_obj_align(mode, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_add_event_cb(mode, onChange, LV_EVENT_VALUE_CHANGED, nullptr);

  // Two rollers side by side with a colon between: HH : MM.
  lv_obj_t *row = lv_obj_create(scr);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(row, 8, 0);
  lv_obj_align(row, LV_ALIGN_CENTER, 0, 10);
  hours = roller(row, hourOpts, visible);
  lv_obj_t *colon = lv_label_create(row);
  lv_label_set_text(colon, ":");
  lv_obj_set_style_text_font(colon, &lv_font_montserrat_28, 0);
  minutes = roller(row, minOpts, visible);
  lv_roller_set_selected(hours, 7, LV_ANIM_OFF);
  lv_roller_set_selected(minutes, 30, LV_ANIM_OFF);

  result = lv_label_create(scr);
  lv_obj_set_style_text_color(result, LB_Style::accent, 0);
  lv_obj_align(result, LV_ALIGN_BOTTOM_MID, 0, -12);
  onChange(nullptr);
}

void loop() {
  LB_LVGL.loop();
}
