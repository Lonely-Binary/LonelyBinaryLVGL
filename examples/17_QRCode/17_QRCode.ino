/*
  17_QRCode — a QR code a phone can scan.

  YOU WILL LEARN
    - lv_qrcode: size, colours, and the text it encodes
    - changing what it says at run time (lv_qrcode_update)
    - the Wi-Fi QR format a phone camera understands: join a network by
      pointing the camera at the screen

  Uses: device setup pages, "open the manual", a support link, pairing codes.

  TOUCH: not needed.

  TO USE A DIFFERENT SCREEN, change the LB_* constant below.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_TFT_24);

static lv_obj_t *qr, *caption;

struct Page { const char *text; const char *what; };
static const Page pages[] = {
  {"https://lonelybinary.com", "Visit our website"},
  // WIFI:T:<security>;S:<network name>;P:<password>;;  - scan to join.
  {"WIFI:T:WPA;S:LonelyBinary-Setup;P:12345678;;", "Scan to join Wi-Fi"},
  {"https://github.com/Lonely-Binary", "Code and examples"},
};

static void next(lv_timer_t *) {
  static int i = 0;
  lv_qrcode_update(qr, pages[i].text, strlen(pages[i].text));
  lv_label_set_text(caption, pages[i].what);
  i = (i + 1) % 3;
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();
  const int32_t size = LV_MIN(w, h - 40) - 16;

  // Phones read dark-on-light far more reliably than the reverse, so the QR
  // stays black on white even on a dark screen. The quiet zone is the white
  // margin scanners need around the code.
  qr = lv_qrcode_create(scr);
  lv_qrcode_set_size(qr, size);
  lv_qrcode_set_dark_color(qr, lv_color_black());
  lv_qrcode_set_light_color(qr, lv_color_white());
  lv_qrcode_set_quiet_zone(qr, true);
  lv_obj_align(qr, LV_ALIGN_TOP_MID, 0, 8);

  caption = lv_label_create(scr);
  lv_obj_set_style_text_font(caption, size > 150 ? &lv_font_montserrat_18 : &lv_font_montserrat_12, 0);
  lv_obj_align(caption, LV_ALIGN_BOTTOM_MID, 0, -8);

  next(nullptr);
  lv_timer_create(next, 4000, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
