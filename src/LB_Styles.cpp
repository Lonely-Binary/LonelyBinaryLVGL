#include "LB_Styles.h"

namespace LB_Style {

lv_color_t bg, surface, text, muted, accent, ok, warn, crit;
lv_style_t card, label, value, unit;

static bool s_ready = false;

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

  bg      = lv_color_hex(0x0C0F14);
  surface = lv_color_hex(0x141923);
  text    = lv_color_hex(0xE6EAF2);
  muted   = lv_color_hex(0x8A93A6);
  accent  = lv_color_hex(0x2DD4BF);
  ok      = lv_color_hex(0x34D27B);
  warn    = lv_color_hex(0xF6B73C);
  crit    = lv_color_hex(0xF2705B);

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
  const uint32_t fg = blackOnWhite ? 0x000000 : 0xFFFFFF;
  const uint32_t bk = blackOnWhite ? 0xFFFFFF : 0x000000;
  bg = surface = lv_color_hex(bk);
  text = accent = ok = warn = crit = lv_color_hex(fg);
  muted = lv_color_hex(fg);
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
