/*
  Shim so that `#include <lvgl.h>` — what every LVGL tutorial on the internet
  tells you to write — resolves to the copy vendored inside THIS library.

  Without it, that line either fails to compile (no lvgl library installed) or
  drags in a second, separate copy of LVGL from Library Manager, which ends in
  several hundred lines of:

      multiple definition of `lv_group_init'

  DO NOT INSTALL THE "lvgl" LIBRARY FROM LIBRARY MANAGER. This library already
  contains LVGL (src/lvgl, version in src/lvgl/VENDORED_VERSION). Two copies
  cannot be linked into one sketch, and Arduino has no way to prefer one, so
  the only fix is to remove the separately-installed one:

      Library Manager -> lvgl -> Remove

  Including <LonelyBinaryLVGL.h> already gives you all of LVGL, so you never
  need this line at all — it exists only so that copied tutorial code works.
*/
#ifndef LB_LVGL_SHIM_H
#define LB_LVGL_SHIM_H
#include "lvgl/lvgl.h"
#endif
