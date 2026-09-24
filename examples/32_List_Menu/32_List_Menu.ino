/*
  32_List_Menu — a settings menu that goes deeper, with a back button.

  YOU WILL LEARN
    - lv_menu: pages, and items that open another page
    - the header with its title and back arrow, handled for you
    - sections and separators to group items
    - lv_list: a quick list of icon + text buttons (the Wi-Fi page)

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *menu;

// One menu row: icon, text, and optionally a control on the right.
static lv_obj_t *item(lv_obj_t *parent, const char *icon, const char *text) {
  lv_obj_t *c = lv_menu_cont_create(parent);
  lv_obj_t *i = lv_label_create(c);
  lv_label_set_text(i, icon);
  lv_obj_set_style_text_color(i, LB_Style::accent, 0);
  lv_obj_t *t = lv_label_create(c);
  lv_label_set_text(t, text);
  lv_obj_set_flex_grow(t, 1);
  return c;
}

static void onNetwork(lv_event_t *e) {
  lv_obj_t *b = (lv_obj_t *)lv_event_get_target(e);
  Serial.printf("chose %s\n", lv_list_get_button_text(lv_obj_get_parent(b), b));
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  menu = lv_menu_create(lv_screen_active());
  lv_obj_set_size(menu, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(menu, LB_Style::bg, 0);
  // Show a back arrow on the root page too? No: there is nowhere to go back to.
  lv_menu_set_mode_root_back_button(menu, LV_MENU_ROOT_BACK_BUTTON_DISABLED);

  // ── Sub-pages ──────────────────────────────────────────────────────────
  // Wi-Fi: an lv_list, the quickest way to a column of tappable rows.
  lv_obj_t *wifiPage = lv_menu_page_create(menu, "Wi-Fi");
  lv_obj_t *list = lv_list_create(wifiPage);
  lv_obj_set_size(list, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(list, 0, 0);
  lv_obj_t *head = lv_list_add_text(list, "NEARBY");
  lv_obj_add_style(head, &LB_Style::label, 0);
  lv_obj_set_style_bg_opa(head, LV_OPA_TRANSP, 0);
  static const char *nets[] = {"Workshop", "Lonely Binary", "Neighbours 5G", "Printer-Setup"};
  for (auto n : nets) {
    lv_obj_t *b = lv_list_add_button(list, LV_SYMBOL_WIFI, n);
    lv_obj_add_event_cb(b, onNetwork, LV_EVENT_CLICKED, nullptr);
  }

  lv_obj_t *displayPage = lv_menu_page_create(menu, "Display");
  lv_obj_t *sec = lv_menu_section_create(displayPage);
  lv_obj_t *row = item(sec, LV_SYMBOL_IMAGE, "Brightness");
  lv_obj_t *slider = lv_slider_create(row);
  lv_obj_set_width(slider, lv_pct(40));
  lv_slider_set_value(slider, 80, LV_ANIM_OFF);
  row = item(sec, LV_SYMBOL_EYE_CLOSE, "Dim at night");
  lv_switch_create(row);

  lv_obj_t *aboutPage = lv_menu_page_create(menu, "About");
  sec = lv_menu_section_create(aboutPage);
  item(sec, LV_SYMBOL_FILE, "Firmware 1.4.2");
  item(sec, LV_SYMBOL_DRIVE, "Flash 16 MB");
  item(sec, LV_SYMBOL_GPS, "lonelybinary.com");

  // ── Root page ──────────────────────────────────────────────────────────
  // lv_menu_set_load_page_event: tapping this row opens that page.
  lv_obj_t *root = lv_menu_page_create(menu, "Settings");
  sec = lv_menu_section_create(root);
  lv_menu_set_load_page_event(menu, item(sec, LV_SYMBOL_WIFI, "Wi-Fi"), wifiPage);
  lv_menu_set_load_page_event(menu, item(sec, LV_SYMBOL_IMAGE, "Display"), displayPage);
  lv_menu_separator_create(root);
  sec = lv_menu_section_create(root);
  lv_menu_set_load_page_event(menu, item(sec, LV_SYMBOL_LIST, "About"), aboutPage);

  lv_menu_set_page(menu, root);
}

void loop() {
  LB_LVGL.loop();
}
