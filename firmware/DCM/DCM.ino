/**
 * DCM.ino — Drone Communication Module firmware (LilyGO T-Beam Supreme, board v3.1)
 *
 * Straps onto the drone and streams telemetry to the DDS over LoRa.
 * Completely independent of the drone's flight controller.
 *
 * ---------------------------------------------------------------- BEHAVIOR
 *   1. Every boot starts asleep.
 *   2. SLEEP is the only way to stop streaming.
 *   3. A valid WAKE (right msg + pair ID) is the only way out of sleep.
 *      Any other packet is ignored and the DCM goes back to sleep.
 *   4. TELEMETRY at 1 Hz. Between sends, listen for SLEEP (Class A).
 *   5. A valid RESTART (asleep or streaming) reboots the ESP32, which comes back asleep.
 *   6. A valid CHECK_BATTERY while asleep sends a few battery-only TELEMETRY frames
 *      (sensor ok flags off), then goes back to sleep.
 *   7. Low battery: the AXP2101 PMU cuts power at its shutdown voltage.
 *
 *   Requirement: about 4 h of streaming per charge; charged between missions.
 *   A reset mid-flight (brownout, crash) comes back asleep, so the DDS must
 *   resend WAKE if telemetry times out while ARMED.
 *
 * ---------------------------------------------------------------- FSM
 *   boot -----------------------> SLEEP
 *   SLEEP ----(our WAKE)--------> TRANSMIT   [sensors + GPS on, re-init them]
 *   SLEEP ----(our CHECK_BATTERY)> SLEEP     [send battery TELEMETRY x3, sensors stay off]
 *   SLEEP ----(any other packet)> SLEEP      [back to light sleep]
 *   TRANSMIT -(1 s elapsed)-----> TRANSMIT   [read sensors, send TELEMETRY]
 *   TRANSMIT -(our SLEEP)-------> SLEEP      [sensors + GPS off]
 *   any ------(our RESTART)-----> boot       [ESP.restart()]
 *
 *   SLEEP: sensors (ALDO1/2) and GPS (ALDO4, GPS_EN) are off. The radio stays
 *   in normal continuous receive, and the ESP32 light-sleeps until DIO1 goes
 *   high (a packet arrived). Light sleep keeps RAM, so loop() carries on
 *   where it left off and the radio object stays set up. About 6 mA.
 *
 *   The GPS cold-starts on every WAKE: no position for 30 s to a few minutes.
 *   The DDS shouldn't treat "no fix yet" right after ARM as a fault.
 *
 *   No delay() in TRANSMIT: FeedGPS() must run often. The one exception is
 *   the blocking transmit() (~140 ms), which the 256-byte GPS UART buffer covers.
 *
 * ---------------------------------------------------------------- BENCH
 *   Serial monitor at 115200. Each TELEMETRY sent prints a few lines.
 *   The charge LED is steady on in SLEEP, and in TRANSMIT flashes once per
 *   TELEMETRY sent (~140 ms, the time on air). It no longer shows charging status.
 *   The Heltec (bench/heltec_rx) stands in for the DDS, or type a command
 *   letter in the monitor: w = WAKE, s = SLEEP, r = RESTART, b = CHECK_BATTERY.
 *   On USB power the DCM skips light sleep (it loops awake in SLEEP), so the
 *   monitor stays connected. Measure sleep current on battery only.
 *
 * ---------------------------------------------------------------- PROTOCOL
 *   ../shared/protocol.h: radio settings, frames, buildTelemetry().
 *
 * ---------------------------------------------------------------- HARDWARE
 *   SX1262 radio (SPI)    CS 10, SCK 12, MOSI 11, MISO 13, RST 5, DIO1 1, BUSY 4, TCXO 1.8 V
 *   Wire1 (I2C)           SDA 42, SCL 41: AXP2101 PMU (+ PCF8563 RTC, unused)
 *   Wire  (I2C)           SDA 17, SCL 18: QMC6309 magnetometer @0x7C, BME280 @0x77
 *   IMU SPI (QMI8658)     CS 34, MOSI 35, MISO 37, SCK 36  (its own SPIClass, HSPI)
 *   GNSS (UART, 9600)     RX 9, TX 8, enable 7
 *   PMU rails             ALDO1/2 sensors, ALDO3 radio, ALDO4 GNSS (all 3.3 V)
 *
 *   Gotchas from the bench test:
 *   - Don't run a full I2C address scan on Wire; it broke later sensor reads.
 *   - Sensors lose their configuration when ALDO1/2 power off: re-run their
 *     Begin() after powering back on (LilyGO waits ~250 ms after power-up).
 *
 * ---------------------------------------------------------------- BUILD
 *   Board:  "ESP32S3 Dev Module", USB CDC On Boot: Enabled, Flash Size: 8MB,
 *           PSRAM: Disabled (OPI PSRAM takes GPIO 35-37, which the IMU uses)
 *   Libraries (Library Manager): RadioLib, XPowersLib, SensorLib (Lewis He),
 *           Adafruit BME280 Library, TinyGPSPlus
 *   Reference: firmware/bench/dcm_reference/dcm_reference.ino (older deep-sleep
 *           design and frame format; still useful for hardware setup)
 */

