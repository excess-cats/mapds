/**
 communication protocol (packet structure)
 */
#pragma once

#include <stdint.h>

namespace mapds {

// ---- Identity ----
constexpr uint8_t kPairId = 0x01;  // unique per DDS/DCM pair

// ---- Radio (both ends must match) ----
constexpr float    kFreqMHz          = 915.0f;
constexpr float    kBandwidthKHz     = 125.0f;
constexpr uint8_t  kSpreadingFactor  = 7;
constexpr uint8_t  kCodingRate       = 5;     // 4/5
constexpr uint8_t  kSyncWord         = 0x12;
constexpr uint16_t kPreamble         = 16;    // symbols

// ---- Message IDs (header byte 0) ----
constexpr uint8_t kMsgTelemetry = 1;  // DCM -> DDS
constexpr uint8_t kMsgWake      = 2;  // DDS -> DCM
constexpr uint8_t kMsgSleep     = 3;  // DDS -> DCM
constexpr uint8_t kMsgRestart   = 4;  // DDS -> DCM

// ---- Frames ----
// Packed, so the struct's bytes are exactly the wire bytes (little-endian,
// IEEE-754 floats on both ESP32 and x86). Send with memcpy into/out of the
// radio buffer. Don't take a pointer or reference to a packed member; copy it out first.

struct __attribute__((packed)) Header {
  uint8_t  msg;     // kMsg*
  uint8_t  pairId;  // kPairId
  uint16_t seq;     // restarts at 0 every DCM boot
};

// WAKE, SLEEP and RESTART are a bare Header.

struct __attribute__((packed)) TelemetryFrame {
  Header  header;

  bool    pmuOk; 
  bool    gnssOk;
  bool    imuOk; 
  bool    magOk; 
  bool    bmeOk; 

  double  latDeg;        // double: a float can't hold 7 decimal places of 122.xxx
  double  lonDeg;
  float   gpsAltM;       // m above sea level
  float   hdop;          // GPS accuracy, lower is better (~1 good, >5 poor)

  float   pressurePa;

  int8_t  battPct;       // 0-100, or -1 with no battery

  float   tempC;
  float   humidityPct;

  float   accelMps2[3];  // x, y, z
  float   gyroDps[3];
  float   magGauss[3];
};

// Compile-time contracts: a layout change breaks the build, not the radio link.
static_assert(sizeof(Header) == 4, "Header must be 4 bytes");
static_assert(sizeof(bool) == 1, "wire format assumes 1-byte bool");
static_assert(sizeof(TelemetryFrame) == 82, "TelemetryFrame must be 82 bytes");
static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__, "wire format is little-endian");

// ---- Builders ----
// `inline` because this is a header included by several files (One Definition Rule).

/** A header for any message type. */
inline Header buildHeader(uint8_t msg, uint16_t seq) {
  Header h = {};
  h.msg    = msg;
  h.pairId = kPairId;
  h.seq    = seq;
  return h;
}

/** WAKE: DDS -> DCM. */
inline Header buildWake(uint16_t seq) {
  return buildHeader(kMsgWake, seq);
}

/** SLEEP: DDS -> DCM. */
inline Header buildSleep(uint16_t seq) {
  return buildHeader(kMsgSleep, seq);
}

/** RESTART: DDS -> DCM. The DCM reboots and comes back asleep. */
inline Header buildRestart(uint16_t seq) {
  return buildHeader(kMsgRestart, seq);
}

/**
 * TELEMETRY: DCM -> DDS. Copies every value into the frame.
 * Parameters are in frame order.
 *
 * gnssOk means the GPS has a fix. battPct is -1 with no battery.
 * accelMps2, gyroDps, magGauss are x, y, z.
 */
inline TelemetryFrame buildTelemetry(uint16_t seq,
                                     bool pmuOk, bool gnssOk, bool imuOk, bool magOk, bool bmeOk,
                                     double latDeg, double lonDeg, float gpsAltM, float hdop,
                                     float pressurePa,
                                     int battPct,
                                     float tempC, float humidityPct,
                                     const float accelMps2[3], const float gyroDps[3], const float magGauss[3]) {
  TelemetryFrame f = {};
  f.header = buildHeader(kMsgTelemetry, seq);

  f.pmuOk  = pmuOk;
  f.gnssOk = gnssOk;
  f.imuOk  = imuOk;
  f.magOk  = magOk;
  f.bmeOk  = bmeOk;

  f.latDeg  = latDeg;
  f.lonDeg  = lonDeg;
  f.gpsAltM = gpsAltM;
  f.hdop    = hdop;

  f.pressurePa = pressurePa;

  f.battPct = battPct;

  f.tempC       = tempC;
  f.humidityPct = humidityPct;

  for (int i = 0; i < 3; i++) {
    f.accelMps2[i] = accelMps2[i];
    f.gyroDps[i]   = gyroDps[i];
    f.magGauss[i]  = magGauss[i];
  }
  return f;
}

}  // namespace mapds
