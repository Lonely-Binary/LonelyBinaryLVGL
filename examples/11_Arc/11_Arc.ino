/*
  11_Arc — round progress rings and gauges.

  YOU WILL LEARN
    - lv_arc: the background arc (MAIN) and the value arc (INDICATOR)
    - rotation and background angles: a full ring, or a 270-degree gauge
    - removing the knob for a display-only arc
    - an arc you can drag: turn the knob, read the value (touch)

  TOUCH: optional. The right-hand arc can be dragged on a touch panel;
  without touch it animates on its own.

  TO USE A DIFFERENT SCREEN, change the LB_* constant below.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *ring, *ringText, *dial, *dialText;

static lv_obj_t *makeArc(lv_obj_t *parent, int32_t size, lv_color_t color) {
  lv_obj_t *a = lv_arc_create(parent);
  lv_obj_set_size(a, size, size);
  lv_obj_set_style_arc_color(a, color, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(a, LB_Style::surface, LV_PART_MAIN);
  const int32_t thick = LV_MAX(size / 10, 4);
  lv_obj_set_style_arc_width(a, thick, LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(a, thick, LV_PART_MAIN);
  return a;
}

// Fires whenever the arc's value changes - by a finger or by code.
static void onDial(lv_event_t *e) {
  lv_obj_t *arc = (lv_obj_t *)lv_event_get_target(e);
  lv_label_set_text_fmt(dialText, "%d", (int)lv_arc_get_value(arc));
}

static void tick(lv_timer_t *) {
  static int v = 0;
  v = (v + 3) % 101;
  lv_arc_set_value(ring, v);
  lv_label_set_text_fmt(ringText, "%d%%", v);
  // Nobody touching it? Move the dial too, so there is something to see.
  if (!LB_LVGL.touch()) {
    lv_arc_set_value(dial, (v * 3) % 101);
    lv_obj_send_event(dial, LV_EVENT_VALUE_CHANGED, nullptr);
  }
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();
  const bool wide = w >= h;
  // Two arcs side by side (or stacked on a portrait screen), as big as fit.
  const int32_t size = wide ? LV_MIN(w / 2, h) - 20 : LV_MIN(w, h / 2) - 20;

  // 1. A full ring: background angles 0..360, knob removed, not clickable.
  ring = makeArc(scr, size, LB_Style::accent);
  lv_arc_set_rotation(ring, 270);                  // start at 12 o'clock
  lv_arc_set_bg_angles(ring, 0, 360);
  lv_obj_remove_style(ring, nullptr, LV_PART_KNOB);
  lv_obj_remove_flag(ring, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_align(ring, wide ? LV_ALIGN_LEFT_MID : LV_ALIGN_TOP_MID, wide ? 10 : 0, wide ? 0 : 10);
  ringText = lv_label_create(ring);
  lv_obj_set_style_text_font(ringText, LB_Style::valueFontFor(size / 2), 0);
  lv_obj_center(ringText);

  // 2. A 270-degree dial with its knob: drag it with a finger.
  dial = makeArc(scr, size, LB_Style::warn);
  lv_arc_set_rotation(dial, 135);
  lv_arc_set_bg_angles(dial, 0, 270);
  lv_arc_set_range(dial, 0, 100);
  lv_arc_set_value(dial, 40);
  lv_obj_set_style_bg_color(dial, LB_Style::text, LV_PART_KNOB);
  lv_obj_align(dial, wide ? LV_ALIGN_RIGHT_MID : LV_ALIGN_BOTTOM_MID, wide ? -10 : 0, wide ? 0 : -10);
  dialText = lv_label_create(dial);
  lv_obj_set_style_text_font(dialText, LB_Style::valueFontFor(size / 2), 0);
  lv_obj_center(dialText);
  lv_obj_add_event_cb(dial, onDial, LV_EVENT_VALUE_CHANGED, nullptr);
  lv_obj_send_event(dial, LV_EVENT_VALUE_CHANGED, nullptr);   // show the start value

  lv_timer_create(tick, 120, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
