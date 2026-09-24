/*
  28_MsgBox — ask "are you sure?", and small pop-up notices.

  YOU WILL LEARN
    - lv_msgbox: title, text, and footer buttons
    - modal: lv_msgbox_create(NULL) greys out and blocks the screen behind
    - which footer button was pressed, and closing the box
    - a "toast": a notice that appears and removes itself (lv_obj_delete_delayed)

  TOUCH: required. Use a touch panel such as LB_SQUARE_392_CTP.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *files;

static void toast(const char *text, lv_color_t color) {
  // A plain label on the top layer, which sits above every screen.
  lv_obj_t *t = lv_label_create(lv_layer_top());
  lv_label_set_text(t, text);
  lv_obj_set_style_bg_opa(t, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(t, color, 0);
  lv_obj_set_style_text_color(t, LB_Style::bg, 0);
  lv_obj_set_style_pad_all(t, 10, 0);
  lv_obj_set_style_radius(t, 6, 0);
  lv_obj_align(t, LV_ALIGN_BOTTOM_MID, 0, -12);
  lv_obj_delete_delayed(t, 2000);              // gone in two seconds
}

// Every footer button calls this; its label says which it was.
static void onAnswer(lv_event_t *e) {
  lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
  lv_obj_t *box = (lv_obj_t *)lv_event_get_user_data(e);
  const char *text = lv_label_get_text(lv_obj_get_child(btn, 0));
  if (!strcmp(text, "Delete")) {
    lv_obj_t *row = lv_obj_get_child(files, 0);
    if (row) lv_obj_delete(row);
    toast(LV_SYMBOL_TRASH "  Deleted", LB_Style::crit);
  } else {
    toast("Kept", LB_Style::ok);
  }
  lv_msgbox_close(box);
}

static void onDeleteTapped(lv_event_t *) {
  if (!lv_obj_get_child_count(files)) { toast("Nothing left to delete", LB_Style::warn); return; }
  const char *name = lv_label_get_text(lv_obj_get_child(lv_obj_get_child(files, 0), 0));

  lv_obj_t *box = lv_msgbox_create(nullptr);   // NULL parent = modal
  lv_msgbox_add_title(box, "Delete file?");
  lv_msgbox_add_text_fmt(box, "\"%s\" will be gone for good.", name);
  lv_obj_t *no = lv_msgbox_add_footer_button(box, "Cancel");
  lv_obj_t *yes = lv_msgbox_add_footer_button(box, "Delete");
  lv_obj_set_style_bg_color(yes, LB_Style::crit, 0);
  lv_obj_set_style_bg_color(no, LB_Style::surface, 0);
  lv_obj_add_event_cb(no, onAnswer, LV_EVENT_CLICKED, box);
  lv_obj_add_event_cb(yes, onAnswer, LV_EVENT_CLICKED, box);
  lv_obj_set_width(box, LV_MIN(display.width() - 20, 280));
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);
  if (!LB_LVGL.touch()) Serial.println("This example needs a touch panel, e.g. LB_SQUARE_392_CTP.");

  lv_obj_t *scr = lv_screen_active();
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(scr, 10, 0);
  lv_obj_set_style_pad_row(scr, 8, 0);

  // A list of "files" to delete from.
  files = lv_obj_create(scr);
  lv_obj_remove_style_all(files);
  lv_obj_set_width(files, lv_pct(100));
  lv_obj_set_flex_grow(files, 1);
  lv_obj_set_flex_flow(files, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(files, 6, 0);
  static const char *names[] = {"log_0921.csv", "log_0922.csv", "log_0923.csv", "photo.jpg", "notes.txt"};
  for (auto n : names) {
    lv_obj_t *row = lv_obj_create(files);
    lv_obj_add_style(row, &LB_Style::card, 0);
    lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_t *l = lv_label_create(row);
    lv_label_set_text(l, n);
  }

  lv_obj_t *del = lv_button_create(scr);
  lv_obj_set_width(del, lv_pct(100));
  lv_obj_set_style_bg_color(del, LB_Style::crit, 0);
  lv_obj_t *l = lv_label_create(del);
  lv_label_set_text(l, LV_SYMBOL_TRASH "  Delete the first file");
  lv_obj_center(l);
  lv_obj_add_event_cb(del, onDeleteTapped, LV_EVENT_CLICKED, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
