# Third-party code

## LVGL — `src/lvgl/`

LVGL v9.5.0, vendored. MIT licensed, unmodified apart from removing the desktop
back-ends this library cannot use. Its own licence text ships alongside it at
[`src/lvgl/LICENCE.txt`](src/lvgl/LICENCE.txt); the vendored version is recorded
in `src/lvgl/VENDORED_VERSION` and re-vendoring is done with
[`tools/vendor_lvgl.sh`](tools/vendor_lvgl.sh).

MIT imposes no obligation beyond retaining that notice, so LVGL can be shipped
inside a closed product without further conditions.

## Why vendored rather than depended on

LVGL normally makes you copy `lv_conf_template.h` into your libraries folder,
rename it, and flip `#if 0` to `#if 1` before anything compiles — the same trap
as `TFT_eSPI`'s `User_Setup.h`, and where most people give up. Vendoring lets a
known-good `lv_conf.h` ship inside `src/`, where Arduino puts it on the include
path automatically, so there is nothing to configure.
