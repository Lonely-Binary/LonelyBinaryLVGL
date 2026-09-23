#include "LB_Styles.h"

namespace LB_Style {

lv_color_t bg, surface, text, muted, accent, ok, warn, crit;
lv_style_t card, label, value, unit;

static bool s_ready = false;

lv_color_t toLv(lb_color_t c) {
  if (lb_color_is_index(c)) c = LB_MAGENTA;
  return lv_color_hex(c & 0xFFFFFF);
}

static void applyTokens() {
  lv_style_set_bg_color(&card, surface);
  lv_style_set_border_color(&card, muted);
  lv_style_set_text_color(&label, muted);
  lv_style_set_text_color(&value, text);
  lv_style_set_text_color(&unit, muted);
}

void begin() {
  if (s_ready) {
    applyTokens();
    return;
  }
  s_ready = true;

  bg      = toLv(palette::bg);
  surface = toLv(palette::surface);
  text    = toLv(palette::text);
  muted   = toLv(palette::muted);
  accent  = toLv(palette::accent);
  ok      = toLv(palette::ok);
  warn    = toLv(palette::warn);
  crit    = toLv(palette::crit);

  // Card — flat. No gradient, no shadow, minimal radius. The border is a
  // hairline only so tiles read as separate without drawing attention.
  lv_style_init(&card);
  lv_style_set_bg_opa(&card, LV_OPA_COVER);
  lv_style_set_border_width(&card, 1);
  lv_style_set_border_opa(&card, LV_OPA_30);
  lv_style_set_radius(&card, 4);
  lv_style_set_pad_all(&card, 6);
  lv_style_set_shadow_width(&card, 0);
  // Animations are off by design: they cost redraws, and on e-paper they are
  // actively harmful. A tile that needs to animate can opt in locally.
  lv_style_set_anim_duration(&card, 0);

  // Caption above the value. Small, muted, spaced — uppercase is applied by
  // the caller because LVGL has no text-transform.
  lv_style_init(&label);
  lv_style_set_text_font(&label, &lv_font_montserrat_12);
  lv_style_set_text_letter_space(&label, 1);

  // The number. Large by default; use valueFontFor() to fit a given tile.
  lv_style_init(&value);
  lv_style_set_text_font(&value, &lv_font_montserrat_32);

  lv_style_init(&unit);
  lv_style_set_text_font(&unit, &lv_font_montserrat_14);

  applyTokens();

  lv_obj_set_style_bg_color(lv_screen_active(), bg, 0);
  lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, 0);
}

void useMono(bool blackOnWhite) {
  const lv_color_t fg = toLv(blackOnWhite ? LB_BLACK : LB_WHITE);
  const lv_color_t bk = toLv(blackOnWhite ? LB_WHITE : LB_BLACK);
  bg = surface = bk;
  text = accent = ok = warn = crit = muted = fg;
  applyTokens();
  lv_obj_set_style_bg_color(lv_screen_active(), bg, 0);
}

const lv_font_t *valueFontFor(lv_coord_t tileHeight) {
  if (tileHeight >= 140) return &lv_font_montserrat_48;
  if (tileHeight >= 100) return &lv_font_montserrat_40;
  if (tileHeight >= 70)  return &lv_font_montserrat_32;
  if (tileHeight >= 50)  return &lv_font_montserrat_24;
  return &lv_font_montserrat_16;
}

void setLevel(lv_obj_t *obj, Level level) {
  lv_color_t c = (level == LEVEL_CRIT) ? crit
               : (level == LEVEL_WARN) ? warn
                                       : text;
  lv_obj_set_style_text_color(obj, c, 0);
}

}  // namespace LB_Style
