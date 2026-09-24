// Host stand-in for LB_Display: a framebuffer in memory, the real panel table
// for sizes, and a scripted finger for touch. Only what LVGL sketches use.
#pragma once
#include <Arduino.h>
#include "LB_Panels.h"
#include "LB_Touch.h"

extern const LB_PanelDef *g_panelOverride;   // the renderer picks the panel
extern uint16_t *g_fb;
extern int16_t g_w, g_h;
extern bool g_down;
extern int16_t g_tx, g_ty;

class LB_SimTouch : public LB_Touch {
 public:
  bool begin() override { _nativeW = g_w; _nativeH = g_h; return true; }
 protected:
  uint8_t readRaw(LB_TouchPoint *p, uint8_t max) override {
    if (!g_down || !max) return 0;
    p[0] = {g_tx, g_ty, 30, 0};
    return 1;
  }
};

class LB_Display {
 public:
  explicit LB_Display(const LB_PanelDef *p) : _p(p) {}
  bool begin(bool = false) {
    if (g_panelOverride) _p = g_panelOverride;
    const bool rot = _p->rotation & 1;
    g_w = rot ? _p->height : _p->width;
    g_h = rot ? _p->width : _p->height;
    _rot = _p->rotation;
    g_fb = (uint16_t *)calloc((size_t)g_w * g_h, 2);
    if (_p->touch != LB_TOUCH_NONE) { _touch = new LB_SimTouch(); _touch->begin(); }
    return true;
  }
  bool begun() const { return g_fb != nullptr; }
  int16_t width() const { return g_w; }
  int16_t height() const { return g_h; }
  bool hasCanvas() const { return true; }
  uint16_t *framebuffer() const { return g_fb; }
  void flush() {}
  void pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *px) {
    for (int r = 0; r < h; r++)
      if (y + r >= 0 && y + r < g_h) memcpy(g_fb + (y + r) * g_w + x, px + r * w, w * 2);
  }
  LB_Touch *touch() const { return _touch; }
  void setRotation(uint8_t r) { _rot = r & 3; }
  uint8_t rotation() const { return _rot; }
  void backlight(uint8_t) {}
  void printInfo() const { fprintf(stderr, "[render] %s %dx%d\n", _p->id, g_w, g_h); }
  const LB_PanelDef *panelDef() const { return _p; }
 private:
  const LB_PanelDef *_p;
  LB_Touch *_touch = nullptr;
  uint8_t _rot = 0;
};
