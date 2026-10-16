/**
 * DDS.ino — Drone Deployment System firmware (Heltec WiFi LoRa 32 V3)
 *
 * Holds the drone on the helicopter, releases it on command, and watches the
 * DCM's telemetry to decide whether the drone is healthy enough to deploy.
 * Design: boeing fsm v3(1).png at the repo root.
 *
 * The web UI is not part of this file yet. Every operator input and every
 * indicator is a plain variable, so a web layer can later set the switches
 * and read the state and lights. Until then, the serial monitor stands in.
 *
 * ---------------------------------------------------------------- STATES
 *   LOAD, STANDBY, NOT_LOADED, ARMED, SOFTWARE_ERROR,
 *   DEPLOYING, DEPLOYED, DEPLOY_ERROR, DEPLOY_FAILURE
 *
 *   Boot in STANDBY. If the limit switch is OPEN, the first loop() moves to NOT_LOADED.
 *
 * ---------------------------------------------------------------- INPUTS
 *   Switches (levels: stay on until turned off)  Load, Arm, Deploy
 *   Commands (one-shot: consumed when read)      END/SLEEP, ACKNOWLEDGE FAILURE
 *   Limit switch                                  CLOSED = drone present, OPEN = gone
 *   Software fault                                computed from telemetry (below)
 *
 *   END and ACK turn all three switches off. Otherwise a still-on Arm and
 *   Deploy would re-arm and re-fire straight out of Standby.
 *
 * ---------------------------------------------------------------- FSM (Mealy: actions happen on the arrow)
 *   STANDBY ------(Load on)---------> LOAD             [servo -> LOAD_POS]
 *   LOAD ---------(Load off)--------> STANDBY          [servo -> LOCK_POS]
 *   STANDBY ------(limit OPEN)------> NOT_LOADED
 *   NOT_LOADED ---(limit CLOSED)----> STANDBY
 *   NOT_LOADED ---(Load on)---------> LOAD             [servo -> LOAD_POS]
 *   STANDBY ------(Arm on)----------> ARMED            [send WAKE, reset health, start 15 s timer]
 *   ARMED --------(Arm off)---------> STANDBY          [send SLEEP]
 *   ARMED --------(limit OPEN)------> NOT_LOADED       [send SLEEP]
 *   ARMED --------(software fault)--> SOFTWARE_ERROR   [send RESTART, start 1 min timer]
 *   ARMED --------(Deploy on, 15 s up)> DEPLOYING      [servo -> RELEASE_POS, start 10 s timer]
 *   SOFTWARE_ERROR (1 min up)-------> ARMED            [send WAKE, reset health, start 15 s timer]
 *   SOFTWARE_ERROR (Arm off)--------> STANDBY          [send SLEEP]
 *   SOFTWARE_ERROR (limit OPEN)-----> NOT_LOADED       [send SLEEP]
 *   DEPLOYING ----(limit OPEN)------> DEPLOYED
 *   DEPLOYING ----(10 s, still CLOSED)> DEPLOY_ERROR   [jiggle the servo 3 times]
 *   DEPLOY_ERROR -(limit OPEN)------> DEPLOYED
 *   DEPLOY_ERROR -(jiggles done, still CLOSED)> DEPLOY_FAILURE
 *   DEPLOYED -----(END/SLEEP)-------> STANDBY          [send SLEEP, switches off]
 *   DEPLOY_FAILURE (ACK)------------> STANDBY          [send SLEEP, switches off]
 *
 *   Ignore any input that has no arrow out of the current state (e.g. Deploy in STANDBY).
 *   DEPLOYED -> STANDBY goes straight on to NOT_LOADED, because the limit switch is OPEN. That's expected.
 *   Every timer is "time since the transition that started it": save millis() once, on the arrow.
 *
 * ---------------------------------------------------------------- INDICATORS
 *   Armed light    ARMED and the 15 s timer is up (that is also when Deploy is allowed)
 *   Error light    SOFTWARE_ERROR, DEPLOY_ERROR, DEPLOY_FAILURE
 *   Status         DEPLOYED = success, DEPLOY_FAILURE = failure
 *   Warnings       no GPS fix (gnssOk false), DCM battery < 20 % (battPct -1 = on USB, no warning)
 *                  Shown only. They never cause a transition.
 *
 * ---------------------------------------------------------------- SOFTWARE FAULT (checked in ARMED only)
 *   Any one of these is a fault:
 *   1. No TELEMETRY for 10 s (counted from the WAKE, then from the last packet).
 *   2. More than 3 missed packets in a 10 s window. Missed = gaps in header.seq.
 *      The DCM restarts seq at 0 every boot, so a seq that goes backwards is a
 *      reboot, not a gap: resync to it.
 *   3. Any of pmuOk, imuOk, magOk, bmeOk false for 10 s straight.
 *      gnssOk is a warning only: the GPS cold-starts on every WAKE.
 *
 *   Accept a packet only if: length == sizeof(TelemetryFrame), msg == kMsgTelemetry, pairId == kPairId.
 *
 * ---------------------------------------------------------------- SERVO (DFRobot SER0070 = Feetech ST3215, 12 V)
 *   Serial bus servo: half-duplex TTL UART, 1 Mbps, through the bus driver board.
 *   Position 0-4095 = 360 deg (2047 = centre). Factory ID 1.
 *   SCServo:  SMS_STS st;  st.pSerial = &Serial1;
 *             st.WritePosEx(id, position, speed, acc)   move (returns at once; the servo moves on its own)
 *             st.Ping(id)                                -1 = no reply (check at boot)
 *             st.ReadPos(id)                             current position (useful for the jiggle)
 *   LOAD_POS, LOCK_POS, RELEASE_POS: TBD on the bench.
 *   Jiggle: 3 small moves either side of RELEASE_POS (~ +/-100 counts, TBD), then back to RELEASE_POS.
 *
 * ---------------------------------------------------------------- SERIAL (115200)
 *   Every transition prints "FROM -> TO" and its action. The serial test stand-in
 *   (keys for the switches, the limit switch, WAKE/SLEEP) was removed on 2026-10-16.
 *
 * ---------------------------------------------------------------- OPEN ISSUES
 *   - Command collisions: the DCM is deaf for ~140 ms per second while it transmits, so a
 *     SLEEP or RESTART can be lost. Options: send copies ~200 ms apart; send right after
 *     a received frame; or resend SLEEP while frames keep arriving in a "should be asleep" state.
 *
 * ---------------------------------------------------------------- PROTOCOL
 *   ../shared/protocol.h: radio settings, Header, TelemetryFrame, buildWake/Sleep/Restart().
 *
 * ---------------------------------------------------------------- HARDWARE
 *   Batteries            3.7 V 1-cell LiPo on the Heltec's JST connector (read with the onboard divider below).
 *                        12 V pack straight into the DRI0057 for the servo (read via the servo: st.ReadVoltage).
 *
 *   Power (no PMU on the Heltec: everything below is plain GPIO, from the V3.1 and V3.2 schematics)
 *   - VDD_3V3            always on. Powers the ESP32 and the SX1262 (no rail to enable for the radio).
 *   - Vext (GPIO 36)     switched 3.3 V: OLED + the "Vext" header pin. LOW = on, HIGH = off (pull-up, P-FET).
 *                        Leave it off unless something is wired to it. Don't power the limit switch from it.
 *   - Battery sense      ADC on GPIO 1, through a 390k/100k divider: VBAT = pin voltage * 4.9.
 *                        The divider is switched by ADC_Ctrl (GPIO 37). The DDS turns it on once in PowerOn()
 *                        and leaves it on: ~8.5 uA, nothing next to the board's ~100 mA.
 *                          V3.0/V3.1: LOW = divider on    V3.2: HIGH = divider on   V3.4 (ours): HIGH, bench-confirmed 2026-10-16
 *                        analogReadMilliVolts() returns calibrated mV. Read only after the divider is on.
 *
 *   SX1262 radio (SPI)   CS 8, SCK 9, MOSI 10, MISO 11, RST 12, BUSY 13, DIO1 14, TCXO 1.8 V, 14 dBm (bench)
 *   Limit switch         GPIO TBD. Wiring TBD: to GND with INPUT_PULLUP means LOW = CLOSED. Debounce ~20 ms.
 *   Servo bus (Serial1)  TX TBD, RX TBD, 1000000 baud, servo ID 1
 *                        via DFRobot DRI0057 driver board: Heltec TX -> board RX, Heltec RX -> board TX,
 *                        common GND, 12 V into the board's DC jack (must match the servo's 12 V).
 *                        Board's two shorting caps: set them for UART, not USB (check the silkscreen).
 *                        Check the board's TX idle voltage before wiring: the ESP32-S3 is 3.3 V only.
 *
 *   Gotchas:
 *   - transmit() also fires DIO1. Clear the received flag after sending, and startReceive() again (see bench/heltec_rx).
 *   - Don't use delay() for the FSM timers. The radio, the switch and the serial stand-in must keep being read.
 *     The jiggle may block for a second or two; nothing else matters in DEPLOY_ERROR.
 *
 * ---------------------------------------------------------------- BUILD
 *   Board:      "Heltec WiFi LoRa 32(V3)" (esp32 core 3.x)
 *   Libraries:  RadioLib, SCServo (FT&WS, Library Manager)
 */

