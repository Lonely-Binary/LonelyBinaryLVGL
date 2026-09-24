/*
  01_Label — text on the screen, and text that changes.

  YOU WILL LEARN
    - lv_label: set the text, pick a font, align it
    - what a label does with text too long for it: wrap, dots, scroll
    - LVGL's built-in icons (LV_SYMBOL_...)
    - lv_timer: the right place to update the screen, instead of loop()

  TOUCH: not needed.

  TO USE A DIFFERENT SCREEN, change the LB_* constant below. Any Lonely Binary
  panel works; the layout follows the screen size.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_TFT_24);

static lv_obj_t *counter;

// Called by LVGL every 1000 ms (see lv_timer_create below). Timers run inside
// LB_LVGL.loop(), so it is always safe to touch LVGL objects from here.
static void tick(lv_timer_t *) {
  static uint32_t seconds = 0;
  lv_label_set_text_fmt(counter, "Uptime %lu s", (unsigned long)++seconds);
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  lv_obj_t *scr = lv_screen_active();
  const bool small = display.width() < 200 || display.height() < 200;

  // Stack everything top to bottom. (How this works is example 03; for now it
  // just saves working out y positions by hand.)
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(scr, small ? 4 : 10, 0);
  lv_obj_set_style_pad_row(scr, small ? 4 : 8, 0);

  // 1. A title. Fonts are Montserrat 8..48; bigger ones cost more flash, and
  //    only the sizes you use are linked in.
  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "Labels");
  lv_obj_set_style_text_font(title, small ? &lv_font_montserrat_16 : &lv_font_montserrat_28, 0);
  lv_obj_set_style_text_color(title, LB_Style::accent, 0);

  // 2. Wrapping. Give the label a width and long text wraps onto new lines;
  //    the height grows to fit.
  lv_obj_t *para = lv_label_create(scr);
  lv_obj_set_width(para, lv_pct(100));
  lv_label_set_long_mode(para, LV_LABEL_LONG_MODE_WRAP);
  lv_label_set_text(para, "A label with a fixed width wraps its text onto as many lines as it needs.");
  lv_obj_set_style_text_align(para, LV_TEXT_ALIGN_CENTER, 0);

  // 3. Dots. Same width, one line: the end is cut off with "...".
  lv_obj_t *dots = lv_label_create(scr);
  lv_obj_set_width(dots, lv_pct(100));
  lv_label_set_long_mode(dots, LV_LABEL_LONG_MODE_DOTS);
  lv_label_set_text(dots, "This line is far too long, so it ends in dots");
  lv_obj_set_style_text_color(dots, LB_Style::muted, 0);

  // 4. Marquee. One line that scrolls round forever - for a song title, a
  //    ticker, a long network name.
  lv_obj_t *marquee = lv_label_create(scr);
  lv_obj_set_width(marquee, lv_pct(100));
  lv_label_set_long_mode(marquee, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
  lv_label_set_text(marquee, "This one scrolls round and round, like a shop sign.   ");

  // 5. Icons. They are ordinary text in a built-in font, so they mix with
  //    words and take the label's colour.
  lv_obj_t *icons = lv_label_create(scr);
  lv_label_set_text(icons, LV_SYMBOL_WIFI "  " LV_SYMBOL_BLUETOOTH "  " LV_SYMBOL_BATTERY_3
                           "  " LV_SYMBOL_BELL "  " LV_SYMBOL_SETTINGS);
  lv_obj_set_style_text_color(icons, LB_Style::ok, 0);

  // 6. Text that changes. lv_label_set_text_fmt works like printf.
  counter = lv_label_create(scr);
  lv_obj_set_style_text_font(counter, small ? &lv_font_montserrat_16 : &lv_font_montserrat_24, 0);
  lv_label_set_text(counter, "Uptime 0 s");
  lv_timer_create(tick, 1000, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
