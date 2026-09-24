/*
  24_Keyboard — type text on the screen.

  YOU WILL LEARN
    - lv_textarea: one line, placeholder text, password dots, max length
    - lv_keyboard: an on-screen keyboard that types into a textarea
    - moving the keyboard between fields when a field is tapped (FOCUSED)
    - READY (the tick key) and CANCEL (the keyboard key) events
    - a number-only keyboard for a PIN

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP. The keyboard
  needs width: 240 pixels is a squeeze, 320 is comfortable.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *kb, *name, *pass, *pin, *hello;

// A field was tapped: point the keyboard at it, in the right mode.
static void onFocus(lv_event_t *e) {
  lv_obj_t *ta = (lv_obj_t *)lv_event_get_target(e);
  lv_keyboard_set_textarea(kb, ta);
  lv_keyboard_set_mode(kb, ta == pin ? LV_KEYBOARD_MODE_NUMBER : LV_KEYBOARD_MODE_TEXT_LOWER);
  lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
}

// The tick key (READY) or the hide-keyboard key (CANCEL).
static void onKeyboard(lv_event_t *e) {
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  if (lv_event_get_code(e) == LV_EVENT_READY) {
    const char *n = lv_textarea_get_text(name);
    lv_label_set_text_fmt(hello, "Hello, %s!  (password %d chars, PIN %s)", n[0] ? n : "stranger",
                          (int)strlen(lv_textarea_get_text(pass)), lv_textarea_get_text(pin));
  }
  // Nothing is being typed into now: drop the cursor.
  lv_obj_remove_state(lv_keyboard_get_textarea(kb), LV_STATE_FOCUSED);
}

static lv_obj_t *field(lv_obj_t *parent, const char *placeholder) {
  lv_obj_t *ta = lv_textarea_create(parent);
  lv_textarea_set_one_line(ta, true);
  lv_textarea_set_placeholder_text(ta, placeholder);
  lv_obj_set_width(ta, lv_pct(100));
  lv_obj_add_event_cb(ta, onFocus, LV_EVENT_FOCUSED, nullptr);
  return ta;
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  lv_obj_t *scr = lv_screen_active();
  const int32_t h = display.height();

  // The form takes the top; the keyboard slides over the bottom half.
  lv_obj_t *form = lv_obj_create(scr);
  lv_obj_remove_style_all(form);
  lv_obj_set_size(form, lv_pct(100), h / 2);
  lv_obj_set_flex_flow(form, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(form, 8, 0);
  lv_obj_set_style_pad_row(form, 6, 0);

  name = field(form, "Your name");
  pass = field(form, "Password");
  lv_textarea_set_password_mode(pass, true);    // dots; the last key shows briefly
  pin = field(form, "4-digit PIN");
  lv_textarea_set_max_length(pin, 4);
  lv_textarea_set_accepted_chars(pin, "0123456789");

  hello = lv_label_create(scr);
  lv_obj_set_width(hello, lv_pct(90));
  lv_label_set_long_mode(hello, LV_LABEL_LONG_MODE_WRAP);
  lv_obj_set_style_text_align(hello, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(hello, LB_Style::accent, 0);
  lv_label_set_text(hello, "Tap a field to type");
  lv_obj_align(hello, LV_ALIGN_BOTTOM_MID, 0, -h / 4);

  kb = lv_keyboard_create(scr);                 // docks to the bottom by itself
  lv_obj_set_height(kb, h / 2);
  lv_obj_add_event_cb(kb, onKeyboard, LV_EVENT_READY, nullptr);
  lv_obj_add_event_cb(kb, onKeyboard, LV_EVENT_CANCEL, nullptr);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);      // until a field is tapped
}

void loop() {
  LB_LVGL.loop();
}
