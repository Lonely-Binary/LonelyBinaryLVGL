/*
 * render — run an LVGL example sketch on the host and write PNG screenshots.
 *
 *   render <panel id> <scenario file | -> <out prefix>
 *
 * The sketch is compiled in with -DSKETCH="path/to/Example.ino". Its setup()
 * runs once, then loop() runs every 5 ms of virtual time while the scenario
 * plays. Scenario lines:
 *
 *   wait <ms>              let the sketch run
 *   shot <name>            write <out prefix>_<name>.png
 *   tap <x> <y>            press for 100 ms, release, wait 300 ms
 *   press <x> <y>          finger down
 *   drag <x> <y> <ms>      move the finger there in a straight line
 *   release                finger up
 *
 * With no scenario ("-") it is: wait 1500, shot main.
 */
#include <Arduino.h>
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>
#include <zlib.h>
#include <string>
#include <vector>

uint32_t g_now = 0;
HardwareSerial Serial;
EspClass ESP;
const LB_PanelDef *g_panelOverride = nullptr;
uint16_t *g_fb = nullptr;
int16_t g_w = 0, g_h = 0;
bool g_down = false;
int16_t g_tx = 0, g_ty = 0;

#include SKETCH

static void run(uint32_t ms) {
  const uint32_t end = g_now + ms;
  while (g_now < end) { loop(); g_now += 5; }
}

static void put32(std::vector<uint8_t> &v, uint32_t x) {
  for (int i = 3; i >= 0; i--) v.push_back(x >> (i * 8));
}
static void chunk(FILE *f, const char *type, const std::vector<uint8_t> &d) {
  std::vector<uint8_t> b;
  put32(b, d.size());
  b.insert(b.end(), type, type + 4);
  b.insert(b.end(), d.begin(), d.end());
  put32(b, crc32(0, b.data() + 4, b.size() - 4));
  fwrite(b.data(), 1, b.size(), f);
}
static void png(const std::string &path) {
  // Let LVGL finish any pending redraw first.
  lv_refr_now(nullptr);
  std::vector<uint8_t> raw;
  for (int y = 0; y < g_h; y++) {
    raw.push_back(0);
    for (int x = 0; x < g_w; x++) {
      const uint16_t c = g_fb[y * g_w + x];
      raw.push_back(((c >> 11) & 31) * 255 / 31);
      raw.push_back(((c >> 5) & 63) * 255 / 63);
      raw.push_back((c & 31) * 255 / 31);
    }
  }
  uLongf n = compressBound(raw.size());
  std::vector<uint8_t> z(n);
  compress2(z.data(), &n, raw.data(), raw.size(), 9);
  z.resize(n);
  FILE *f = fopen(path.c_str(), "wb");
  if (!f) { perror(path.c_str()); exit(1); }
  static const uint8_t sig[8] = {0x89, 'P', 'N', 'G', 13, 10, 26, 10};
  fwrite(sig, 1, 8, f);
  std::vector<uint8_t> ihdr;
  put32(ihdr, g_w); put32(ihdr, g_h);
  ihdr.insert(ihdr.end(), {8, 2, 0, 0, 0});
  chunk(f, "IHDR", ihdr);
  chunk(f, "IDAT", z);
  chunk(f, "IEND", {});
  fclose(f);
  fprintf(stderr, "[render] wrote %s\n", path.c_str());
}

int main(int argc, char **argv) {
  if (argc != 4) { fprintf(stderr, "usage: render <panel id> <scenario|-> <out prefix>\n"); return 2; }
  for (int i = 0; i < LB_PANEL_COUNT; i++)
    if (!strcmp(LB_PANELS[i].id, argv[1])) g_panelOverride = &LB_PANELS[i];
  if (!g_panelOverride) { fprintf(stderr, "unknown panel %s\n", argv[1]); return 2; }
  const std::string out = argv[3];

  setup();

  std::vector<std::string> lines;
  if (strcmp(argv[2], "-")) {
    FILE *f = fopen(argv[2], "r");
    if (!f) { perror(argv[2]); return 1; }
    char buf[256];
    while (fgets(buf, sizeof buf, f)) lines.push_back(buf);
    fclose(f);
  } else {
    lines = {"wait 1500", "shot main"};
  }
  for (auto &l : lines) {
    char cmd[32] = {0}, name[128] = {0};
    int a = 0, b = 0, c = 0;
    if (sscanf(l.c_str(), "%31s", cmd) != 1 || cmd[0] == '#') continue;
    if (!strcmp(cmd, "wait") && sscanf(l.c_str(), "%*s %d", &a) == 1) run(a);
    else if (!strcmp(cmd, "shot") && sscanf(l.c_str(), "%*s %127s", name) == 1) png(out + "_" + name + ".png");
    else if (!strcmp(cmd, "tap") && sscanf(l.c_str(), "%*s %d %d", &a, &b) == 2) {
      g_tx = a; g_ty = b; g_down = true; run(100); g_down = false; run(300);
    } else if (!strcmp(cmd, "press") && sscanf(l.c_str(), "%*s %d %d", &a, &b) == 2) {
      g_tx = a; g_ty = b; g_down = true; run(50);
    } else if (!strcmp(cmd, "drag") && sscanf(l.c_str(), "%*s %d %d %d", &a, &b, &c) == 3) {
      const int x0 = g_tx, y0 = g_ty, steps = c / 5 > 0 ? c / 5 : 1;
      for (int i = 1; i <= steps; i++) { g_tx = x0 + (a - x0) * i / steps; g_ty = y0 + (b - y0) * i / steps; run(5); }
    } else if (!strcmp(cmd, "release")) { g_down = false; run(50); }
    else { fprintf(stderr, "bad scenario line: %s", l.c_str()); return 1; }
  }
  return 0;
}
