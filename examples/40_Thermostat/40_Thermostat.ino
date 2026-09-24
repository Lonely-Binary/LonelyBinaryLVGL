/*
  40_Thermostat — a complete thermostat front panel.

  PUTS TOGETHER
    - an arc you drag to set the temperature (11_Arc)
    - big numbers in the house style (02_Styles)
    - a button matrix as a mode selector, one mode at a time (26_ButtonMatrix)
    - a timer running the control loop, separate from the drawing (01_Label)

  THE CONTROL LOGIC is real and small: heat or cool with hysteresis, so the
  relay does not chatter on and off around the set point. The room itself is
  simulated - replace readRoom() with your sensor and setRelay() with a
  digitalWrite to your relay, and this is a working thermostat.

  TOUCH: drag the ring to set the temperature; tap a mode. Without touch it
  runs and shows everything, at the set point it starts with.
*/
#include <LonelyBinaryDisplay.h>
#include <LonelyBinaryLVGL.h>

LB_Display display(LB_SQUARE_392_CTP);

// ── Settings ───────────────────────────────────────────────────────────────
static const float HYSTERESIS = 0.3f;      // switch on 0.3 C away from the set point
static const int SET_MIN = 10, SET_MAX = 30;

enum Mode { OFF, HEAT, COOL, AUTO };
static Mode mode = AUTO;
static float setPoint = 21.5f;
static float room = 18.4f;
static bool heating = false, cooling = false;

// ── Hardware (replace these) ───────────────────────────────────────────────
static float readRoom() {
  // Simulated room: drifts toward 16 C outside, pushed by the heater/cooler.
  room += (16.0f - room) * 0.002f;
  if (heating) room += 0.05f;
  if (cooling) room -= 0.05f;
  return room;
}
static void setRelay(bool heat, bool cool) {
  static bool h = false, c = false;
  if (heat != h || cool != c) Serial.printf("relay: heat %d cool %d\n", heat, cool);
  h = heat;
  c = cool;
}

// ── UI ─────────────────────────────────────────────────────────────────────
static lv_obj_t *dial, *setLabel, *roomLabel, *stateLabel;

static void showSetPoint() {
  const int tenths = (int)(setPoint * 10 + 0.5f);
  lv_label_set_text_fmt(setLabel, "%d.%d", tenths / 10, tenths % 10);
}

static void showState() {
  const int t = (int)(room * 10 + 0.5f);
  lv_label_set_text_fmt(roomLabel, "Room %d.%d C", t / 10, t % 10);
  const char *s = mode == OFF ? "OFF" : heating ? LV_SYMBOL_UP "  HEATING" : cooling ? LV_SYMBOL_DOWN "  COOLING" : "IDLE";
  lv_label_set_text(stateLabel, s);
  const lv_color_t c = heating ? LB_Style::warn : cooling ? LB_Style::accent : LB_Style::muted;
  lv_obj_set_style_text_color(stateLabel, c, 0);
  lv_obj_set_style_arc_color(dial, mode == OFF ? LB_Style::muted : c, LV_PART_INDICATOR);
}

// The control loop: once a second, decide what the relay should do.
static void control(lv_timer_t *) {
  const float r = readRoom();
  const bool mayHeat = mode == HEAT || mode == AUTO;
  const bool mayCool = mode == COOL || mode == AUTO;
  if (!mayHeat || r > setPoint + HYSTERESIS) heating = false;
  else if (r < setPoint - HYSTERESIS) heating = true;
  if (!mayCool || r < setPoint - HYSTERESIS) cooling = false;
  else if (r > setPoint + HYSTERESIS) cooling = true;
  setRelay(heating, cooling);
  showState();
}

static void onDial(lv_event_t *) {
  setPoint = lv_arc_get_value(dial) / 2.0f;   // the arc counts half degrees
  showSetPoint();
}

static void onMode(lv_event_t *e) {
  lv_obj_t *m = (lv_obj_t *)lv_event_get_target(e);
  mode = (Mode)lv_buttonmatrix_get_selected_button(m);
  control(nullptr);                           // react now, not in a second
}

void setup() {
  Serial.begin(115200);
  display.begin(true);
  LB_LVGL.begin(display);

  lv_obj_t *scr = lv_screen_active();
  const int32_t w = display.width(), h = display.height();
  const int32_t barH = h >= 240 ? 48 : 36;
  const int32_t d = LV_MIN(w, h - barH) - 16;

  // The dial: 270 degrees, in half-degree steps so the knob moves in 0.5 C.
  dial = lv_arc_create(scr);
  lv_obj_set_size(dial, d, d);
  lv_obj_align(dial, LV_ALIGN_TOP_MID, 0, 6);
  lv_arc_set_rotation(dial, 135);
  lv_arc_set_bg_angles(dial, 0, 270);
  lv_arc_set_range(dial, SET_MIN * 2, SET_MAX * 2);
  lv_arc_set_value(dial, (int32_t)(setPoint * 2));
  const int32_t thick = LV_MAX(d / 14, 6);
  lv_obj_set_style_arc_width(dial, thick, LV_PART_MAIN);
  lv_obj_set_style_arc_width(dial, thick, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(dial, LB_Style::surface, LV_PART_MAIN);
  lv_obj_set_style_bg_color(dial, LB_Style::text, LV_PART_KNOB);
  lv_obj_set_style_pad_all(dial, thick / 3, LV_PART_KNOB);
  lv_obj_add_event_cb(dial, onDial, LV_EVENT_VALUE_CHANGED, nullptr);

  // Inside the dial: state, set point, room temperature.
  stateLabel = lv_label_create(dial);
  lv_obj_add_style(stateLabel, &LB_Style::label, 0);
  lv_obj_align(stateLabel, LV_ALIGN_CENTER, 0, -d / 5);
  setLabel = lv_label_create(dial);
  lv_obj_add_style(setLabel, &LB_Style::value, 0);
  lv_obj_set_style_text_font(setLabel, LB_Style::valueFontFor(d * 3 / 4), 0);
  lv_obj_center(setLabel);
  roomLabel = lv_label_create(dial);
  lv_obj_set_style_text_color(roomLabel, LB_Style::muted, 0);
  lv_obj_align(roomLabel, LV_ALIGN_CENTER, 0, d / 5);

  // Mode selector: CHECKABLE keys, and "one checked" makes it a radio group.
  static const char *modes[] = {"Off", "Heat", "Cool", "Auto", ""};
  lv_obj_t *sel = lv_buttonmatrix_create(scr);
  lv_buttonmatrix_set_map(sel, modes);
  lv_buttonmatrix_set_button_ctrl_all(sel, LV_BUTTONMATRIX_CTRL_CHECKABLE);
  lv_buttonmatrix_set_one_checked(sel, true);
  lv_buttonmatrix_set_button_ctrl(sel, mode, LV_BUTTONMATRIX_CTRL_CHECKED);
  lv_obj_set_size(sel, w - 12, barH);
  lv_obj_align(sel, LV_ALIGN_BOTTOM_MID, 0, -4);
  lv_obj_set_style_pad_all(sel, 3, 0);
  lv_obj_set_style_bg_color(sel, LB_Style::surface, 0);
  lv_obj_set_style_border_width(sel, 0, 0);
  lv_obj_add_event_cb(sel, onMode, LV_EVENT_VALUE_CHANGED, nullptr);

  showSetPoint();
  control(nullptr);
  lv_timer_create(control, 1000, nullptr);
}

void loop() {
  LB_LVGL.loop();
}