#include <SPI.h>
#include <RadioLib.h>   // SX1262
#include <SCServo.h>    // Feetech ST3215 bus servo (SMS_STS class)

#include "../shared/protocol.h"


// ---------------------------------------------------------------- pins

constexpr int LED_PIN    = 35;
constexpr int ADC_CTRL   = 37;
constexpr int VBAT_PIN   = 1;    // battery divider output (ADC)

constexpr int RADIO_CS   = 8;
constexpr int RADIO_SCK  = 9;
constexpr int RADIO_MOSI = 10;
constexpr int RADIO_MISO = 11;
constexpr int RADIO_RST  = 12;
constexpr int RADIO_BUSY = 13;
constexpr int RADIO_DIO1 = 14;

constexpr int LIMIT_PIN  = -1;   // TBD: not wired yet
constexpr int SERVO_TX   = -1;   // TBD: Heltec TX -> DRI0057 RX
constexpr int SERVO_RX   = -1;   // TBD: Heltec RX <- DRI0057 TX

// ---------------------------------------------------------------- settings

constexpr int      kAdcCtrlOn   = HIGH;   // V3.4: HIGH confirmed on the bench; V3.0/V3.1 would be LOW
constexpr int8_t   kTxPowerDbm  = 14;     // bench value; flight value TBD
constexpr float    kTcxoVolts   = 1.8f;
constexpr uint32_t kServoBaud   = 1000000;
constexpr uint8_t  kServoId     = 1;      // factory default