#include <SPI.h>
#include <Wire.h>
#include <esp_sleep.h>         // light sleep and its wake sources
#include <driver/gpio.h>       // gpio_wakeup_enable: wake from light sleep on DIO1
#include <RadioLib.h>          // SX1262
#define XPOWERS_CHIP_AXP2101   // must come before the XPowersLib include
#include <XPowersLib.h>        // AXP2101 PMU
#include <ImuDrv.hpp>          // SensorLib: QMI8658
#include <MagnetometerDrv.hpp> // SensorLib: QMC6309
#include <Adafruit_BME280.h>
#include <TinyGPSPlus.h>

#include "../shared/protocol.h"

// ---------------------------------------------------------------- pins

constexpr int RADIO_CS   = 10;
constexpr int RADIO_SCK  = 12;
constexpr int RADIO_MOSI = 11;
constexpr int RADIO_MISO = 13;
constexpr int RADIO_RST  = 5;
constexpr int RADIO_DIO1 = 1;
constexpr int RADIO_BUSY = 4;

constexpr int PMU_SDA    = 42;
constexpr int PMU_SCL    = 41;
constexpr int SENSOR_SDA = 17;
constexpr int SENSOR_SCL = 18;

constexpr int IMU_CS     = 34;
constexpr int IMU_MOSI   = 35;
constexpr int IMU_MISO   = 37;
constexpr int IMU_SCK    = 36;

constexpr int GPS_RX     = 9;
constexpr int GPS_TX     = 8;
constexpr int GPS_EN     = 7;

// ---------------------------------------------------------------- settings

constexpr uint32_t kTelemetryPeriodMs = 1000;
constexpr int8_t   kTxPowerDbm        = 14;    // bench value; flight value TBD
constexpr float    kTcxoVolts         = 1.8f;
constexpr uint16_t kPmuShutdownMv     = 3000;  // check against the 18650's datasheet
constexpr uint32_t kFixMaxAgeMs       = 2000;  // older than this = no fix
constexpr uint32_t kGpsBaud           = 9600;
constexpr int      kBatteryCopies     = 3;     // CHECK_BATTERY replies; extras in case one is lost
constexpr uint32_t kBatteryGapMs      = 200;   // between copies, so the DDS reads one before the next lands

enum State { TRANSMIT, SLEEP }; 
State state;
XPowersPMU power;

SPIClass      imuSpi(HSPI);  // IMU gets its own SPI bus; the radio uses the default one
SensorQMI8658 imu;
SensorQMC6309 mag;
Adafruit_BME280 bme;
TinyGPSPlus gps;
SX1262 radio = new Module(RADIO_CS,RADIO_DIO1, RADIO_RST, RADIO_BUSY);
bool pmuUp = false;          // begin() succeeded; if false, don't try to read
bool imuUp = false;
bool magUp = false;
bool bmeUp = false;
bool radioUp = false;

uint32_t tLastSend = 0;
uint16_t seq = 0;            // telemetry frame counter; restarts at 0 every boot


void PowerON(){
  pmuUp = power.begin(Wire1, AXP2101_SLAVE_ADDRESS, PMU_SDA, PMU_SCL);
  if (!pmuUp) {
    Serial.println("PMU not found: sensors, radio and GPS may be unpowered");
    return;
  }

  power.setALDO3Voltage(3300);
  power.enableALDO3();

  power.enableBattDetection();
  power.enableBattVoltageMeasure();

  power.setSysPowerDownVoltage(kPmuShutdownMv);
  
  return;
}

