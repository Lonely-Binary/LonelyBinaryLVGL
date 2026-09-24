/*
  27_Calendar — pick a date.

  YOU WILL LEARN
    - lv_calendar: today's date, which month is shown
    - a header with arrows to change month (lv_calendar_header_arrow_create)
    - highlighting dates (holidays, bookings, bin day)
    - which day was tapped: lv_calendar_get_pressed_date

  The calendar does not know today's date: it has no clock. Set it from
  NTP, an RTC chip or GPS - here it is simply written in.

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *cal, *picked;

static void onPick(lv_event_t *) {
  lv_calendar_date_t d;
  if (lv_calendar_get_pressed_date(cal, &d) == LV_RESULT_OK)
    lv_label_set_text_fmt(picked, LV_SYMBOL_BELL "  Booked %04d-%02d-%02d", d.year, d.month, d.day);
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();

  cal = lv_calendar_create(scr);
  lv_obj_set_size(cal, w - 8, h - 36);
  lv_obj_align(cal, LV_ALIGN_TOP_MID, 0, 4);
  lv_calendar_set_today_date(cal, 2026, 9, 24);
  lv_calendar_set_month_shown(cal, 2026, 9);
  lv_calendar_header_arrow_create(cal);          // "<  September 2026  >"
  if (w < 280) lv_obj_set_style_text_font(cal, &lv_font_montserrat_12, 0);

  // Highlighted dates are drawn in the accent colour. The array must stay
  // alive (static), because the calendar keeps a pointer to it.
  static lv_calendar_date_t bins[] = {{2026, 9, 3}, {2026, 9, 17}, {2026, 10, 1}, {2026, 10, 15}};
  lv_calendar_set_highlighted_dates(cal, bins, 4);

  lv_obj_add_event_cb(cal, onPick, LV_EVENT_VALUE_CHANGED, nullptr);

  picked = lv_label_create(scr);
  lv_label_set_text(picked, "Highlighted: bin day");
  lv_obj_set_style_text_color(picked, LB_Style::muted, 0);
  lv_obj_align(picked, LV_ALIGN_BOTTOM_MID, 0, -6);
}

void loop() {
  LB_LVGL.loop();
}
