// Host stand-in for the parts of Arduino the LVGL examples touch. Time is
// virtual: millis() is whatever the renderer says it is, and delay() moves it.
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <algorithm>

extern uint32_t g_now;
inline uint32_t millis() { return g_now; }
inline uint32_t micros() { return g_now * 1000; }
inline void delay(uint32_t ms) { g_now += ms; }
inline void yield() {}

#define F(s) (s)
#define PROGMEM
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
inline void pinMode(int, int) {}
inline int digitalRead(int) { return HIGH; }
inline void digitalWrite(int, int) {}
inline int analogRead(int) { return 2048; }

inline long random(long hi) { return hi > 0 ? rand() % hi : 0; }
inline long random(long lo, long hi) { return hi > lo ? lo + rand() % (hi - lo) : lo; }
inline void randomSeed(unsigned long s) { srand(s); }
inline long map(long x, long a, long b, long c, long d) { return (x - a) * (d - c) / (b - a) + c; }
template <class T, class L, class H> inline T constrain(T x, L lo, H hi) { return x < lo ? lo : x > hi ? hi : x; }
#ifndef PI
#define PI 3.14159265358979
#endif

class Print {
 public:
  virtual ~Print() {}
  void print(const char *s) { fputs(s, stderr); }
  void print(int v) { fprintf(stderr, "%d", v); }
  void println(const char *s = "") { fprintf(stderr, "%s\n", s); }
  void println(int v) { fprintf(stderr, "%d\n", v); }
  int printf(const char *fmt, ...) {
    va_list a; va_start(a, fmt); int n = vfprintf(stderr, fmt, a); va_end(a); return n;
  }
};
class HardwareSerial : public Print {
 public:
  void begin(unsigned long) {}
  operator bool() const { return true; }
};
extern HardwareSerial Serial;

struct EspClass {
  uint32_t getFreeHeap() { return 180000; }
  uint32_t getMinFreeHeap() { return 150000; }
  uint32_t getHeapSize() { return 320000; }
  uint32_t getFreePsram() { return 0; }
  uint32_t getPsramSize() { return 0; }
  uint32_t getCpuFreqMHz() { return 240; }
  const char *getChipModel() { return "ESP32 (host render)"; }
  void restart() {}
};
extern EspClass ESP;

#define MALLOC_CAP_SPIRAM 1
#define MALLOC_CAP_8BIT 2
#define MALLOC_CAP_INTERNAL 4
inline void *heap_caps_malloc(size_t n, uint32_t caps) { return (caps & MALLOC_CAP_SPIRAM) ? nullptr : malloc(n); }

// Just enough of Arduino's String for WiFi.SSID(i).c_str() and Preferences.
#include <string>
class String {
 public:
  String(const char *s = "") : _s(s ? s : "") {}
  String(const std::string &s) : _s(s) {}
  const char *c_str() const { return _s.c_str(); }
  size_t length() const { return _s.size(); }
  bool isEmpty() const { return _s.empty(); }
  bool operator==(const String &o) const { return _s == o._s; }
  bool operator==(const char *o) const { return _s == o; }
 private:
  std::string _s;
};
