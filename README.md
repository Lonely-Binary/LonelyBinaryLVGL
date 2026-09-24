# Lonely Binary LVGL

LVGL on a Lonely Binary display, with **no `lv_conf.h` to copy** and nothing to
configure.

```cpp
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_TFT_24);      // ← the only line that changes per panel

void setup() {
  display.begin(true);              // true = PSRAM framebuffer
  LB_LVGL.begin(display);           // flush cb, buffers, tick, theme — done

  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_obj_add_style(label, &LB_Style::value, 0);
  lv_label_set_text(label, "24.6");
  lv_obj_center(label);
}

void loop() {
  LB_LVGL.loop();                   // call this often
}
```

That's the whole setup. No template to rename, no flush callback to write, no
draw buffers to size.

---

## Why this exists

Getting LVGL running on Arduino normally means copying `lv_conf_template.h`
into your `libraries` folder, renaming it, flipping `#if 0` to `#if 1`, and
then writing a flush callback and buffer allocation by hand. That first step is
the same trap as TFT_eSPI's `User_Setup.h`, and it is where most people give up.

A known-good `lv_conf.h` ships **inside** this library, and LVGL finds it
because Arduino puts the library's `src/` on the include path. You install one
library and edit nothing.

---

## Install

Library Manager → **Lonely Binary LVGL**. It pulls in *Lonely Binary Display*
and *Lonely Binary GFX*.

