/*
  42_Settings — a complete settings app: Wi-Fi, display, about.

  PUTS TOGETHER
    - lv_menu pages with a back button (32_List_Menu)
    - a real Wi-Fi scan that does not freeze the screen: started async,
      polled from an lv_timer, with a spinner meanwhile (15_LED_Spinner)
    - joining a network with a password typed on the keyboard (24_Keyboard)
    - brightness on a slider that drives the backlight (22_Slider)
    - settings that survive a reboot, in flash, with Preferences
    - a screen timeout: dims after a while, any touch wakes it
    - a QR code and live system information (17_QRCode)

  FLASH: LVGL plus Wi-Fi fills 97% of the default 1.3 MB app partition on a
  classic ESP32 - it builds, but the first thing you add (MQTT, a web server)
  will not. Choose Tools > Partition Scheme > "Huge APP (3MB No OTA/1MB
  SPIFFS)" before you start extending it.

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <WiFi.h>
#include <Preferences.h>
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);
static Preferences prefs;

// ── Saved settings ─────────────────────────────────────────────────────────
static uint8_t brightness = 80;    // percent
static uint16_t timeoutSec = 60;   // 0 = never
static const uint16_t TIMEOUTS[] = {0, 30, 60, 300};

static lv_obj_t *menu, *wifiPage, *netList, *spinner, *scanButton, *wifiStatus;
static lv_obj_t *passPage, *passTitle, *passField, *keyboard;
static lv_obj_t *infoLabel;
static char chosenSsid[33];

static void toast(const char *text, lv_color_t color) {
  lv_obj_t *t = lv_label_create(lv_layer_top());
  lv_label_set_text(t, text);
  lv_obj_set_style_bg_opa(t, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(t, color, 0);
  lv_obj_set_style_text_color(t, LB_Style::bg, 0);
  lv_obj_set_style_pad_all(t, 8, 0);
  lv_obj_set_style_radius(t, 6, 0);
  lv_obj_align(t, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_delete_delayed(t, 2500);
}

static lv_obj_t *row(lv_obj_t *parent, const char *icon, const char *text) {
  lv_obj_t *c = lv_menu_cont_create(parent);
  lv_obj_t *i = lv_label_create(c);
  lv_label_set_text(i, icon);
  lv_obj_set_style_text_color(i, LB_Style::accent, 0);
  lv_obj_t *t = lv_label_create(c);
  lv_label_set_text(t, text);
  lv_obj_set_flex_grow(t, 1);
  return c;
}

// ── Screen timeout ─────────────────────────────────────────────────────────
// LVGL knows how long since the last touch. Dim when it passes the limit; the
// next touch brings the brightness back. The touch that wakes the screen is
// also delivered to whatever is under the finger - fine for a settings app;
// a sketch where that matters could swallow it.
static void checkIdle(lv_timer_t *) {
  static bool dimmed = false;
  const bool idle = timeoutSec && lv_display_get_inactive_time(nullptr) > timeoutSec * 1000UL;
  if (idle != dimmed) {
    dimmed = idle;
    display.backlight(idle ? 8 : brightness * 255 / 100);
  }
}

// ── Wi-Fi: scan ────────────────────────────────────────────────────────────
static void onNetwork(lv_event_t *e);

static void pollScan(lv_timer_t *t) {
  const int16_t n = WiFi.scanComplete();
  if (n == WIFI_SCAN_RUNNING) return;         // still going: try again next tick
  lv_timer_delete(t);
  lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_state(scanButton, LV_STATE_DISABLED);
  lv_obj_clean(netList);
  if (n <= 0) {
    lv_list_add_text(netList, "No networks found");
    return;
  }
  for (int i = 0; i < n && i < 12; i++) {
    const int32_t rssi = WiFi.RSSI(i);
    const bool open = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
    char text[48];
    snprintf(text, sizeof text, "%s%s", WiFi.SSID(i).c_str(), open ? "" : "  " LV_SYMBOL_EYE_CLOSE);
    lv_obj_t *b = lv_list_add_button(netList, LV_SYMBOL_WIFI, text);
    // Signal strength as the icon's colour.
    lv_obj_t *icon = lv_obj_get_child(b, 0);
    lv_obj_set_style_text_color(icon, rssi > -60 ? LB_Style::ok : rssi > -75 ? LB_Style::warn : LB_Style::crit, 0);
    lv_obj_add_event_cb(b, onNetwork, LV_EVENT_CLICKED, (void *)(intptr_t)i);
  }
  WiFi.scanDelete();
}

static void onScan(lv_event_t *) {
  WiFi.mode(WIFI_STA);
  WiFi.scanNetworks(true);                    // true = async: returns at once
  lv_obj_remove_flag(spinner, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_state(scanButton, LV_STATE_DISABLED);
  lv_timer_create(pollScan, 200, nullptr);
}

// ── Wi-Fi: join ────────────────────────────────────────────────────────────
static void pollJoin(lv_timer_t *t) {
  const wl_status_t s = WiFi.status();
  const uint32_t started = (uint32_t)(uintptr_t)lv_timer_get_user_data(t);
  if (s == WL_CONNECTED) {
    lv_timer_delete(t);
    prefs.putString("ssid", chosenSsid);
    prefs.putString("pass", lv_textarea_get_text(passField));
    lv_label_set_text_fmt(wifiStatus, "Connected to %s", chosenSsid);
    toast(LV_SYMBOL_OK "  Connected", LB_Style::ok);
  } else if (s == WL_CONNECT_FAILED || s == WL_NO_SSID_AVAIL || millis() - started > 15000) {
    lv_timer_delete(t);
    WiFi.disconnect();
    lv_label_set_text(wifiStatus, "Not connected");
    toast(LV_SYMBOL_CLOSE "  Could not connect", LB_Style::crit);
  }
}

static void join(const char *pass) {
  WiFi.begin(chosenSsid, pass);
  lv_label_set_text_fmt(wifiStatus, "Connecting to %s...", chosenSsid);
  lv_menu_set_page(menu, wifiPage);
  lv_timer_create(pollJoin, 250, (void *)(uintptr_t)millis());
}

static void onNetwork(lv_event_t *e) {
  lv_obj_t *b = (lv_obj_t *)lv_event_get_target(e);
  // The button text is "name  <lock>": copy the name part.
  const char *t = lv_list_get_button_text(netList, b);
  size_t n = 0;
  while (t[n] && !(t[n] == ' ' && t[n + 1] == ' ') && n < sizeof chosenSsid - 1) n++;
  memcpy(chosenSsid, t, n);
  chosenSsid[n] = 0;
  if (!strstr(t, LV_SYMBOL_EYE_CLOSE)) { join(nullptr); return; }   // open network
  lv_label_set_text_fmt(passTitle, "Password for %s", chosenSsid);
  lv_textarea_set_text(passField, "");
  lv_menu_set_page(menu, passPage);
}

static void onPassKeyboard(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_READY) join(lv_textarea_get_text(passField));
  else lv_menu_set_page(menu, wifiPage);
}

// ── Display ────────────────────────────────────────────────────────────────
static void onBrightness(lv_event_t *e) {
  brightness = lv_slider_get_value((lv_obj_t *)lv_event_get_target(e));
  display.backlight(brightness * 255 / 100);
}
static void onBrightnessSaved(lv_event_t *) { prefs.putUChar("bright", brightness); }

static void onTimeout(lv_event_t *e) {
  timeoutSec = TIMEOUTS[lv_dropdown_get_selected((lv_obj_t *)lv_event_get_target(e))];
  prefs.putInt("timeout", timeoutSec);
}

// ── About ──────────────────────────────────────────────────────────────────
static void refreshInfo(lv_timer_t *) {
  const uint32_t s = millis() / 1000;
  const bool up = WiFi.status() == WL_CONNECTED;
  lv_label_set_text_fmt(infoLabel,
                        "Chip       %s\n"
                        "Free RAM   %lu KB\n"
                        "Uptime     %luh %02lum %02lus\n"
                        "Wi-Fi      %s\n"
                        "IP         %s\n"
                        "MAC        %s",
                        ESP.getChipModel(), (unsigned long)(ESP.getFreeHeap() / 1024),
                        (unsigned long)(s / 3600), (unsigned long)(s / 60 % 60), (unsigned long)(s % 60),
                        up ? WiFi.SSID().c_str() : "not connected", up ? WiFi.localIP().toString().c_str() : "-",
                        WiFi.macAddress().c_str());
}

// ── Build ──────────────────────────────────────────────────────────────────
static lv_obj_t *buildWifiPage() {
  lv_obj_t *p = lv_menu_page_create(menu, "Wi-Fi");
  lv_obj_t *sec = lv_menu_section_create(p);
  lv_obj_t *r = row(sec, LV_SYMBOL_WIFI, "");
  wifiStatus = lv_obj_get_child(r, 1);
  lv_label_set_text(wifiStatus, "Not connected");
  lv_label_set_long_mode(wifiStatus, LV_LABEL_LONG_MODE_DOTS);
  spinner = lv_spinner_create(r);
  lv_obj_set_size(spinner, 22, 22);
  lv_obj_set_style_arc_width(spinner, 3, LV_PART_MAIN);
  lv_obj_set_style_arc_width(spinner, 3, LV_PART_INDICATOR);
  lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
  scanButton = lv_button_create(r);
  lv_label_set_text(lv_label_create(scanButton), LV_SYMBOL_REFRESH " Scan");
  lv_obj_add_event_cb(scanButton, onScan, LV_EVENT_CLICKED, nullptr);

  netList = lv_list_create(p);
  lv_obj_set_size(netList, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(netList, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(netList, 0, 0);
  lv_list_add_text(netList, "Tap Scan to look for networks");
  return p;
}

static lv_obj_t *buildPassPage() {
  lv_obj_t *p = lv_menu_page_create(menu, "Join");
  lv_obj_set_flex_flow(p, LV_FLEX_FLOW_COLUMN);
  passTitle = lv_label_create(p);
  passField = lv_textarea_create(p);
  lv_textarea_set_one_line(passField, true);
  lv_textarea_set_password_mode(passField, true);
  lv_obj_set_width(passField, lv_pct(100));
  keyboard = lv_keyboard_create(p);
  lv_obj_set_size(keyboard, lv_pct(100), display.height() / 2);
  lv_keyboard_set_textarea(keyboard, passField);
  lv_obj_add_event_cb(keyboard, onPassKeyboard, LV_EVENT_READY, nullptr);
  lv_obj_add_event_cb(keyboard, onPassKeyboard, LV_EVENT_CANCEL, nullptr);
  return p;
}

static lv_obj_t *buildDisplayPage() {
  lv_obj_t *p = lv_menu_page_create(menu, "Display");
  lv_obj_t *sec = lv_menu_section_create(p);
  lv_obj_t *r = row(sec, LV_SYMBOL_IMAGE, "Brightness");
  lv_obj_t *s = lv_slider_create(r);
  lv_obj_set_width(s, lv_pct(45));
  lv_slider_set_range(s, 5, 100);
  lv_slider_set_value(s, brightness, LV_ANIM_OFF);
  lv_obj_add_event_cb(s, onBrightness, LV_EVENT_VALUE_CHANGED, nullptr);
  lv_obj_add_event_cb(s, onBrightnessSaved, LV_EVENT_RELEASED, nullptr);   // write flash once

  r = row(sec, LV_SYMBOL_EYE_CLOSE, "Dim after");
  lv_obj_t *d = lv_dropdown_create(r);
  lv_dropdown_set_options(d, "Never\n30 s\n1 min\n5 min");
  for (int i = 0; i < 4; i++) if (TIMEOUTS[i] == timeoutSec) lv_dropdown_set_selected(d, i);
  lv_obj_set_width(d, 100);
  lv_obj_add_event_cb(d, onTimeout, LV_EVENT_VALUE_CHANGED, nullptr);
  return p;
}

static lv_obj_t *buildAboutPage() {
  lv_obj_t *p = lv_menu_page_create(menu, "About");
  lv_obj_set_flex_flow(p, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(p, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  infoLabel = lv_label_create(p);
  lv_obj_set_style_text_font(infoLabel, &lv_font_montserrat_12, 0);
  lv_obj_t *qr = lv_qrcode_create(p);
  lv_qrcode_set_size(qr, LV_MIN(display.width(), display.height()) / 3);
  lv_qrcode_set_quiet_zone(qr, true);
  lv_qrcode_set_data(qr, "https://lonelybinary.com");
  refreshInfo(nullptr);
  lv_timer_create(refreshInfo, 1000, nullptr);
  return p;
}

void setup() {
  Serial.begin(115200);

  // Settings first, so the screen comes up at the saved brightness.
  prefs.begin("settings");
  brightness = prefs.getUChar("bright", brightness);
  timeoutSec = prefs.getInt("timeout", timeoutSec);

  display.begin(true);
  LB_LVGL.begin(display);
  display.backlight(brightness * 255 / 100);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  menu = lv_menu_create(lv_screen_active());
  lv_obj_set_size(menu, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(menu, LB_Style::bg, 0);
  lv_menu_set_mode_root_back_button(menu, LV_MENU_ROOT_BACK_BUTTON_DISABLED);

  wifiPage = buildWifiPage();
  passPage = buildPassPage();
  lv_obj_t *displayPage = buildDisplayPage();
  lv_obj_t *aboutPage = buildAboutPage();

  lv_obj_t *root = lv_menu_page_create(menu, "Settings");
  lv_obj_t *sec = lv_menu_section_create(root);
  lv_menu_set_load_page_event(menu, row(sec, LV_SYMBOL_WIFI, "Wi-Fi"), wifiPage);
  lv_menu_set_load_page_event(menu, row(sec, LV_SYMBOL_IMAGE, "Display"), displayPage);
  sec = lv_menu_section_create(root);
  lv_menu_set_load_page_event(menu, row(sec, LV_SYMBOL_LIST, "About"), aboutPage);
  lv_menu_set_page(menu, root);

  // Rejoin the saved network, if there is one.
  const String ssid = prefs.getString("ssid");
  if (ssid.length()) {
    strncpy(chosenSsid, ssid.c_str(), sizeof chosenSsid - 1);
    WiFi.mode(WIFI_STA);
    WiFi.begin(chosenSsid, prefs.getString("pass").c_str());
    lv_label_set_text_fmt(wifiStatus, "Connecting to %s...", chosenSsid);
    lv_timer_create(pollJoin, 250, (void *)(uintptr_t)millis());
  }

  lv_timer_create(checkIdle, 500, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
