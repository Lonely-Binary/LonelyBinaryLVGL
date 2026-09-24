/*
  33_Screens — whole screens, and moving between them with an animation.

  YOU WILL LEARN
    - a screen is an object with no parent: lv_obj_create(NULL)
    - lv_screen_load_anim: slide or fade to another screen
    - building each screen once and keeping it, versus rebuilding it
    - a swipe gesture (LV_EVENT_GESTURE) to go back
    - lv_layer_top: a status bar that stays put while screens change

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *homeScreen, *detailScreen, *detailTitle;

static void goHome() {
  lv_screen_load_anim(homeScreen, LV_SCREEN_LOAD_ANIM_MOVE_RIGHT, 250, 0, false);
}

// Swipe right anywhere on the detail screen = back.
static void onGesture(lv_event_t *) {
  if (lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_RIGHT) goHome();
}

static void onBack(lv_event_t *) { goHome(); }

static void onOpen(lv_event_t *e) {
  lv_label_set_text(detailTitle, (const char *)lv_event_get_user_data(e));
  // false: keep the home screen (we come back to it). true would delete it.
  lv_screen_load_anim(detailScreen, LV_SCREEN_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
}

static lv_obj_t *newScreen() {
  lv_obj_t *s = lv_obj_create(nullptr);        // no parent: a screen
  lv_obj_set_style_bg_color(s, LB_Style::bg, 0);
  lv_obj_set_style_pad_top(s, 34, 0);           // room for the status bar
  lv_obj_set_style_pad_hor(s, 10, 0);
  lv_obj_set_style_pad_bottom(s, 10, 0);
  return s;
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  // ── Status bar on the top layer: above every screen, never slides ─────
  lv_obj_t *bar = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(bar);
  lv_obj_set_size(bar, lv_pct(100), 26);
  lv_obj_set_style_bg_color(bar, LB_Style::surface, 0);
  lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
  lv_obj_t *clock = lv_label_create(bar);
  lv_label_set_text(clock, "09:41");
  lv_obj_align(clock, LV_ALIGN_LEFT_MID, 10, 0);
  lv_obj_t *icons = lv_label_create(bar);
  lv_label_set_text(icons, LV_SYMBOL_WIFI "  " LV_SYMBOL_BATTERY_3);
  lv_obj_align(icons, LV_ALIGN_RIGHT_MID, -10, 0);

  // ── Home: a grid of big buttons ────────────────────────────────────────
  homeScreen = newScreen();
  lv_obj_set_flex_flow(homeScreen, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(homeScreen, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_SPACE_EVENLY);
  static const char *rooms[] = {LV_SYMBOL_HOME " Lounge", LV_SYMBOL_POWER " Kitchen", LV_SYMBOL_EYE_OPEN " Bedroom",
                                LV_SYMBOL_CHARGE " Garage"};
  for (auto r : rooms) {
    lv_obj_t *b = lv_button_create(homeScreen);
    lv_obj_set_size(b, lv_pct(45), lv_pct(42));
    lv_obj_set_style_bg_color(b, LB_Style::surface, 0);
    lv_obj_t *l = lv_label_create(b);
    lv_label_set_text(l, r);
    lv_obj_center(l);
    lv_obj_add_event_cb(b, onOpen, LV_EVENT_CLICKED, (void *)r);
  }

  // ── Detail: built once, its title changed each time it opens ──────────
  detailScreen = newScreen();
  lv_obj_t *back = lv_button_create(detailScreen);
  lv_obj_set_style_bg_color(back, LB_Style::surface, 0);
  lv_label_set_text(lv_label_create(back), LV_SYMBOL_LEFT " Back");
  lv_obj_align(back, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_add_event_cb(back, onBack, LV_EVENT_CLICKED, nullptr);
  detailTitle = lv_label_create(detailScreen);
  lv_obj_set_style_text_font(detailTitle, &lv_font_montserrat_24, 0);
  lv_obj_align(detailTitle, LV_ALIGN_TOP_MID, 0, 56);
  lv_obj_t *hint = lv_label_create(detailScreen);
  lv_obj_set_style_text_color(hint, LB_Style::muted, 0);
  lv_label_set_text(hint, "Swipe right to go back");
  lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_event_cb(detailScreen, onGesture, LV_EVENT_GESTURE, nullptr);

  lv_screen_load(homeScreen);
}

void loop() {
  LB_LVGL.loop();
}