Then **File ▸ Examples ▸ Lonely Binary LVGL ▸ 00_Hello**, and work down the
list — see [Examples](#examples).

> On a board with PSRAM, set **Tools ▸ PSRAM ▸ Enabled**. `display.begin(true)`
> then gives LVGL the panel's own framebuffer and rendering becomes
> **zero-copy** — the flush callback copies nothing at all.
> `LB_LVGL.usingFramebuffer()` tells you whether you got it.

### Flash

LVGL is not small. Check what you have room for:

| Sketch | ESP32-S3 | classic ESP32 |
|---|---:|---:|
| One label, default font | 690 KB (52%) | — |
| `00_Hello` (arc + 6 font sizes) | 912 KB (69%) | 895 KB (68%) |

Percentages are of the **default** 1.31 MB app partition. Roughly 690 KB is
the LVGL floor; everything above that is the fonts and widgets *you* reference
— only those get linked, so a sketch that sticks to one font size stays near
the floor.

On a 16 MB board none of this matters. On a 4 MB classic ESP32 it does: LVGL
plus WiFi plus MQTT will not fit in 1.31 MB, so pick **Tools ▸ Partition
Scheme ▸ Minimal SPIFFS** (or a custom scheme with a bigger app partition)
before you add networking.

---

## Do not also install the `lvgl` library

This library **contains** LVGL. Installing the separate `lvgl` library from
Library Manager gives you two copies, and no build system can link both.

`src/lvgl.h` is a shim so that `#include <lvgl.h>` — what every LVGL tutorial
tells you to write — resolves to the vendored copy. Tested with the official
`lvgl` 9.5.0 also installed:

| Your sketch | `lvgl` also installed | Result |
|---|:--:|---|
| `<LonelyBinaryLVGL.h>` only | no | works |
| `<LonelyBinaryLVGL.h>` only | **yes** | works — `lvgl` is not linked in |
| ours, then `#include <lvgl.h>` | no | works |
| ours, then `#include <lvgl.h>` | **yes** | works — `lvgl` is not linked in |
| `#include <lvgl.h>` **first** | no | works |
| `#include <lvgl.h>` **first** | **yes** | **fails** ⚠️ |

The one broken case is the last: with `<lvgl.h>` on the very first line,
Arduino resolves it to the separately-installed `lvgl` before this library is
in play, and that copy has no config of its own:

```
fatal error: ../../lv_conf.h: No such file or directory
```

That is LVGL's standard "you never copied lv_conf.h" error. The fix is not to
copy one — it is to remove the library you do not need: **Library Manager →
lvgl → Remove**. Or simply put `#include <LonelyBinaryLVGL.h>` above it.

---

## `LB_Style` — the house look

LVGL's stock theme is gradients, shadows and rounded corners. On a 240 px panel
read from a metre away that turns to mush. `LB_Style` is the opposite: flat
colour, large numerals, high contrast — defined **once**, so a screen full of
tiles cannot drift.

```cpp
lv_obj_add_style(caption, &LB_Style::label, 0);   // small, muted, spaced
lv_obj_add_style(number,  &LB_Style::value, 0);   // the big numeral
lv_obj_add_style(tile,    &LB_Style::card,  0);   // flat surface, hairline border
```

Colour tokens are semantic, never literal — `LB_Style::accent`,
`LB_Style::muted`, `LB_Style::ok` / `warn` / `crit`. Redefine the tokens and
every widget follows, which is how the monochrome e-paper palette works:

```cpp
LB_Style::useMono();                              // black on white
LB_Style::setLevel(number, LB_Style::LEVEL_WARN); // consistent state colours
lv_obj_set_style_text_font(number, LB_Style::valueFontFor(tileHeight), 0);
```

The same colours are available to the plain drawing API, as Lonely Binary GFX
colours, so a screen drawn without LVGL matches one drawn with it:

```cpp
display.fillScreen(LB_Style::palette::bg);
display.setTextColor(LB_Style::palette::accent);
```

`LB_Style::palette` is where the colours are defined; the LVGL tokens are
derived from it. `LB_Style::toLv()` converts any `lb_color_t` for LVGL.

Animations are off in the shared styles by design — they cost redraws, and on
e-paper they are actively harmful. Opt in locally where you want one.

---

## Touch

On a touch panel — the `_ctp` / `_rtp` constants, such as `LB_SQUARE_392_CTP` —
`LB_LVGL.begin()` registers the touch controller as LVGL's pointer input. There
is no input code to write: buttons click, sliders drag, lists scroll.

```cpp
LB_Display display(LB_SQUARE_392_CTP);   // 3.92" square, GT911 capacitive touch

void setup() {
  display.begin(true);
  LB_LVGL.begin(display);                // touch included
  lv_obj_t *b = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(b, onTap, LV_EVENT_CLICKED, nullptr);
}
```

Rotation is handled too: `display.setRotation()` turns the touch coordinates
with the picture. `LB_LVGL.touch()` returns the `lv_indev_t *`, or `nullptr`
on a panel without touch — a sketch can check it and say so.

## The stock widgets match

`LB_LVGL.begin()` starts LVGL's default theme in **dark** mode with the house
accent as its primary colour. So a plain `lv_button`, `lv_slider` or
`lv_switch` already sits right on the `LB_Style` background; you only style
what you want to be different.

---

## Examples

One idea per example, in order. Each file starts with what it teaches.

| | Example | What it shows | Touch |
|---|---|---|:--:|
| **Start** | `00_Hello` | The four lines every sketch starts with; a live gauge | |
| | `01_Label` | Fonts, wrapping, dots, marquee, icons, updating text from a timer | |
| | `02_Styles` | The stock theme, `LB_Style`, your own `lv_style_t`, local styles, pressed state | optional |
| | `03_Layout` | Align, Flex, Grid and percentages — one sketch for every screen shape | |
| **Show** | `10_Bar` | Progress, a vertical tank, a range bar, colour by level | |
| | `11_Arc` | A ring, a 270° dial, and one you can drag | optional |
| | `12_Scale` | An instrument dial with ticks, a red zone and a needle; a ruler | |
| | `13_Chart` | A scrolling two-line chart with a legend and an axis; a bar chart | |
| | `14_Table` | Rows and columns, a coloured header, zebra rows, updating one cell | optional |
| | `15_LED_Spinner` | Status lights, a busy spinner, showing and hiding | |
| | `16_Image` | A picture from a C array, scaled, rotated, tinted — and how to make your own | |
| | `17_QRCode` | A QR a phone can scan, including the join-this-Wi-Fi format | |
| | `18_Animation` | `lv_anim`: easing curves, there-and-back, forever, fading | |
| **Touch** | `20_Button` | **Events** — tap, hold, toggle, one callback for many buttons | ✔ |
| | `21_Switch_Checkbox` | Switches, checkboxes, radio buttons, disabling a control | ✔ |
| | `22_Slider` | A slider that sets the real backlight; range and vertical sliders | ✔ |
| | `23_Roller_Dropdown` | A time picker from two rollers; a dropdown | ✔ |
| | `24_Keyboard` | Text fields, password dots, the on-screen keyboard, a number pad | ✔ |
| | `25_Spinbox` | An exact value with + and −, repeating while held | ✔ |
| | `26_ButtonMatrix` | A PIN pad as one object | ✔ |
| | `27_Calendar` | Pick a date; highlighted days | ✔ |
| | `28_MsgBox` | "Are you sure?" dialogs and self-removing notices | ✔ |
| **Navigate** | `30_TabView` | Pages behind a tab bar; swipe or tap | ✔ |
| | `31_TileView` | Swipe between full-screen pages, with page dots | ✔ |
| | `32_List_Menu` | A settings menu with sub-pages and a back button | ✔ |
| | `33_Screens` | Whole screens, slide transitions, swipe to go back, a fixed status bar | ✔ |
| **Build** | `40_Thermostat` | Drag-to-set dial, mode selector, and real hysteresis control logic | optional |
| | `41_Dashboard` | Live tiles with sparklines, thresholds, an alert banner, tap for stats | optional |
| | `42_Settings` | Wi-Fi scan and join, brightness, screen timeout, saved settings, device info | ✔ |

"Touch ✔" examples still compile and draw on a panel without touch; they say
on Serial that they need one.

Every example lays itself out from `display.width()` / `display.height()`, so
changing the panel constant is the only edit. They are written for 240 px and
up; the smallest panels run them, but cramped.

---

## API

| | |
|---|---|
| `LB_LVGL.begin(display, partialLines = 40)` | Bring up LVGL against an already-begun `LB_Display`. |
| `LB_LVGL.loop()` | Run LVGL's timers and redraw. Call every pass of `loop()`. |
| `LB_LVGL.usingFramebuffer()` | `true` when rendering zero-copy into the panel framebuffer. |
| `LB_LVGL.display()` | The `lv_display_t *`, if you need it. |
| `LB_LVGL.touch()` | The touch `lv_indev_t *`, or `nullptr` when the panel has none. |

Without a canvas, LVGL falls back to a partial buffer of `partialLines` rows
and pushes each band over SPI. It works; it is just not free.

**How much does that cost?** Measured on a classic ESP32 (no PSRAM) driving a
240 × 320 panel at 40 MHz. A framebuffer would have wanted 150 KB, which does
not fit, so `begin(true)` said so and fell back on its own:

```
[LB_Display] no room for a 240 x 320 framebuffer (153600 bytes)
             — drawing direct instead. Enable Tools > PSRAM if your board has it.
zero-copy framebuffer: NO — partial rendering
heap after LVGL init: 300896
```

A full dashboard — anti-aliased arc, 60-point scrolling chart, two bars and a
header, all updating four times a second — ran smoothly on that, with the heap
flat at 291 KB. So partial rendering is not a consolation prize at this size;
you would reach for PSRAM for a bigger panel or a heavier screen, not for this
one.

---

## The vendored LVGL

LVGL lives under [`src/lvgl`](src/lvgl) — MIT licensed, unmodified apart from
deleting sources that cannot apply here (SDL/X11/Wayland/Windows back-ends,
desktop image decoders). Headers are kept, because `lvgl.h` includes them
unconditionally and they self-guard on `LV_USE_*`.

The pinned release is in [`src/lvgl/VENDORED_VERSION`](src/lvgl/VENDORED_VERSION).

**Never hand-edit anything under `src/lvgl`** — it is regenerated wholesale:

```bash
./tools/vendor_lvgl.sh v9.5.0
```

CI re-runs that and fails if the tree differs, so a local patch cannot survive
unnoticed.

Our config is [`src/lv_conf.h`](src/lv_conf.h), and it deliberately only
*overrides* what we care about — everything else falls through to LVGL's own
defaults, so an LVGL bump does not leave a stale 1500-line copy behind.

> LVGL 9.5 removed the XML parser and the bundled expat from core
> ([lvgl/lvgl#9565](https://github.com/lvgl/lvgl/pull/9565)); the runtime XML
> loader is a commercial LVGL Pro feature. Nothing here uses it, and the core
> we vendor is cleanly MIT.

---

## Seeing an example without the hardware

`tools/render` runs an example on your computer and writes PNG screenshots, at
the real size of any panel in the table, and can script taps and drags:

```bash
tools/render/render.py 20_Button                 # every example that starts with 20
tools/render/render.py 03 -p square_392_ctp      # one panel
tools/render/sheet.py 03_Layout                  # all panels side by side
```

It builds LVGL natively with `clang` and swaps `LB_Display` for an in-memory
framebuffer (`tools/render/shim`), so layout, text, colours and what a tap does
are exactly what the device draws. Speed, and anything the real bus or touch
chip does, still needs the bench. Tap scripts live in
`tools/render/scenarios/<Example>.txt` — see the top of `render.cpp`.

---

## License

MIT. LVGL is MIT (see `src/lvgl/LICENCE.txt`). Brought to you by Lonely Binary
— thank you for supporting our open-source work!
