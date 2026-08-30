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
and *GFX Library for Arduino*.

Then **File ▸ Examples ▸ Lonely Binary LVGL ▸ HelloLVGL**.

> On a board with PSRAM, set **Tools ▸ PSRAM ▸ Enabled**. `display.begin(true)`
> then gives LVGL the panel's own framebuffer and rendering becomes
> **zero-copy** — the flush callback copies nothing at all.
> `LB_LVGL.usingFramebuffer()` tells you whether you got it.

### Flash

LVGL is not small. Check what you have room for:

| Sketch | ESP32-S3 | classic ESP32 |
|---|---:|---:|
| One label, default font | 690 KB (52%) | — |
| `HelloLVGL` (arc + 6 font sizes) | 912 KB (69%) | 895 KB (68%) |

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

Animations are off in the shared styles by design — they cost redraws, and on
e-paper they are actively harmful. Opt in locally where you want one.

---

## API

| | |
|---|---|
| `LB_LVGL.begin(display, partialLines = 40)` | Bring up LVGL against an already-begun `LB_Display`. |
| `LB_LVGL.loop()` | Run LVGL's timers and redraw. Call every pass of `loop()`. |
| `LB_LVGL.usingFramebuffer()` | `true` when rendering zero-copy into the panel framebuffer. |
| `LB_LVGL.display()` | The `lv_display_t *`, if you need it. |

Without a canvas, LVGL falls back to a partial buffer of `partialLines` rows
and pushes each band over SPI. It works; it is just not free.

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

## License

MIT. LVGL is MIT (see `src/lvgl/LICENCE.txt`). Brought to you by Lonely Binary
— thank you for supporting our open-source work!