enum State  {
    LOAD,
    STANDBY,
    NOT_LOADED,
    ARMED,
    SOFTWARE_ERROR,
    DEPLOYING,
    DEPLOY_ERROR,
    DEPLOYED,
    DEPLOY_FAILURE
};

State state = STANDBY;
uint32_t TimeStateEntered;
SX1262 radio = new Module(RADIO_CS, RADIO_DIO1, RADIO_RST, RADIO_BUSY);
bool radioUp = false;        // RadioBegin() succeeded; if false, never arm (no telemetry possible)
uint16_t cmdSeq = 0;         // command counter (header.seq on WAKE/SLEEP/RESTART)




// BEGIN: GLOBALS USED IN INTERFACE 
bool     dcmPmuOk       = false;
bool     dcmGnssOk      = false;   // GPS has a fix
bool     dcmImuOk       = false;
bool     dcmMagOk       = false;
bool     dcmBmeOk       = false;
double   dcmLatDeg      = 0;
double   dcmLonDeg      = 0;
float    dcmGpsAltM     = 0;       // m above sea level
float    dcmHdop        = 0;       // lower is better (~1 good, >5 poor)
float    dcmPressurePa  = 0;
int8_t   dcmBattPct     = -1;      // 0-100, or -1 with no battery
float    dcmTempC       = 0;
float    dcmHumidityPct = 0;
float    dcmAccelMps2[3] = {};     // x, y, z
float    dcmGyroDps[3]   = {};
float    dcmMagGauss[3]  = {};
uint32_t tLastPacket = 0;    // millis() when the last valid frame arrived
float    lastRssi    = 0;    // dBm, of the last valid frame
float    lastSnr     = 0;    // dB

bool LoadCommand = false;
bool ArmCommand = false;
bool DeployCommand = false;
bool EndCommand = false;    
bool AckCommand = false;    

// END: GLOBALS USED IN INTERFACE




bool LimitClosed = true;     // true = drone present. Not read from LIMIT_PIN yet (GPIO TBD)

/** Name of a state, for serial prints. */
const char* StateName(State s){
    switch(s){
        case LOAD:           return "LOAD";
        case STANDBY:        return "STANDBY";
        case NOT_LOADED:     return "NOT_LOADED";
        case ARMED:          return "ARMED";
        case SOFTWARE_ERROR: return "SOFTWARE_ERROR";
        case DEPLOYING:      return "DEPLOYING";
        case DEPLOY_ERROR:   return "DEPLOY_ERROR";
        case DEPLOYED:       return "DEPLOYED";
        case DEPLOY_FAILURE: return "DEPLOY_FAILURE";
    }
    return "?";
}

/** The part every arrow shares: print it, switch state, restart the state timer. */
void UpdateState(State to){
    Serial.printf("%s -> %s\n", StateName(state), StateName(to));
    state = to;
    TimeStateEntered = millis();
}