void SetTransmitMode(){
  power.setALDO1Voltage(3300);
  power.enableALDO1();
  power.setALDO2Voltage(3300);
  power.enableALDO2();
  GPSEnable(true);
  delay(250);
  IMUBegin();
  MagBegin();
  BMEBegin();
  if (pmuUp) power.setChargingLedMode(XPOWERS_CHG_LED_OFF);   // TX flashes only from here on
  state = TRANSMIT;
  return;
}

void SetSleepMode(){
  power.disableALDO1();
  power.disableALDO2();
  GPSEnable(false);
  if (pmuUp) power.setChargingLedMode(XPOWERS_CHG_LED_ON);    // steady on = asleep
  state = SLEEP;
  return;
}
void BMEBegin(){
  bmeUp = bme.begin(BME280_ADDRESS, &Wire);
  if (!bmeUp){
    Serial.println("BME bad");
    return;
  }
}

/** Fills tempC (degC), pressurePa (Pa) and humidityPct (%). Returns false if the BME280 didn't start. */
bool ReadBME(float &tempC, float &pressurePa, float &humidityPct){
  if (!bmeUp) { return false; }
  tempC       = bme.readTemperature();
  pressurePa  = bme.readPressure();
  humidityPct = bme.readHumidity();
  return true;
}

/** Turns the GNSS module, its power rail and its UART all on (true) or off (false). */
void GPSEnable(bool on){
  pinMode(GPS_EN, OUTPUT);
  if (on) {
    power.setALDO4Voltage(3300);
    power.enableALDO4();
    Serial1.begin(kGpsBaud, SERIAL_8N1, GPS_RX, GPS_TX);  // UART to the GNSS module
    digitalWrite(GPS_EN, HIGH);                           // HIGH = GNSS module on
  } else {
    // Release every pin first, so none is held high into the unpowered module
    digitalWrite(GPS_EN, LOW);
    Serial1.end();
    power.disableALDO4();
  }
}

/** Hands every byte the GNSS has sent to the parser. Call every loop(), or bytes pile up and get lost. */
void FeedGPS(){
  while (Serial1.available()) {
    gps.encode(Serial1.read());
  }
}

/** Fills position (deg), altitude (m) and hdop. Returns false if there's no fresh fix. */
bool ReadGPS(double &latDeg, double &lonDeg, float &altM, float &hdop){
  latDeg = gps.location.lat();
  lonDeg = gps.location.lng();
  altM   = gps.altitude.meters();
  hdop   = gps.hdop.hdop();
  return gps.location.isValid() && gps.location.age() < kFixMaxAgeMs;
}

void IMUBegin(){
  imuUp = imu.begin(imuSpi, IMU_CS, IMU_MOSI, IMU_MISO, IMU_SCK);
  if (!imuUp) {
    Serial.println("IMU not found");
    return;
  }
  // Range: the biggest reading before it clips. Rate: samples per second.
  imu.configAccel(AccelFullScaleRange::FS_8G, 1000.0f);
  imu.configGyro(GyroFullScaleRange::FS_1000_DPS, 1000.0f);
  imu.enableAccel();
  imu.enableGyro();
}

void MagBegin(){
  // Also starts Wire on the sensor pins, so the BME280 can use Wire after this
  magUp = mag.begin(Wire, QMC6309_SLAVE_ADDRESS, SENSOR_SDA, SENSOR_SCL);
  if (!magUp) {
    Serial.println("Magnetometer not found");
    return;
  }
  // Continuous: the chip measures on its own, 200 times a second
  mag.configMagnetometer(OperationMode::CONTINUOUS_MEASUREMENT, MagFullScaleRange::FS_8G,
                         200.0f, MagOverSampleRatio::OSR_1, MagDownSampleRatio::DSR_1);
}

/** Fills accelMps2 (m/s^2) and gyroDps (deg/s) as x, y, z. Returns false if the read failed. */
bool ReadIMU(float accelMps2[3], float gyroDps[3]){
  if (!imuUp) return false;

  AccelerometerData a;
  GyroscopeData g;
  if (!imu.readAccel(a) || !imu.readGyro(g)) return false;

  accelMps2[0] = a.mps2.x;
  accelMps2[1] = a.mps2.y;
  accelMps2[2] = a.mps2.z;
  gyroDps[0] = g.dps.x;
  gyroDps[1] = g.dps.y;
  gyroDps[2] = g.dps.z;
  return true;
}

/** Fills magGauss (gauss) as x, y, z. Returns false if the read failed or no new sample was ready. */
bool ReadMag(float magGauss[3]){
  if (!magUp) return false;

  MagnetometerData m;
  if (!mag.readData(m)) return false;

  magGauss[0] = m.magnetic_field.x;
  magGauss[1] = m.magnetic_field.y;
  magGauss[2] = m.magnetic_field.z;
  return true;
}

