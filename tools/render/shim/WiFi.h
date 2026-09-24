// Host stand-in: a few fixed networks, a scan that takes 1.5 s of virtual
// time, and a connection that succeeds after 2 s unless the password is
// "wrong".
#pragma once
#include <Arduino.h>

typedef enum { WIFI_AUTH_OPEN = 0, WIFI_AUTH_WPA2_PSK = 3 } wifi_auth_mode_t;
typedef enum { WL_IDLE_STATUS = 0, WL_NO_SSID_AVAIL = 1, WL_CONNECTED = 3, WL_CONNECT_FAILED = 4,
               WL_DISCONNECTED = 6 } wl_status_t;
#define WIFI_STA 1
#define WIFI_OFF 0
#define WIFI_SCAN_RUNNING (-1)
#define WIFI_SCAN_FAILED (-2)

class IPAddress {
 public:
  String toString() const { return String("192.168.1.42"); }
};

class WiFiClass {
 public:
  void mode(int) {}
  int16_t scanNetworks(bool async = false) {
    _scanAt = g_now;
    return async ? WIFI_SCAN_RUNNING : 5;
  }
  int16_t scanComplete() { return g_now - _scanAt >= 1500 ? 5 : WIFI_SCAN_RUNNING; }
  void scanDelete() {}
  String SSID(int i) { static const char *n[] = {"Workshop", "Lonely Binary", "Neighbours 5G", "Cafe Guest", "Printer-Setup"}; return String(i < 0 ? _ssid.c_str() : n[i % 5]); }
  int32_t RSSI(int i = -1) { static const int r[] = {-48, -57, -71, -80, -88}; return i < 0 ? -52 : r[i % 5]; }
  wifi_auth_mode_t encryptionType(int i) { return i == 3 ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK; }
  wl_status_t begin(const char *ssid, const char *pass = nullptr) {
    _ssid = ssid; _ok = !pass || strcmp(pass, "wrong"); _beginAt = g_now; _started = true;
    return WL_DISCONNECTED;
  }
  wl_status_t status() {
    if (!_started) return WL_DISCONNECTED;
    if (g_now - _beginAt < 2000) return WL_DISCONNECTED;
    return _ok ? WL_CONNECTED : WL_CONNECT_FAILED;
  }
  bool disconnect(bool = false) { _started = false; return true; }
  String SSID() { return _ssid; }
  IPAddress localIP() { return IPAddress(); }
  String macAddress() { return String("24:6F:28:AA:BB:CC"); }
 private:
  uint32_t _scanAt = 0, _beginAt = 0;
  bool _ok = false, _started = false;
  String _ssid;
};
static WiFiClass WiFi;
