/*
  26_ButtonMatrix — a whole keypad as one object.

  YOU WILL LEARN
    - lv_buttonmatrix: many buttons from one text map, "\n" for a new row
    - one VALUE_CHANGED event for every key, and which key it was
    - widening a key (LV_BUTTONMATRIX_CTRL_WIDTH_2), styling keys by part
    - a door-lock PIN pad: mask the digits, check the code

  A button matrix costs far less RAM than the same number of lv_buttons -
  the right choice for keypads, number pads and toolbars.

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static const char *SECRET = "1234";   // try it

static const char *keys[] = {
  "1", "2", "3", "\n",
  "4", "5", "6", "\n",
  "7", "8", "9", "\n",
  LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_OK, ""   // "" ends the map
};

static lv_obj_t *dots, *status;
static char entered[8];

static void showDots() {
  char d[16] = "";
  for (size_t i = 0; i < strlen(entered); i++) strcat(d, "* ");
  lv_label_set_text(dots, entered[0] ? d : "_ _ _ _");
}

static void onKey(lv_event_t *e) {
  lv_obj_t *pad = (lv_obj_t *)lv_event_get_target(e);
  const char *k = lv_buttonmatrix_get_button_text(pad, lv_buttonmatrix_get_selected_button(pad));
  if (!k) return;
  const size_t n = strlen(entered);
  if (!strcmp(k, LV_SYMBOL_BACKSPACE)) {
    if (n) entered[n - 1] = 0;
  } else if (!strcmp(k, LV_SYMBOL_OK)) {
    const bool ok = !strcmp(entered, SECRET);
    lv_label_set_text(status, ok ? LV_SYMBOL_OK "  Unlocked" : LV_SYMBOL_CLOSE "  Wrong code");
    lv_obj_set_style_text_color(status, ok ? LB_Style::ok : LB_Style::crit, 0);
    entered[0] = 0;
  } else if (n < 4) {
    strcat(entered, k);
  }
  showDots();
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();

  status = lv_label_create(scr);
  lv_label_set_text(status, LV_SYMBOL_EYE_CLOSE "  Enter code");
  lv_obj_align(status, LV_ALIGN_TOP_MID, 0, 8);

  dots = lv_label_create(scr);
  lv_obj_set_style_text_font(dots, &lv_font_montserrat_28, 0);
  lv_obj_align(dots, LV_ALIGN_TOP_MID, 0, 30);
  showDots();

  lv_obj_t *pad = lv_buttonmatrix_create(scr);
  lv_buttonmatrix_set_map(pad, keys);
  lv_obj_set_size(pad, LV_MIN(w, 300), h - 76);
  lv_obj_align(pad, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_text_font(pad, &lv_font_montserrat_24, LV_PART_ITEMS);
  lv_obj_set_style_bg_opa(pad, LV_OPA_TRANSP, 0);         // no box round the keys
  lv_obj_set_style_border_width(pad, 0, 0);
  // Button index counts keys, not rows: 9 is backspace, 11 is OK.
  lv_buttonmatrix_set_button_ctrl(pad, 11, LV_BUTTONMATRIX_CTRL_CHECKED);   // accent colour
  lv_obj_add_event_cb(pad, onKey, LV_EVENT_VALUE_CHANGED, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
