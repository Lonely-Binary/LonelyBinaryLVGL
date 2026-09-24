/*
  25_Spinbox — set a number precisely with + and -.

  YOU WILL LEARN
    - lv_spinbox: digits, a decimal point, range and step
    - buttons that repeat while held (LV_EVENT_LONG_PRESSED_REPEAT)
    - reading the value: an integer, with the decimal point applied by you
    - when a spinbox beats a slider: exact values, like 21.5 C

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *box, *note;

static void onStep(lv_event_t *e) {
  // user_data: +1 for the plus button, -1 for minus. CLICKED covers a tap;
  // LONG_PRESSED_REPEAT keeps stepping while the finger stays down.
  if ((intptr_t)lv_event_get_user_data(e) > 0) lv_spinbox_increment(box);
  else lv_spinbox_decrement(box);
}

static void onValue(lv_event_t *) {
  // The spinbox stores 215 and draws "21.5": the decimal point is display
  // only. Divide by 10 yourself when you use the number.
  const int32_t v = lv_spinbox_get_value(box);
  lv_label_set_text_fmt(note, "Heating starts below %ld.%ld C", (long)(v / 10), (long)(v % 10));
}

static lv_obj_t *stepButton(lv_obj_t *parent, const char *sym, int dir) {
  lv_obj_t *b = lv_button_create(parent);
  lv_obj_set_size(b, 56, 56);
  lv_obj_set_style_radius(b, LV_RADIUS_CIRCLE, 0);
  lv_obj_t *l = lv_label_create(b);
  lv_label_set_text(l, sym);
  lv_obj_center(l);
  lv_obj_add_event_cb(b, onStep, LV_EVENT_CLICKED, (void *)(intptr_t)dir);
  lv_obj_add_event_cb(b, onStep, LV_EVENT_LONG_PRESSED_REPEAT, (void *)(intptr_t)dir);
  return b;
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(scr, 20, 0);

  lv_obj_t *cap = lv_label_create(scr);
  lv_obj_add_style(cap, &LB_Style::label, 0);
  lv_label_set_text(cap, "SET POINT  (C)");

  lv_obj_t *row = lv_obj_create(scr);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(row, 12, 0);

  stepButton(row, LV_SYMBOL_MINUS, -1);
  box = lv_spinbox_create(row);
  lv_spinbox_set_digit_format(box, 3, 2);       // 3 digits, point after 2: "21.5"
  lv_spinbox_set_range(box, 50, 300);           // 5.0 .. 30.0
  lv_spinbox_set_step(box, 5);                  // 0.5 per press
  lv_spinbox_set_value(box, 215);
  lv_obj_set_width(box, 110);
  lv_obj_set_style_text_font(box, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_align(box, LV_TEXT_ALIGN_CENTER, 0);
  // The spinbox shows a cursor on the digit a step changes; with a fixed
  // step of 0.5 it only confuses, so hide it.
  lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, LV_PART_CURSOR);
  lv_obj_add_event_cb(box, onValue, LV_EVENT_VALUE_CHANGED, nullptr);
  stepButton(row, LV_SYMBOL_PLUS, +1);

  note = lv_label_create(scr);
  lv_obj_set_style_text_color(note, LB_Style::muted, 0);
  onValue(nullptr);
}

void loop() {
  LB_LVGL.loop();
}
