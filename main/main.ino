// /src/main.cpp
// DShot600 only, single pin (PIN=12), no separate telemetry wire.
// Arming kept EXACTLY as you require. 3D is enabled once at init (LSB=0), then re-arm.
// Loop linearly ramps speed from +5% to −5% and back (laminar), maintaining DShot packet cadence.

#include <Arduino.h>
#include <PIO_DShot.h>

#define PIN 12

#ifndef DSHOT_CMD_3D_MODE_ON
#define DSHOT_CMD_3D_MODE_ON 10
#endif
#ifndef DSHOT_CMD_SAVE_SETTINGS
#define DSHOT_CMD_SAVE_SETTINGS 12
#endif

// Raw 3D ranges (11-bit): 48..1047=reverse, 1049..2047=forward, 1048=gap, 0=stop
static constexpr uint16_t RAW_REV_MIN  = 48;
static constexpr uint16_t RAW_REV_MAX  = 1047;
static constexpr uint16_t RAW_FWD_MIN  = 1049;
static constexpr uint16_t RAW_FWD_MAX  = 2047;
static constexpr uint16_t PACKET_US     = 200;   // ~5 kHz cadence

BidirDShotX1 *esc = nullptr;
bool escInitialized = false;

// forward declaration for use in SendThrottle(int16_t)
static inline void sendRaw(uint16_t raw11);

// Your function name/signature preserved
void SendThrottle(uint16_t value) {
  if (escInitialized && esc) {
    esc->sendThrottle(value);
  }
}

// OVERLOAD: 3D wrapper. Accepts -1000..1000 and converts to correct RAW.
void SendThrottle(int16_t value) {
  if (!escInitialized || !esc) return;
  if (value == 0) { sendRaw(0); return; }
  if (value > 1000) value = 1000;
  if (value < -1000) value = -1000;

  if (value > 0) {
    const long in_min = 1, in_max = 1000;
    const long out_min = RAW_FWD_MIN, out_max = RAW_FWD_MAX;
    long raw = out_min + (long)(value - in_min) * (out_max - out_min) / (in_max - in_min);
    if (raw < RAW_FWD_MIN) raw = RAW_FWD_MIN;
    if (raw > RAW_FWD_MAX) raw = RAW_FWD_MAX;
    sendRaw((uint16_t)raw);
  } else { // value < 0
    // map -1..-1000 -> 48..1047
    long mag = -value; // 1..1000
    const long out_min = RAW_REV_MIN, out_max = RAW_REV_MAX;
    long raw = out_min + (mag - 1) * (out_max - out_min) / (1000 - 1);
    if (raw < RAW_REV_MIN) raw = RAW_REV_MIN;
    if (raw > RAW_REV_MAX) raw = RAW_REV_MAX;
    sendRaw((uint16_t)raw);
  }
}

static inline void sendRaw(uint16_t raw11) {
  if (!esc) return;
  if (raw11 > 2047) raw11 = 2047;
  esc->sendRaw12Bit(raw11 << 1); // LSB=0 (no telemetry)
}

static void armZeros_600ms() {
  for (int i = 0; i < 3000; i++) {
    esc->sendThrottle(0);
    delayMicroseconds(PACKET_US);
  }
}

static void sendCmdRepeated(uint16_t cmd, uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    sendRaw(cmd);
    delayMicroseconds(PACKET_US);
  }
}

// === Init: DShot600 only, enable 3D, re-arm ===
void initializeESC() {
  delay(500);
  digitalWrite(PIN, LOW);
  delay(500);

  esc = new BidirDShotX1(PIN, 600); // DShot600 only

  // Arming (exactly as requested)
  armZeros_600ms();

  // Enable 3D (no telemetry) and save, then re-arm
  sendCmdRepeated(DSHOT_CMD_3D_MODE_ON, 10);
  sendCmdRepeated(DSHOT_CMD_SAVE_SETTINGS, 10);
  armZeros_600ms();

  escInitialized = true;
}

void setup() {
  pinMode(PIN, OUTPUT);
  initializeESC();
}

void loop() {
  static int val = 50;      // current command in −50..50
  static int dir = -1;      // ramp direction: −1 or +1
  static int hold = 0;      // iterations spent on current step
  const int HOLD = 1000;    // much slower: ~200 ms per step at 200 µs packet cadence (~20 s from +50 to −50)

  // send one packet per iteration, maintain cadence with a single delay below
  SendThrottle((int16_t)val);  // cast to resolve overload (int → int16_t)

  // laminar ramp logic without extra delays
  if (++hold >= HOLD) {
    hold = 0;
    val += dir;
    if (val <= -50) { val = -50; dir = +1; }
    if (val >=  50) { val =  10; dir = -1; }
  }

  // single cadence delay
  delayMicroseconds(PACKET_US);
}
