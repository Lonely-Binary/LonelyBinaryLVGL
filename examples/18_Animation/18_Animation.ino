/*
  18_Animation — make things move smoothly.

  YOU WILL LEARN
    - lv_anim: animate any number from A to B over a time
    - paths (easing): linear, ease in-out, overshoot, bounce
    - playing back (there and back) and repeating forever
    - animating a style property (opacity) with your own callback

  Animations cost redraws. On a big panel without PSRAM, keep moving things
  small; a full-screen fade is the most expensive thing you can ask for.

  TOUCH: not needed.

  TO USE A DIFFERENT SCREEN, change the LB_* constant below.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

// lv_anim calls this with each new value. The variable is whatever you set
// with lv_anim_set_var - here, the ball to move.
static void setX(void *obj, int32_t v) { lv_obj_set_x((lv_obj_t *)obj, v); }
static void setOpa(void *obj, int32_t v) { lv_obj_set_style_opa((lv_obj_t *)obj, v, 0); }

static void ball(lv_obj_t *parent, int32_t y, int32_t size, int32_t travel, lv_color_t color,
                 lv_anim_path_cb_t path, const char *name) {
  lv_obj_t *b = lv_obj_create(parent);
  lv_obj_remove_style_all(b);
  lv_obj_set_size(b, size, size);
  lv_obj_set_style_radius(b, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(b, color, 0);
  lv_obj_set_pos(b, 0, y);

  lv_obj_t *cap = lv_label_create(parent);
  lv_obj_add_style(cap, &LB_Style::label, 0);
  lv_label_set_text(cap, name);
  lv_obj_set_pos(cap, 0, y - 16);

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, b);
  lv_anim_set_exec_cb(&a, setX);
  lv_anim_set_values(&a, 0, travel);             // from x = 0 to x = travel
  lv_anim_set_duration(&a, 1200);                // ms
  lv_anim_set_reverse_duration(&a, 1200);        // then back again
  lv_anim_set_repeat_delay(&a, 300);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_set_path_cb(&a, path);                 // the shape of the motion
  lv_anim_start(&a);                             // LVGL copies `a`; it can go
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();
  const int32_t pad = 10;
  lv_obj_t *area = lv_obj_create(scr);
  lv_obj_remove_style_all(area);
  lv_obj_set_size(area, w - 2 * pad, h - 2 * pad);
  lv_obj_center(area);

  const int32_t rows = 5;
  const int32_t pitch = (h - 2 * pad) / rows;
  const int32_t size = LV_MIN(pitch / 3, 24);
  const int32_t travel = w - 2 * pad - size;
  ball(area, pitch * 0 + pitch - size - 2, size, travel, LB_Style::muted, lv_anim_path_linear, "LINEAR");
  ball(area, pitch * 1 + pitch - size - 2, size, travel, LB_Style::accent, lv_anim_path_ease_in_out, "EASE IN-OUT");
  ball(area, pitch * 2 + pitch - size - 2, size, travel, LB_Style::ok, lv_anim_path_overshoot, "OVERSHOOT");
  ball(area, pitch * 3 + pitch - size - 2, size, travel, LB_Style::warn, lv_anim_path_bounce, "BOUNCE");

  // A word that fades in and out: the same lv_anim, a different property.
  lv_obj_t *hello = lv_label_create(area);
  lv_label_set_text(hello, "Hello, LVGL");
  lv_obj_set_style_text_font(hello, w < 200 ? &lv_font_montserrat_14 : &lv_font_montserrat_24, 0);
  lv_obj_align(hello, LV_ALIGN_BOTTOM_MID, 0, -4);
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, hello);
  lv_anim_set_exec_cb(&a, setOpa);
  lv_anim_set_values(&a, LV_OPA_10, LV_OPA_COVER);
  lv_anim_set_duration(&a, 900);
  lv_anim_set_reverse_duration(&a, 900);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&a);
}

void loop() {
  LB_LVGL.loop();
}
