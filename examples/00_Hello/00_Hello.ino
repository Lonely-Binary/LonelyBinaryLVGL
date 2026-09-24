/*
  00_Hello — a live LVGL gauge and readout on a Lonely Binary screen.

  The first of a series that goes from here to a full settings app, one idea
  per example. Start at 00 and go in order; each one assumes the ones before.

  YOU WILL LEARN
    - the four lines every LVGL sketch here starts with
    - that LVGL owns the screen: you create objects, it draws them
    - that loop() must call LB_LVGL.loop() often, or nothing moves

  Nothing to configure. No lv_conf.h to copy, no flush callback to write.

  TO USE A DIFFERENT SCREEN, CHANGE ONE LINE — the LB_* constant below:
      LB_TFT_096  LB_TFT_18  LB_TFT_20  LB_TFT_24  LB_TFT_28  LB_TFT_35
      LB_NARROW_114  LB_NARROW_168  LB_NARROW_19  LB_NARROW_225  LB_NARROW_279
      LB_SQUARE_392_CTP  (touch)

  Tools > Board: "ESP32S3 Dev Module" or "ESP32 Dev Module".
  On a board with PSRAM, also set Tools > PSRAM to Enabled — display.begin(true)
  then renders with no copy at all.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_TFT_24);

static lv_obj_t *arc;
static lv_obj_t *reading;

void setup() {
  Serial.begin(115200);
  delay(300);

  // true asks for a framebuffer. Without the room for one (no PSRAM) it says
  // so and carries on without, and LVGL renders in bands instead - so it is
  // always safe to ask.
  display.begin(true);
  if (!LB_LVGL.begin(display)) {
    Serial.println("LVGL failed to start");
    return;
  }

  display.printInfo();
  Serial.printf("LVGL zero-copy framebuffer: %s\n",
                LB_LVGL.usingFramebuffer() ? "yes" : "no");

  lv_obj_t *scr = lv_screen_active();

  // Caption
  lv_obj_t *cap = lv_label_create(scr);
  lv_obj_add_style(cap, &LB_Style::label, 0);
  lv_label_set_text(cap, "TEMPERATURE");
  lv_obj_align(cap, LV_ALIGN_TOP_MID, 0, 8);

  // The gauge. lv_arc is anti-aliased because lv_conf.h turns on
  // LV_DRAW_SW_COMPLEX — that is the difference between this and a staircase.
  arc = lv_arc_create(scr);
  int16_t d = LV_MIN(display.width(), display.height()) - 40;
  lv_obj_set_size(arc, d, d);
  lv_arc_set_rotation(arc, 135);
  lv_arc_set_bg_angles(arc, 0, 270);
  lv_arc_set_range(arc, 0, 100);
  lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
  lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_color(arc, LB_Style::accent, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(arc, LB_Style::muted, LV_PART_MAIN);
  lv_obj_center(arc);

  // The number, sized to the panel.
  reading = lv_label_create(scr);
  lv_obj_add_style(reading, &LB_Style::value, 0);
  lv_obj_set_style_text_font(reading, LB_Style::valueFontFor(d), 0);
  lv_label_set_text(reading, "--");
  lv_obj_center(reading);
}

void loop() {
  static uint32_t last = 0;
  static int v = 0;
  static int dir = 1;

  if (millis() - last > 60) {
    last = millis();
    v += dir;
    if (v >= 100 || v <= 0) dir = -dir;

    lv_arc_set_value(arc, v);
    lv_label_set_text_fmt(reading, "%d", v);

    // Same three-state colour mapping everything else uses.
    LB_Style::setLevel(reading, v > 85   ? LB_Style::LEVEL_CRIT
                              : v > 70   ? LB_Style::LEVEL_WARN
                                         : LB_Style::LEVEL_OK);
  }

  LB_LVGL.loop();     // must be called often
}
