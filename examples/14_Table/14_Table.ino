/*
  14_Table — rows and columns of text.

  YOU WILL LEARN
    - lv_table: rows, columns, column widths, cell text
    - styling a header row differently from the rest (a draw event)
    - updating one cell without touching the others
    - a table longer than the screen scrolls (by finger, on a touch panel)

  TOUCH: optional, to scroll.

  TO USE A DIFFERENT SCREEN, change the LB_* constant below.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *table;

struct Room { const char *name; float temp; int hum; };
static Room rooms[] = {
  {"Kitchen", 22.4, 51}, {"Lounge", 21.0, 48}, {"Bedroom", 19.6, 55}, {"Study", 20.8, 47},
  {"Bath", 23.9, 71}, {"Garage", 14.2, 62}, {"Attic", 26.3, 39}, {"Cellar", 12.8, 74},
};
static const int N = sizeof rooms / sizeof rooms[0];

// Every cell is drawn through this event. Row 0 is the header: give it the
// accent colour. Anything else keeps the theme.
static void onDraw(lv_event_t *e) {
  lv_draw_task_t *task = lv_event_get_draw_task(e);
  lv_draw_dsc_base_t *base = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(task);
  if (base->part != LV_PART_ITEMS) return;
  const uint32_t row = base->id1;
  if (row == 0) {
    lv_draw_fill_dsc_t *fill = lv_draw_task_get_fill_dsc(task);
    if (fill) { fill->color = LB_Style::accent; fill->opa = LV_OPA_COVER; }
    lv_draw_label_dsc_t *text = lv_draw_task_get_label_dsc(task);
    if (text) text->color = LB_Style::bg;
  } else if (row % 2 == 0) {
    // Zebra stripes make wide rows easier to follow.
    lv_draw_fill_dsc_t *fill = lv_draw_task_get_fill_dsc(task);
    if (fill) fill->color = lv_color_mix(LB_Style::surface, LB_Style::bg, LV_OPA_50);
  }
}

static void refresh(lv_timer_t *) {
  // A reading changes: rewrite just that cell.
  const int i = random(0, N);
  rooms[i].temp += random(-5, 6) / 10.0f;
  lv_table_set_cell_value_fmt(table, i + 1, 1, "%.1f", rooms[i].temp);
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  const int32_t w = display.width(), h = display.height();
  table = lv_table_create(lv_screen_active());
  lv_obj_set_size(table, w, h);                 // fills the screen; scrolls inside
  lv_obj_center(table);

  lv_table_set_column_count(table, 3);
  lv_table_set_row_count(table, N + 1);         // +1 for the header
  // Column widths in pixels: half for the name, a quarter each for numbers.
  lv_table_set_column_width(table, 0, w / 2 - 2);
  lv_table_set_column_width(table, 1, w / 4);
  lv_table_set_column_width(table, 2, w / 4);

  lv_table_set_cell_value(table, 0, 0, "Room");
  lv_table_set_cell_value(table, 0, 1, "C");
  lv_table_set_cell_value(table, 0, 2, "%RH");
  for (int i = 0; i < N; i++) {
    lv_table_set_cell_value(table, i + 1, 0, rooms[i].name);
    lv_table_set_cell_value_fmt(table, i + 1, 1, "%.1f", rooms[i].temp);
    lv_table_set_cell_value_fmt(table, i + 1, 2, "%d", rooms[i].hum);
  }

  // Tighter cells than the default on small screens.
  const bool small = w < 200 || h < 200;
  lv_obj_set_style_pad_ver(table, small ? 3 : 8, LV_PART_ITEMS);
  lv_obj_set_style_pad_hor(table, small ? 3 : 8, LV_PART_ITEMS);
  lv_obj_set_style_text_font(table, small ? &lv_font_montserrat_12 : &lv_font_montserrat_16, LV_PART_ITEMS);
  lv_obj_set_style_border_width(table, 0, 0);

  lv_obj_add_event_cb(table, onDraw, LV_EVENT_DRAW_TASK_ADDED, nullptr);
  lv_obj_add_flag(table, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);   // needed for onDraw

  lv_timer_create(refresh, 1500, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