bool StateChange(State to){
    State from = state;
    switch(from){
        case STANDBY:
            if(to == LOAD)       { UpdateState(to); Serial.println("  [servo -> LOAD_POS]"); return true; }
            if(to == NOT_LOADED) { UpdateState(to); return true; }
            if(to == ARMED)      { UpdateState(to); Serial.println("  [send WAKE, reset health]"); Transmit(mapds::buildWake(cmdSeq++)); return true; }
            break;
        case LOAD:
            if(to == STANDBY)    { UpdateState(to); Serial.println("  [servo -> LOCK_POS]"); return true; }
            break;
        case NOT_LOADED:
            if(to == STANDBY)    { UpdateState(to); return true; }
            if(to == LOAD)       { UpdateState(to); Serial.println("  [servo -> LOAD_POS]"); return true; }
            break;
        case ARMED:
            if(to == STANDBY)        { UpdateState(to); Serial.println("  [send SLEEP]"); Transmit(mapds::buildSleep(cmdSeq++)); return true; }
            if(to == NOT_LOADED)     { UpdateState(to); Serial.println("  [send SLEEP]"); Transmit(mapds::buildSleep(cmdSeq++)); return true; }
            if(to == SOFTWARE_ERROR) { UpdateState(to); Serial.println("  [send RESTART]"); Transmit(mapds::buildRestart(cmdSeq++)); return true; }
            if(to == DEPLOYING)      { UpdateState(to); Serial.println("  [servo -> RELEASE_POS]"); return true; }
            break;
        case SOFTWARE_ERROR:
            if(to == ARMED)      { UpdateState(to); Serial.println("  [send WAKE, reset health]"); Transmit(mapds::buildWake(cmdSeq++)); return true; }
            if(to == STANDBY)    { UpdateState(to); Serial.println("  [send SLEEP]"); Transmit(mapds::buildSleep(cmdSeq++)); return true; }
            if(to == NOT_LOADED) { UpdateState(to); Serial.println("  [send SLEEP]"); Transmit(mapds::buildSleep(cmdSeq++)); return true; }
            break;
        case DEPLOYING:
            if(to == DEPLOYED)     { UpdateState(to); return true; }
            if(to == DEPLOY_ERROR) { UpdateState(to); Serial.println("  [jiggle servo x3]"); return true; }
            break;
        case DEPLOY_ERROR:
            if(to == DEPLOYED)       { UpdateState(to); return true; }
            if(to == DEPLOY_FAILURE) { UpdateState(to); return true; }
            break;
        case DEPLOYED:
            if(to == STANDBY)    { UpdateState(to); Serial.println("  [send SLEEP, switches off]"); Transmit(mapds::buildSleep(cmdSeq++)); return true; }
            break;
        case DEPLOY_FAILURE:
            if(to == STANDBY)    { UpdateState(to); Serial.println("  [send SLEEP, switches off]"); Transmit(mapds::buildSleep(cmdSeq++)); return true; }
            break;
    }
    Serial.printf("No arrow %s -> %s, ignored\n", StateName(from), StateName(to));
    return false;
}

// STATES
void Load(){
    // exits: Load off -> STANDBY
}

void Standby(){
    // exits: Load on -> LOAD, limit OPEN -> NOT_LOADED, Arm on -> ARMED
}

void NotLoaded(){
    // exits: limit CLOSED -> STANDBY, Load on -> LOAD
}

void Armed(){
    // exits: Arm off -> STANDBY, limit OPEN -> NOT_LOADED, software fault -> SOFTWARE_ERROR,
    //        Deploy on and 15 s up -> DEPLOYING
}

void SoftwareError(){
    // exits: 1 min up -> ARMED, Arm off -> STANDBY, limit OPEN -> NOT_LOADED
}

void Deploying(){
    // exits: limit OPEN -> DEPLOYED, 10 s up and still CLOSED -> DEPLOY_ERROR
}

void DeployError(){
    // exits: limit OPEN -> DEPLOYED, still CLOSED -> DEPLOY_FAILURE
}

void Deployed(){
    // exits: END/SLEEP -> STANDBY
}

void DeployFailure(){
    // exits: ACKNOWLEDGE FAILURE -> STANDBY
}

// END STATES

void PowerOn(){
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);                  // power light

    pinMode(ADC_CTRL, OUTPUT);
    digitalWrite(ADC_CTRL, kAdcCtrlOn);           // battery divider on, and left on
    analogSetPinAttenuation(VBAT_PIN, ADC_11db);  // 0 to ~3.1 V; a full LiPo reads ~857 mV
}

