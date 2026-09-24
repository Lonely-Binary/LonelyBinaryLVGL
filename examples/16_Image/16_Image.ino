/*
  16_Image — show a picture, and move, turn and tint it.

  YOU WILL LEARN
    - lv_image from a C array (sunset.c, next to this file)
    - LV_IMAGE_DECLARE: how the sketch sees the picture
    - scale (zoom) and rotation, with the pivot point
    - recolour: tint an image, e.g. to grey it out
    - LVGL's built-in symbols as images, no picture file at all

  To use your own picture, read the steps at the top of sunset.c.

  TOUCH: not needed.

  TO USE A DIFFERENT SCREEN, change the LB_* constant below.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

LV_IMAGE_DECLARE(sunset);   // defined in sunset.c

static lv_obj_t *spin;

static void turn(lv_timer_t *) {
  static int32_t angle = 0;
  angle = (angle + 50) % 3600;                  // tenths of a degree
  lv_image_set_rotation(spin, angle);
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();

  // 1. The picture as it is, scaled to fit the top half. 256 = 100%.
  lv_obj_t *big = lv_image_create(scr);
  lv_image_set_src(big, &sunset);
  const int32_t fit = LV_MIN(256 * (w - 16) / 128, 256 * (h / 2 - 8) / 80);
  lv_image_set_scale(big, fit);
  // Scaling does not change the object's layout size; make it the scaled
  // size so alignment works on what you see.
  lv_obj_set_size(big, 128 * fit / 256, 80 * fit / 256);
  lv_image_set_inner_align(big, LV_IMAGE_ALIGN_CENTER);
  lv_obj_align(big, LV_ALIGN_TOP_MID, 0, 8);

  // Bottom half: three small copies side by side.
  lv_obj_t *row = lv_obj_create(scr);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, lv_pct(100), h / 2 - 8);
  lv_obj_align(row, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  const int32_t small = LV_MIN(256 * (w / 3 - 12) / 128, 256 * (h / 2 - 24) / 128);

  // 2. Turning. Rotation is in 0.1 degree steps around the pivot (default:
  //    the centre). Rotating costs CPU; small images turn smoothly.
  spin = lv_image_create(row);
  lv_image_set_src(spin, &sunset);
  lv_image_set_scale(spin, small);
  lv_obj_set_size(spin, 128 * small / 256, 128 * small / 256);   // room to turn
  lv_image_set_inner_align(spin, LV_IMAGE_ALIGN_CENTER);

  // 3. Tinted. recolor mixes a colour in; opa says how much.
  lv_obj_t *grey = lv_image_create(row);
  lv_image_set_src(grey, &sunset);
  lv_image_set_scale(grey, small);
  lv_obj_set_size(grey, 128 * small / 256, 80 * small / 256);
  lv_image_set_inner_align(grey, LV_IMAGE_ALIGN_CENTER);
  lv_obj_set_style_image_recolor(grey, LB_Style::muted, 0);
  lv_obj_set_style_image_recolor_opa(grey, LV_OPA_70, 0);

  // 4. A symbol as an image: no file, takes the text colour.
  lv_obj_t *icon = lv_image_create(row);
  lv_image_set_src(icon, LV_SYMBOL_IMAGE);
  lv_obj_set_style_text_font(icon, &lv_font_montserrat_40, 0);
  lv_obj_set_style_text_color(icon, LB_Style::accent, 0);

  lv_timer_create(turn, 40, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
