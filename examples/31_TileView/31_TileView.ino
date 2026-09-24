/*
  31_TileView — swipe between full-screen pages, like a phone.

  YOU WILL LEARN
    - lv_tileview: tiles on a grid; swipe left/right/up/down between them
    - which directions each tile allows (LV_DIR_LEFT, LV_DIR_RIGHT, both...)
    - jumping to a tile from code (lv_tileview_set_tile_by_index)
    - page dots that follow the swipe

  Tabs (30) suit a few labelled pages; tiles suit pages you flick through,
  where the whole screen is the content.

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static const int PAGES = 3;
static lv_obj_t *dots[PAGES];

static void showDot(int active) {
  for (int i = 0; i < PAGES; i++)
    lv_obj_set_style_bg_color(dots[i], i == active ? LB_Style::accent : LB_Style::muted, 0);
}

static void onScroll(lv_event_t *e) {
  lv_obj_t *tv = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *tile = lv_tileview_get_tile_active(tv);
  showDot(lv_obj_get_x(tile) / lv_obj_get_width(tv));
}

static lv_obj_t *page(lv_obj_t *tv, int col, lv_dir_t dirs, const char *icon, const char *title,
                      const char *value, lv_color_t color) {
  lv_obj_t *t = lv_tileview_add_tile(tv, col, 0, dirs);
  lv_obj_t *i = lv_label_create(t);
  lv_label_set_text(i, icon);
  lv_obj_set_style_text_font(i, &lv_font_montserrat_40, 0);
  lv_obj_set_style_text_color(i, color, 0);
  lv_obj_align(i, LV_ALIGN_CENTER, 0, -60);
  lv_obj_t *v = lv_label_create(t);
  lv_obj_add_style(v, &LB_Style::value, 0);
  lv_obj_set_style_text_font(v, &lv_font_montserrat_48, 0);
  lv_label_set_text(v, value);
  lv_obj_center(v);
  lv_obj_t *c = lv_label_create(t);
  lv_obj_add_style(c, &LB_Style::label, 0);
  lv_label_set_text(c, title);
  lv_obj_align(c, LV_ALIGN_CENTER, 0, 44);
  return t;
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  lv_obj_t *scr = lv_screen_active();
  lv_obj_t *tv = lv_tileview_create(scr);
  lv_obj_set_style_bg_color(tv, LB_Style::bg, 0);
  // A tileview shows scrollbars by default; the dots do that job better.
  lv_obj_set_scrollbar_mode(tv, LV_SCROLLBAR_MODE_OFF);

  // Three tiles in a row. The first may only go right, the last only left.
  page(tv, 0, LV_DIR_RIGHT, LV_SYMBOL_CHARGE, "SOLAR TODAY  (KWH)", "12.4", LB_Style::warn);
  // In C++ OR-ing two directions gives an int, hence the cast.
  page(tv, 1, (lv_dir_t)(LV_DIR_LEFT | LV_DIR_RIGHT), LV_SYMBOL_HOME, "HOUSE NOW  (KW)", "1.8", LB_Style::accent);
  page(tv, 2, LV_DIR_LEFT, LV_SYMBOL_BATTERY_3, "BATTERY  (%)", "76", LB_Style::ok);

  // Page dots on top of the tileview (a sibling, so they do not scroll).
  lv_obj_t *row = lv_obj_create(scr);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(row, 8, 0);
  lv_obj_align(row, LV_ALIGN_BOTTOM_MID, 0, -14);
  for (int i = 0; i < PAGES; i++) {
    dots[i] = lv_obj_create(row);
    lv_obj_remove_style_all(dots[i]);
    lv_obj_set_size(dots[i], 8, 8);
    lv_obj_set_style_radius(dots[i], LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(dots[i], LV_OPA_COVER, 0);
  }

  // Start on the middle page, so there is somewhere to swipe either way.
  lv_tileview_set_tile_by_index(tv, 1, 0, LV_ANIM_OFF);
  showDot(1);
  lv_obj_add_event_cb(tv, onScroll, LV_EVENT_VALUE_CHANGED, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
