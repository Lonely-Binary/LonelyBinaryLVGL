#!/usr/bin/env bash
#
# Vendor LVGL into src/lvgl/.
#
# WHY VENDOR AT ALL: LVGL needs an lv_conf.h on the include path, and the
# sanctioned way to provide one in Arduino is to copy lv_conf_template.h into
# the libraries/ folder and rename it. That is the same class of problem as
# TFT_eSPI's User_Setup.h, and it is the single largest support burden for a
# teaching audience. Shipping a known-good copy means the customer installs one
# library and edits nothing.
#
# Re-run this to move to a new LVGL release. It is destructive and idempotent:
# src/lvgl/ is deleted and rebuilt, so never hand-edit anything inside it.
# Our own lv_conf.h lives at src/lv_conf.h and is not touched.
#
#     ./tools/vendor_lvgl.sh            # use LVGL_VERSION below
#     ./tools/vendor_lvgl.sh v9.5.1     # or pin explicitly
#
set -euo pipefail

LVGL_VERSION="${1:-v9.5.0}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="$ROOT/src/lvgl"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo "==> cloning lvgl $LVGL_VERSION"
git clone -q --depth 1 --branch "$LVGL_VERSION" https://github.com/lvgl/lvgl.git "$TMP/lvgl"

rm -rf "$DEST"
mkdir -p "$DEST"
cp -R "$TMP/lvgl/src" "$DEST/"
cp "$TMP/lvgl/lvgl.h" "$TMP/lvgl/lvgl_private.h" "$TMP/lvgl/lv_version.h" "$DEST/"
cp "$TMP/lvgl/LICENCE.txt" "$DEST/"
cp "$TMP/lvgl/lv_conf_template.h" "$DEST/"   # reference only; ours is src/lv_conf.h

# ── Trim ─────────────────────────────────────────────────────────────────────
# Arduino compiles every .c under a library's src/ tree, whether or not the
# feature is enabled in lv_conf.h. Dropping what we can never use is therefore
# a real first-build-time saving, not just disk.

# NOTE: lvgl.h includes the libs and drivers headers UNCONDITIONALLY (they
# self-guard on LV_USE_*), so the headers must stay. Only the implementation
# files go — which is where the build time is anyway.

# Platform back-ends: we drive the panel through Arduino_GFX's flush callback,
# so LVGL's own SDL / X11 / Wayland / Windows / NuttX / UEFI / OpenGL / evdev
# drivers can never run here.
find "$DEST/src/drivers" \( -name "*.c" -o -name "*.cpp" \) -delete

# Optional integrations. Kept whole: qrcode (device provisioning screens),
# fsdrv (images off LittleFS/SD), bin_decoder + rle + lz4 (LVGL's own binary
# image formats). The rest need a desktop-class dependency or a decoder we do
# not ship, so their sources go and their headers stay.
for drop in thorvg gltf vg_lite_driver freetype lodepng tiny_ttf \
            FT800-FT813 svg nanovg gif tjpgd barcode ffmpeg frogfs \
            gstreamer libjpeg_turbo rlottie libpng libwebp bmp; do
  [ -d "$DEST/src/libs/$drop" ] &&
    find "$DEST/src/libs/$drop" \( -name "*.c" -o -name "*.cpp" \) -delete
done

# Fonts are kept in full, CJK included: this library serves every customer who
# bought a screen, not just the English-language ones, and a disabled font
# compiles to nothing. Only the odd-numbered large Montserrat steps go — nobody
# needs 34 AND 36 AND 38.
for px in 34 36 38 42 44 46; do
  rm -f "$DEST/src/font/lv_font_montserrat_${px}.c"
done

# Arduino will not descend into a folder named after a build system, and these
# would only confuse the library scanner.
find "$DEST" -name CMakeLists.txt -delete
find "$DEST" -name "*.mk" -delete
find "$DEST" -name Kconfig -delete

echo "$LVGL_VERSION" > "$DEST/VENDORED_VERSION"

echo "==> vendored $LVGL_VERSION"
echo "    $(find "$DEST" -name '*.c' | wc -l | tr -d ' ') .c files, $(du -sh "$DEST" | cut -f1)"