void RadioBegin(){
  SPI.begin(RADIO_SCK, RADIO_MISO, RADIO_MOSI, RADIO_CS);
  int16_t status = radio.begin(mapds::kFreqMHz, mapds::kBandwidthKHz, mapds::kSpreadingFactor, mapds::kCodingRate,
                               mapds::kSyncWord, kTxPowerDbm, mapds::kPreamble, kTcxoVolts);
  radioUp = (status == RADIOLIB_ERR_NONE);
  if (!radioUp) {
    Serial.printf("Radio init failed (%d)\n", status);  // look the code up in RadioLib's status codes
    return;
  }
  radio.startReceive();
}
/**
 * Checks the radio for a command. Returns mapds::kMsgWake, kMsgSleep, kMsgRestart or kMsgCheckBattery
 * if one of ours arrived, or 0 for nothing, junk, or someone else's packet.
 * Only the first call after a packet arrives sees it: reading clears it.
 */
uint8_t ReadPacket(){
  if (!radioUp) { return 0; }
  if (digitalRead(RADIO_DIO1) == LOW) { return 0; }   // DIO1 high = a packet is waiting

  size_t len = radio.getPacketLength();               // before readData(), which clears the packet
  uint8_t buf[sizeof(mapds::Header)];
  int16_t st = radio.readData(buf, sizeof(buf));      // always read: caps at sizeof(buf), clears DIO1
  if (st != RADIOLIB_ERR_NONE) { return 0; }          // e.g. CRC error: corrupted in the air
  if (len != sizeof(mapds::Header)) { return 0; }     // commands are a bare Header

  mapds::Header h;
  memcpy(&h, buf, sizeof(h));
  if (h.pairId != mapds::kPairId) { return 0; }       // not our DDS
  if (h.msg == mapds::kMsgWake || h.msg == mapds::kMsgSleep || h.msg == mapds::kMsgRestart ||
      h.msg == mapds::kMsgCheckBattery) { return h.msg; }
  return 0;
}

/** Bench only: a letter typed in the serial monitor (w, s, r, b) stands in for a DDS command. */
uint8_t ReadSerial(){
  if (!Serial.available()) { return 0; }
  switch (Serial.read()) {
    case 'w': return mapds::kMsgWake;
    case 's': return mapds::kMsgSleep;
    case 'r': return mapds::kMsgRestart;
    case 'b': return mapds::kMsgCheckBattery;
  }
  return 0;   // anything else, including the newline
}

/** A command from the radio, or failing that from the serial monitor. 0 = none. */
uint8_t ReadCommand(){
  uint8_t msg = ReadPacket();
  if (msg == 0) { msg = ReadSerial(); }
  return msg;
}

/**
 * Prints one frame to the serial monitor as it goes over the air.
 * Safe with no laptop attached: USB serial drops writes when no host is connected.
 */
void PrintFrame(const mapds::TelemetryFrame &f){
  Serial.printf("seq=%u ok pmu=%d gnss=%d imu=%d mag=%d bme=%d | lat=%.7f lon=%.7f alt=%.1fm hdop=%.1f | batt=%umV | %.1fC %.0fPa %.0f%%\n",
                f.header.seq, f.pmuOk, f.gnssOk, f.imuOk, f.magOk, f.bmeOk, f.latDeg, f.lonDeg, f.gpsAltM, f.hdop,
                f.battMv, f.tempC, f.pressurePa, f.humidityPct);
  Serial.printf("    accel=%.2f,%.2f,%.2f m/s2 | gyro=%.1f,%.1f,%.1f dps | mag=%.3f,%.3f,%.3f G\n",
                f.accelMps2[0], f.accelMps2[1], f.accelMps2[2],
                f.gyroDps[0], f.gyroDps[1], f.gyroDps[2],
                f.magGauss[0], f.magGauss[1], f.magGauss[2]);
  // Not in the frame: GPS link health. chars should keep climbing; badChecksum should stay near 0.
  Serial.printf("    gps: sats=%lu chars=%lu fixSentences=%lu badChecksum=%lu\n",
                (unsigned long)gps.satellites.value(), (unsigned long)gps.charsProcessed(),
                (unsigned long)gps.sentencesWithFix(), (unsigned long)gps.failedChecksum());
}

