/*
  03_Layout — let LVGL place things, so one sketch fits every screen.

  YOU WILL LEARN
    - lv_obj_align: place one object relative to its parent
    - Flex: a row or column that spaces its children for you
    - flex_grow: "take whatever space is left"
    - Grid: rows and columns, like a table, sized in fractions (LV_GRID_FR)
    - sizes in percent (lv_pct) instead of pixels

  Change the panel constant and run it again: the same code lays out a
  square, a portrait and a strip display correctly. That is the point.

  TOUCH: not needed.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

static lv_obj_t *makeTile(lv_obj_t *parent, const char *name, const char *value, const char *unit) {
  lv_obj_t *tile = lv_obj_create(parent);
  lv_obj_add_style(tile, &LB_Style::card, 0);
  lv_obj_remove_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

  // Inside the tile, alignment is enough: caption top-left, value centred.
  lv_obj_t *cap = lv_label_create(tile);
  lv_obj_add_style(cap, &LB_Style::label, 0);
  lv_label_set_text(cap, name);
  lv_obj_align(cap, LV_ALIGN_TOP_LEFT, 0, 0);

  // The number and its unit sit side by side, bottoms lined up. A tiny flex
  // row does that, and keeps doing it when the number gets wider. (Aligning
  // the unit to the number with lv_obj_align_to would not: it is computed
  // once, before the grid has even decided how big this tile is.)
  lv_obj_t *row = lv_obj_create(tile);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
  lv_obj_set_style_pad_column(row, 3, 0);
  lv_obj_align(row, LV_ALIGN_CENTER, 0, 6);

  lv_obj_t *v = lv_label_create(row);
  lv_obj_add_style(v, &LB_Style::value, 0);
  lv_obj_set_style_text_font(v, display.height() < 200 ? &lv_font_montserrat_20 : &lv_font_montserrat_32, 0);
  lv_label_set_text(v, value);

  lv_obj_t *u = lv_label_create(row);
  lv_obj_add_style(u, &LB_Style::unit, 0);
  lv_obj_set_style_pad_bottom(u, 4, 0);
  lv_label_set_text(u, unit);
  return tile;
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  lv_obj_t *scr = lv_screen_active();
  const bool small = display.width() < 200 || display.height() < 200;
  const int32_t gap = small ? 4 : 8;

  // The screen is a column: a header, then a body that takes the rest.
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(scr, gap, 0);
  lv_obj_set_style_pad_row(scr, gap, 0);

  // ── Header: a Flex row ──────────────────────────────────────────────────
  // Title on the left, icons on the right. The spacer between them has
  // flex_grow 1, so it swallows the free space and pushes the icons over.
  lv_obj_t *header = lv_obj_create(scr);
  lv_obj_remove_style_all(header);                 // a bare container
  lv_obj_set_size(header, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *title = lv_label_create(header);
  lv_label_set_text(title, "Greenhouse");
  lv_obj_set_style_text_font(title, small ? &lv_font_montserrat_14 : &lv_font_montserrat_20, 0);

  lv_obj_t *spacer = lv_obj_create(header);
  lv_obj_remove_style_all(spacer);
  lv_obj_set_height(spacer, 1);
  lv_obj_set_flex_grow(spacer, 1);

  lv_obj_t *icons = lv_label_create(header);
  lv_label_set_text(icons, LV_SYMBOL_WIFI " " LV_SYMBOL_BATTERY_FULL);
  lv_obj_set_style_text_color(icons, LB_Style::muted, 0);

  // ── Body: a Grid ────────────────────────────────────────────────────────
  // Two columns and two rows of equal share. LV_GRID_FR(1) means "one part
  // of what is left", so the tiles always fill the body exactly. On a strip
  // display (much wider than tall) it becomes four columns in one row.
  lv_obj_t *body = lv_obj_create(scr);
  lv_obj_remove_style_all(body);
  lv_obj_set_width(body, lv_pct(100));
  lv_obj_set_flex_grow(body, 1);                   // the rest of the column

  const bool strip = display.width() > 2 * display.height();
  static const int32_t cols2[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static const int32_t cols4[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static const int32_t rows2[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static const int32_t rows1[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(body, strip ? cols4 : cols2, strip ? rows1 : rows2);
  lv_obj_set_style_pad_gap(body, gap, 0);

  const char *names[4] = {"TEMP", "HUMIDITY", "SOIL", "LIGHT"};
  const char *values[4] = {"24.6", "58", "31", "820"};
  const char *units[4] = {"C", "%", "%", "lx"};
  for (int i = 0; i < 4; i++) {
    lv_obj_t *t = makeTile(body, names[i], values[i], units[i]);
    // Cell (column, row), each spanning 1, stretched to fill the cell.
    const int col = strip ? i : i % 2, row = strip ? 0 : i / 2;
    lv_obj_set_grid_cell(t, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
  }
}

void loop() {
  LB_LVGL.loop();
}
