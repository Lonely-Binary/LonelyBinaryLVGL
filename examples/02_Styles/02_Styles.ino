/*
  02_Styles — how things look, and how to change it in one place.

  YOU WILL LEARN
    - the stock look you get for free (LVGL's theme, in house colours)
    - LB_Style: the Lonely Binary tile, caption, value and unit
    - lv_style_t: build your own style once, apply it to many objects
    - local styles (lv_obj_set_style_...) for a one-off change
    - states: a style that only applies while an object is pressed

  TOUCH: optional. With touch, press the right-hand tile to see the pressed
  state. Without, everything else still shows.

  TO USE A DIFFERENT SCREEN, change the LB_* constant below.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

// A style is an object you build once and share. Keep it static or global:
// the objects using it point at it, they do not copy it.
static lv_style_t pill;
static lv_style_t pillPressed;

static lv_obj_t *reading;

static void update(lv_timer_t *) {
  static int v = 20;
  v = v >= 100 ? 20 : v + 7;
  lv_label_set_text_fmt(reading, "%d", v);
  // The same three colours for OK / warning / critical everywhere.
  LB_Style::setLevel(reading, v > 85 ? LB_Style::LEVEL_CRIT
                            : v > 65 ? LB_Style::LEVEL_WARN
                                     : LB_Style::LEVEL_OK);
}

static lv_obj_t *caption(lv_obj_t *parent, const char *text) {
  lv_obj_t *l = lv_label_create(parent);
  lv_obj_add_style(l, &LB_Style::label, 0);
  lv_label_set_text(l, text);
  return l;
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();
  const int32_t pad = w < 200 ? 4 : 10;
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);
  lv_obj_set_style_pad_all(scr, pad, 0);
  lv_obj_set_style_pad_gap(scr, pad, 0);

  // Four tiles, two by two (or in a column on a strip display).
  const bool strip = w > 2 * h;
  const int32_t tw = strip ? (w - 5 * pad) / 4 : (w - 3 * pad) / 2;
  const int32_t th = strip ? h - 2 * pad : (h - 3 * pad) / 2;

  // 1. Stock. An lv_obj with no styling of its own: LVGL's default theme,
  //    which LB_LVGL sets up dark, in the house accent colour.
  lv_obj_t *stock = lv_obj_create(scr);
  lv_obj_set_size(stock, tw, th);
  caption(stock, "STOCK");

  // 2. LB_Style. The Lonely Binary tile: flat, hairline border, a caption, a
  //    big number and its unit.
  lv_obj_t *tile = lv_obj_create(scr);
  lv_obj_set_size(tile, tw, th);
  lv_obj_add_style(tile, &LB_Style::card, 0);
  caption(tile, "LB_STYLE");
  reading = lv_label_create(tile);
  lv_obj_add_style(reading, &LB_Style::value, 0);
  lv_obj_set_style_text_font(reading, LB_Style::valueFontFor(th - 30), 0);
  lv_obj_align(reading, LV_ALIGN_CENTER, 0, 6);
  lv_obj_t *unit = lv_label_create(tile);
  lv_obj_add_style(unit, &LB_Style::unit, 0);
  lv_label_set_text(unit, "%");
  lv_obj_align(unit, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  update(nullptr);
  lv_timer_create(update, 800, nullptr);

  // 3. Your own style. Every lv_style_set_... is one property; anything you
  //    do not set comes from the theme.
  lv_style_init(&pill);
  lv_style_set_radius(&pill, LV_RADIUS_CIRCLE);        // fully rounded ends
  lv_style_set_bg_color(&pill, LB_Style::accent);
  lv_style_set_bg_opa(&pill, LV_OPA_20);
  lv_style_set_border_color(&pill, LB_Style::accent);
  lv_style_set_border_width(&pill, 2);
  lv_style_set_text_color(&pill, LB_Style::accent);

  // A second style for the PRESSED state only: while a finger is on the
  // object it is added on top of the first.
  lv_style_init(&pillPressed);
  lv_style_set_bg_opa(&pillPressed, LV_OPA_COVER);
  lv_style_set_text_color(&pillPressed, LB_Style::bg);
  lv_style_set_transform_scale(&pillPressed, 240);     // 256 = 100%

  lv_obj_t *mine = lv_obj_create(scr);
  lv_obj_set_size(mine, tw, th);
  lv_obj_add_style(mine, &pill, 0);
  lv_obj_add_style(mine, &pillPressed, LV_STATE_PRESSED);
  lv_obj_t *t = lv_label_create(mine);
  lv_label_set_text(t, "PRESS ME");
  lv_obj_center(t);

  // 4. Local style. lv_obj_set_style_... changes one object only. Handy for a
  //    one-off; for anything repeated, a shared style (3) is cheaper.
  lv_obj_t *local = lv_obj_create(scr);
  lv_obj_set_size(local, tw, th);
  lv_obj_add_style(local, &LB_Style::card, 0);
  lv_obj_set_style_border_color(local, LB_Style::crit, 0);
  lv_obj_set_style_border_width(local, 3, 0);
  lv_obj_set_style_border_opa(local, LV_OPA_COVER, 0);
  lv_obj_set_style_border_side(local, LV_BORDER_SIDE_LEFT, 0);
  caption(local, "LOCAL");
  lv_obj_t *warn = lv_label_create(local);
  lv_label_set_text(warn, LV_SYMBOL_WARNING " Alert");
  lv_obj_set_style_text_color(warn, LB_Style::crit, 0);
  lv_obj_center(warn);
}

void loop() {
  LB_LVGL.loop();
}