/** Reboots the ESP32. Never returns; setup() runs again and the DCM comes back asleep. */
void Restart(){
  Serial.println("RESTART received");
  Serial.flush();
  SetSleepMode();   // sensors + GPS off, same as a fresh boot
  ESP.restart();
}
void CheckBattery(){
  Serial.println("CHECK_BATTERY received");
  if (!radioUp) { return; }

  float zeros[3] = {0, 0, 0};
  for (int i = 0; i < kBatteryCopies; i++) {
    uint16_t battMv = pmuUp ? power.getBattVoltage() : 0;
    mapds::TelemetryFrame frame = mapds::buildTelemetry(seq, pmuUp, false, false, false, false, 0, 0, 0, 0, 0, battMv, 0, 0, zeros, zeros, zeros);
    PrintFrame(frame);

    int16_t radio_status = radio.transmit(reinterpret_cast<uint8_t*>(&frame), sizeof(frame));
    if (radio_status != RADIOLIB_ERR_NONE) {
      Serial.printf("TX failed (%d)\n", radio_status);
    }
    seq+=1;
    delay(kBatteryGapMs);
  }
  radio.startReceive();   // transmit() leaves the radio deaf; listen for WAKE again
}

void Transmit(){
  // Every pass, not just once a second: notice a SLEEP or RESTART right away
  uint8_t msg = ReadCommand();
  if (msg == mapds::kMsgRestart) {
    Restart();
  }
  if (msg == mapds::kMsgSleep) {
    Serial.println("SLEEP received");
    SetSleepMode();
    return;
  }

  if (millis() - tLastSend < kTelemetryPeriodMs) {
    return;
  }

  double latDeg;
  double lonDeg;
  float gpsAltM;
  float hdop;
  float pressurePa;
  float tempC;
  float humidityPct;
  float accelMps2[3];
  float gyroDps[3];
  float magGauss[3];
  uint16_t battMv = power.getBattVoltage();   // 0 with no battery
  bool imuOk = ReadIMU(accelMps2, gyroDps);
  bool magOk = ReadMag(magGauss);
  bool bmeOk = ReadBME(tempC, pressurePa, humidityPct);
  bool GPSOk = ReadGPS(latDeg, lonDeg, gpsAltM, hdop);
  mapds::TelemetryFrame frame = mapds::buildTelemetry(seq, pmuUp, GPSOk, imuOk, magOk, bmeOk, latDeg, lonDeg, gpsAltM, hdop, pressurePa, battMv, tempC, humidityPct, accelMps2, gyroDps, magGauss);
  PrintFrame(frame);

  if (radioUp) {
    // The PMU's charge LED doubles as a TX light: lit while the packet is on air
    if (pmuUp) power.setChargingLedMode(XPOWERS_CHG_LED_ON);
    int16_t radio_status = radio.transmit(reinterpret_cast<uint8_t*>(&frame), sizeof(frame));
    if (pmuUp) power.setChargingLedMode(XPOWERS_CHG_LED_OFF);
    if (radio_status != RADIOLIB_ERR_NONE) {
      Serial.printf("TX failed (%d)\n", radio_status);
    }
    radio.startReceive();   // transmit() leaves the radio deaf; listen for SLEEP (and WAKE once asleep)
  }

  seq+=1;
  tLastSend = millis();
}
void Sleep(){
  // On USB power (the bench) stay awake, so the serial monitor can send commands
  if (!(pmuUp && power.isVbusGood())) {
    Serial.flush();             // let pending prints finish before the CPU stops
    esp_light_sleep_start();    // stops here until DIO1 goes high (packet arrived)
  }
  uint8_t msg = ReadCommand();
  if (msg == mapds::kMsgRestart) {
    Restart();
  }
  if (msg == mapds::kMsgWake) {
    Serial.println("WAKE received");
    SetTransmitMode();
  }
  if (msg == mapds::kMsgCheckBattery){
    CheckBattery();
  }
  // anything else: return, and the next loop() sleeps again
}

void setup() {
  Serial.begin(115200);
  PowerON();   
  RadioBegin();
  SetSleepMode();
  gpio_wakeup_enable((gpio_num_t)RADIO_DIO1, GPIO_INTR_HIGH_LEVEL);  // wake from light sleep on DIO1
  esp_sleep_enable_gpio_wakeup();
}

void loop() {
  FeedGPS();


  switch(state) {
    case TRANSMIT:
      Transmit();
      break;
    case SLEEP:
      Sleep();
      break;
  }
}
