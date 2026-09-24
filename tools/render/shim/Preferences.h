// Host stand-in: forgets everything, returns the defaults.
#pragma once
#include <Arduino.h>
class Preferences {
 public:
  bool begin(const char *, bool = false) { return true; }
  void end() {}
  uint8_t getUChar(const char *, uint8_t d = 0) { return d; }
  size_t putUChar(const char *, uint8_t) { return 1; }
  int32_t getInt(const char *, int32_t d = 0) { return d; }
  size_t putInt(const char *, int32_t) { return 4; }
  bool getBool(const char *, bool d = false) { return d; }
  size_t putBool(const char *, bool) { return 1; }
  String getString(const char *, const String &d = String()) { return d; }
  size_t putString(const char *, const String &) { return 1; }
};