uint32_t ReadBatteryMv(){
    return analogReadMilliVolts(VBAT_PIN) * 49 / 10;   // undo the 390k/100k divider (x4.9)
}

bool RadioBegin(){
    SPI.begin(RADIO_SCK, RADIO_MISO, RADIO_MOSI, RADIO_CS);
    int16_t status = radio.begin(mapds::kFreqMHz, mapds::kBandwidthKHz, mapds::kSpreadingFactor, mapds::kCodingRate,
                               mapds::kSyncWord, kTxPowerDbm, mapds::kPreamble, kTcxoVolts);
    if (status != RADIOLIB_ERR_NONE) {
        Serial.printf("Radio init failed (%d)\n", status);  // look the code up in RadioLib's status codes
        return false;
    }
    radio.startReceive();   // DDS listens by default; SendCommand() switches to TX briefly
    return true;
}

/** Sends a bare-Header command (WAKE/SLEEP/RESTART), then goes back to listening. Returns false if it didn't go out. */
bool Transmit(mapds::Header message){
    if (!radioUp) { return false; }
    int16_t radio_status = radio.transmit(reinterpret_cast<uint8_t*>(&message), sizeof(message));
    radio.startReceive();   // transmit() leaves the radio idle (and raises DIO1); listen again for telemetry
    if (radio_status != RADIOLIB_ERR_NONE) {
        Serial.printf("TX failed (%d)\n", radio_status);
        return false;
    }
    return true;
}

/**
 * Checks the radio for a TELEMETRY frame from our DCM. If one arrived, stores it in
 * the dcm* globals, tLastPacket, lastRssi and lastSnr, and returns its seq (0-65535).
 * Returns -1 for nothing, junk, or someone else's packet.
 * Only the first call after a packet arrives sees it: reading clears it.
 */
int32_t ReadPacket(){
    if (!radioUp) { return -1; }
    if (digitalRead(RADIO_DIO1) == LOW) { return -1; }   // DIO1 high = a packet is waiting

    size_t len = radio.getPacketLength();                // before readData(), which clears the packet
    uint8_t buf[sizeof(mapds::TelemetryFrame)];
    int16_t st = radio.readData(buf, sizeof(buf));       // always read: caps at sizeof(buf), clears DIO1
    float rssi = radio.getRSSI();                        // both describe the packet just read
    float snr  = radio.getSNR();
    radio.startReceive();
    if (st != RADIOLIB_ERR_NONE) { return -1; }          // e.g. CRC error: corrupted in the air
    if (len != sizeof(mapds::TelemetryFrame)) { return -1; }

    mapds::TelemetryFrame f;
    memcpy(&f, buf, sizeof(f));
    if (f.header.msg != mapds::kMsgTelemetry) { return -1; }
    if (f.header.pairId != mapds::kPairId) { return -1; }   // not our DCM

    dcmPmuOk       = f.pmuOk;
    dcmGnssOk      = f.gnssOk;
    dcmImuOk       = f.imuOk;
    dcmMagOk       = f.magOk;
    dcmBmeOk       = f.bmeOk;
    dcmLatDeg      = f.latDeg;
    dcmLonDeg      = f.lonDeg;
    dcmGpsAltM     = f.gpsAltM;
    dcmHdop        = f.hdop;
    dcmPressurePa  = f.pressurePa;
    dcmBattPct     = f.battPct;
    dcmTempC       = f.tempC;
    dcmHumidityPct = f.humidityPct;
    for (int i = 0; i < 3; i++) {
        dcmAccelMps2[i] = f.accelMps2[i];
        dcmGyroDps[i]   = f.gyroDps[i];
        dcmMagGauss[i]  = f.magGauss[i];
    }
    tLastPacket = millis();
    lastRssi    = rssi;
    lastSnr     = snr;
    return f.header.seq;
}
void setup() {
    PowerOn();
    Serial.begin(115200);
    radioUp = RadioBegin();
}

void loop() {
    ReadPacket();   // keeps the dcm* globals fresh in every state

    switch(state){
        case LOAD:           Load();          break;
        case STANDBY:        Standby();       break;
        case NOT_LOADED:     NotLoaded();     break;
        case ARMED:          Armed();         break;
        case SOFTWARE_ERROR: SoftwareError(); break;
        case DEPLOYING:      Deploying();     break;
        case DEPLOY_ERROR:   DeployError();   break;
        case DEPLOYED:       Deployed();      break;
        case DEPLOY_FAILURE: DeployFailure(); break;
    }
}
