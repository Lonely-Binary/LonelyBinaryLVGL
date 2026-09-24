/*
  15_LED_Spinner — status lights and "please wait".

  YOU WILL LEARN
    - lv_led: on, off, colour and brightness
    - lv_spinner: an animated "busy" ring, and hiding it when done
    - LV_OBJ_FLAG_HIDDEN: show and hide an object without deleting it

  TOUCH: not needed.

  TO USE A DIFFERENT SCREEN, change the LB_* constant below.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_TFT_24);

static lv_obj_t *power, *net, *alarmLed, *heart;
static lv_obj_t *spinner, *status;

static lv_obj_t *led(lv_obj_t *parent, const char *name, lv_color_t color, int32_t size) {
  lv_obj_t *box = lv_obj_create(parent);
  lv_obj_remove_style_all(box);
  lv_obj_set_size(box, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(box, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(box, 6, 0);

  lv_obj_t *l = lv_led_create(box);
  lv_obj_set_size(l, size, size);
  lv_led_set_color(l, color);
  // The theme's glow is sized for a big LED; on a small one it swamps the
  // lamp. Scale it to the LED instead.
  lv_obj_set_style_shadow_width(l, size / 2, 0);
  lv_obj_set_style_shadow_spread(l, 0, 0);

  lv_obj_t *cap = lv_label_create(box);
  lv_obj_add_style(cap, &LB_Style::label, 0);
  lv_label_set_text(cap, name);
  return l;
}

static void blink(lv_timer_t *) {
  static uint32_t n = 0;
  n++;
  lv_led_on(power);                               // steady
  if (n % 2) lv_led_toggle(net);                  // blinking: traffic
  if (n % 8 < 4) lv_led_on(alarmLed); else lv_led_off(alarmLed);
  // Brightness 0..255 - a heartbeat that fades rather than blinks.
  lv_led_set_brightness(heart, 80 + (uint8_t)(175 * fabsf(sinf(n * 0.4f))));
}

static void work(lv_timer_t *) {
  // Pretend to do something that takes a while, then show the result.
  static int phase = 0;
  phase = (phase + 1) % 4;
  if (phase == 3) {
    lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(status, LV_SYMBOL_OK "  Connected");
    lv_obj_set_style_text_color(status, LB_Style::ok, 0);
  } else {
    lv_obj_remove_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(status, "Connecting...");
    lv_obj_set_style_text_color(status, LB_Style::text, 0);
  }
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();
  const int32_t s = LV_MIN(w, h);
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // A row of four status lights.
  lv_obj_t *row = lv_obj_create(scr);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  const int32_t d = LV_MAX(s / 10, 10);
  power = led(row, "PWR", LB_Style::ok, d);
  net = led(row, "NET", LB_Style::accent, d);
  alarmLed = led(row, "ALM", LB_Style::crit, d);
  heart = led(row, "HB", LB_Style::warn, d);

  // A spinner and a status line under it.
  spinner = lv_spinner_create(scr);
  lv_obj_set_size(spinner, s / 3, s / 3);
  lv_spinner_set_anim_params(spinner, 1000, 60);  // one turn per second, 60-degree arc
  lv_obj_set_style_arc_color(spinner, LB_Style::accent, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(spinner, LB_Style::surface, LV_PART_MAIN);

  status = lv_label_create(scr);
  lv_obj_set_style_text_font(status, s < 200 ? &lv_font_montserrat_14 : &lv_font_montserrat_20, 0);

  work(nullptr);
  lv_timer_create(blink, 250, nullptr);
  lv_timer_create(work, 1500, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
