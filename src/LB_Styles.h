/*
  LB_Styles — the Lonely Binary look, defined once.

  LVGL's stock theme is gradients, shadows and rounded corners. On a 240px
  panel viewed from a metre away that reads as mush. The house style is the
  opposite: flat colour, large numerals, high contrast — and it has to live in
  ONE place, because a screen where every tile styles itself is a screen that
  drifts.

  Use the tokens, not literal colours:

      lv_obj_t *card = lv_obj_create(lv_screen_active());
      lv_obj_add_style(card, &LB_Style::card, 0);

      lv_obj_t *v = lv_label_create(card);
      lv_obj_add_style(v, &LB_Style::value, 0);
      lv_label_set_text(v, "24.6");

  Everything here is initialised by LB_LVGL.begin(); you never call begin()
  on this yourself.
*/
#ifndef LB_STYLES_H
#define LB_STYLES_H

#include <LonelyBinaryGFX.h>
#include "lvgl/lvgl.h"

namespace LB_Style {

// ── The house palette ────────────────────────────────────────────────────────
// The single source of the colours below, as Lonely Binary GFX colours, so a
// screen drawn with the canvas API matches an LVGL one exactly:
//
//     display.fillScreen(LB_Style::palette::bg);
//     display.setTextColor(LB_Style::palette::accent);
//
// These are subtle shades on purpose, which is why they live here and not in
// LB_GFX's own short list (LB_RED, ...): that list has to survive being
// quantised to two e-paper inks, and a dark slate next to a darker slate does
// not. On e-paper, useMono() is the answer, not these.
namespace palette {
constexpr lb_color_t bg      = LB_RGB(0x0C, 0x0F, 0x14);
constexpr lb_color_t surface = LB_RGB(0x14, 0x19, 0x23);
constexpr lb_color_t text    = LB_RGB(0xE6, 0xEA, 0xF2);
constexpr lb_color_t muted   = LB_RGB(0x8A, 0x93, 0xA6);
constexpr lb_color_t accent  = LB_RGB(0x2D, 0xD4, 0xBF);
constexpr lb_color_t ok      = LB_RGB(0x34, 0xD2, 0x7B);
constexpr lb_color_t warn    = LB_RGB(0xF6, 0xB7, 0x3C);
constexpr lb_color_t crit    = LB_RGB(0xF2, 0x70, 0x5B);
}  // namespace palette

/** An LB_GFX colour as an LVGL one. Both are 0xRRGGBB underneath. A palette
 *  index means nothing to LVGL, so it comes out magenta, as it does on a
 *  direct-colour canvas - visibly wrong rather than subtly wrong. */
lv_color_t toLv(lb_color_t c);

// ── Colour tokens ────────────────────────────────────────────────────────────
// Semantic, not literal. A mono e-paper build redefines these and every
// widget follows; that is why nothing below hard-codes a colour. They start
// as the palette above; useMono() replaces them.
extern lv_color_t bg;       // page background
extern lv_color_t surface;  // a tile sitting on the page
extern lv_color_t text;     // primary text and numerals
extern lv_color_t muted;    // labels, units, axis marks
extern lv_color_t accent;   // the one colour that means "this is the value"
extern lv_color_t ok;
extern lv_color_t warn;
extern lv_color_t crit;

// ── Styles ───────────────────────────────────────────────────────────────────
extern lv_style_t card;   // a tile: flat surface, hairline border, no shadow
extern lv_style_t label;  // small uppercase caption above a value
extern lv_style_t value;  // the big number — the reason the screen exists
extern lv_style_t unit;   // the suffix after the number

/** Pick the numeral font that fits a tile this tall. */
const lv_font_t *valueFontFor(lv_coord_t tileHeight);

/** Recolour a widget by state. Threshold logic stays with the caller; this is
 *  only the mapping from state to colour, so it is consistent everywhere. */
enum Level { LEVEL_OK, LEVEL_WARN, LEVEL_CRIT };
void setLevel(lv_obj_t *obj, Level level);

/** Called by LB_LVGL.begin(). Safe to call twice. */
void begin();

/** Swap to the high-contrast monochrome palette (e-paper, or a deliberately
 *  stark look). Call after LB_LVGL.begin(). */
void useMono(bool blackOnWhite = true);

}  // namespace LB_Style

#endif  // LB_STYLES_H
