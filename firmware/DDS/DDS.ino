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
 *   LOAD, STANDBY, ARMED, SOFTWARE_ERROR,
 *   DEPLOYING, DEPLOYED, DEPLOY_ERROR, DEPLOY_FAILURE
 *
 *   Boot in STANDBY. FSM v3's NOT_LOADED is cut: STANDBY with the limit switch OPEN
 *   covers it (no arming without a drone).
 *
 * ---------------------------------------------------------------- INPUTS
 *   Switches (levels: stay on until turned off)  Load, Arm, Deploy
 *   Commands (one-shot: cleared on every state change)  END/SLEEP, ACKNOWLEDGE FAILURE
 *   Limit switch                                  CLOSED = drone present, OPEN = gone
 *   Software fault                                computed from telemetry (below)
 *
 *   END and ACK turn all three switches off. Otherwise a still-on Arm and
 *   Deploy would re-arm and re-fire straight out of Standby.
 *   Deploy only counts if it is turned on after the Armed light: ARMED turns it off until then.
 *   Clearing END/ACK on every state change means one pressed in another state can't fire later.
 *
 * ---------------------------------------------------------------- FSM
 *   STANDBY ------(Load on)---------> LOAD
 *   LOAD ---------(Load off)--------> STANDBY
 *   STANDBY ------(Arm on, limit CLOSED, radio up)> ARMED   [send WAKE, start 15 s timer]
 *   ARMED --------(Arm off)---------> STANDBY          [send SLEEP]
 *   ARMED --------(software fault)--> SOFTWARE_ERROR   [send RESTART, start 1 min timer]
 *   ARMED --------(Deploy on, 15 s up)> DEPLOYING      [start 10 s timer]
 *   SOFTWARE_ERROR (1 min up)-------> STANDBY          [send SLEEP] (STANDBY re-arms if Arm is on and limit CLOSED)
 *   SOFTWARE_ERROR (Arm off)--------> STANDBY          [send SLEEP]
 *   DEPLOYING ----(limit OPEN)------> DEPLOYED
 *   DEPLOYING ----(10 s, still CLOSED)> DEPLOY_ERROR   [jiggle the servo 3 times]
 *   DEPLOY_ERROR -(limit OPEN)------> DEPLOYED
 *   DEPLOY_ERROR -(jiggles done, still CLOSED)> DEPLOY_FAILURE
 *   DEPLOYED -----(END/SLEEP)-------> STANDBY          [send SLEEP, switches off]
 *   DEPLOY_FAILURE (ACK)------------> STANDBY          [send SLEEP, switches off]
 *
 *   Ignore any input that has no arrow out of the current state (e.g. Deploy in STANDBY).
 *   DEPLOYED -> STANDBY lands with the limit switch OPEN, so Arm is refused until a drone is loaded.
 *   Every timer is "time since the transition that started it": save millis() once, on the arrow.
 *
 * ---------------------------------------------------------------- INDICATORS
 *   Armed light    ARMED and the 15 s timer is up (that is also when Deploy is allowed)
 *   Error light    SOFTWARE_ERROR, DEPLOY_ERROR, DEPLOY_FAILURE
 *   Status         DEPLOYED = success, DEPLOY_FAILURE = failure
 *   DDS battery    ddsBatteryMv, the Heltec's own LiPo
 *   Warnings       no GPS fix (gnssOk false), DCM battery < 20 % (battPct -1 = on USB, no warning)
 *                  Shown only. They never cause a transition.
 *
 * ---------------------------------------------------------------- SOFTWARE FAULT (checked in ARMED only, once the 15 s are up)
 *   Any one of these is a fault:
 *   1. No TELEMETRY for 10 s. With nothing since the WAKE, this fires as the 15 s end.
 *   2. More than 3 missed packets in a 10 s window. Missed = a frame whose seq isn't lastSeq + 1.
 *      The DCM restarts seq at 0 every boot, so a reboot counts as one miss.
 *   3. Any of pmuOk, imuOk, magOk, bmeOk false.
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
 *   Limit switch         GPIO 40, to GND with INPUT_PULLUP: LOW = CLOSED. Debounced 20 ms (ReadLimitSwitch).
 *   Servo bus (Serial1)  TX 38, RX 39, 1000000 baud, servo ID 1
 *                        via DFRobot DRI0057 driver board: Heltec TX -> board RX, Heltec RX -> board TX,
 *                        common GND, 12 V into the board's DC jack (must match the servo's 12 V).
 *                        Board's two shorting caps: set them for UART, not USB (check the silkscreen).
 *                        Check the board's TX idle voltage before wiring: the ESP32-S3 is 3.3 V only.
 *
 *   Gotchas:
 *   - transmit() also fires DIO1. Clear the received flag after sending, and startReceive() again (see bench/heltec_rx).
 *   - Don't use delay() for the FSM timers. The radio and the switch must keep being read.
 *     The jiggle doesn't block either: the time spent in DEPLOY_ERROR picks the move.
 *
 * ---------------------------------------------------------------- BUILD
 *   Board:      "Heltec WiFi LoRa 32(V3)" (esp32 core 3.x)
 *   Libraries:  RadioLib, SCServo (FT&WS, Library Manager)
 */

#include <SPI.h>
#include <RadioLib.h>   // SX1262
#include <SCServo.h>    // Feetech ST3215 bus servo (SMS_STS class)
#include <WiFi.h>
#include <WebServer.h> // bundled with the ESP32 board package
#include "web_assets.h"

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

constexpr int LIMIT_PIN  = 40;   // drone presence limit switch
constexpr int SERVO_TX   = 38;   // Heltec TX -> DRI0057 RX
constexpr int SERVO_RX   = 39;   // Heltec RX <- DRI0057 TX

// BOARD SETTINGS

constexpr int      kAdcCtrlOn   = HIGH;   // V3.4: HIGH confirmed on the bench; V3.0/V3.1 would be LOW
constexpr int8_t   kTxPowerDbm  = 14;     // bench value; flight value TBD
constexpr float    kTcxoVolts   = 1.8f;
constexpr uint32_t kLimitDebounceMs = 20; // switch must hold a level this long before it counts

// SERVO SETTINGS

constexpr uint32_t kServoBaud   = 1000000;
constexpr uint8_t  kServoId     = 1;      // factory default

// Servo positions: 0-4095 = 360 deg, 2047 = centre.
// VALUES SUBJECT TO CHANGE 
constexpr int16_t  LOAD_POS     = 0;   // hook open, drone can be hooked on
constexpr int16_t  LOCK_POS     = 2047;   // hook closed, drone held
constexpr int16_t  RELEASE_POS  = 0;   // hook open, drone falls free
constexpr uint16_t kServoSpeed  = 1500;   // steps/s (50 ~ 0.73 RPM, max ~3073)
constexpr uint8_t  kServoAcc    = 50;     // 0-150, lower = gentler (unit undocumented)
constexpr int16_t  kJiggleDistance = 100;   // distance each side of RELEASE_POS, TBD
constexpr int      kJiggleTimes  = 3;
constexpr uint32_t kJiggleStepMs = 200;   // time allowed per move before the next one, TBD

// END SERVO SETTINGS

// FSM TIMING

constexpr uint32_t kArmDelayMs       = 15000;   // ARMED: no faults or Deploy until this is up, then the Armed light
constexpr uint32_t kNoPacketMs       = 10000;   // ARMED: no telemetry for this long = fault
constexpr uint32_t kMissWindowMs     = 10000;   // ARMED: missCounter restarts every window
constexpr int      kMaxMissed        = 3;       // ARMED: more than this many misses in one window = fault
constexpr uint32_t kSoftwareErrorMs  = 60000;   // SOFTWARE_ERROR: wait this long, then STANDBY
constexpr uint32_t kDeployTimeoutMs  = 10000;   // DEPLOYING: limit still CLOSED after this = DEPLOY_ERROR

enum State  {
    LOAD,
    STANDBY,
    ARMED,
    SOFTWARE_ERROR,
    DEPLOYING,
    DEPLOY_ERROR,
    DEPLOYED,
    DEPLOY_FAILURE
};

// Commands to the DCM. The values are the protocol's message IDs, so Transmit() can put them straight in the header.
enum class Command : uint8_t {
    WAKE    = mapds::kMsgWake,
    SLEEP   = mapds::kMsgSleep,
    RESTART = mapds::kMsgRestart,
    CHECK_BATTERY = mapds::kMsgCheckBattery
};

State state = STANDBY;
uint32_t TimeStateEntered;
SX1262 radio = new Module(RADIO_CS, RADIO_DIO1, RADIO_RST, RADIO_BUSY);
SMS_STS servo;               // servo object
bool servoUp = false;
bool     limitRaw      = false;   // last raw switch reading, before debounce
uint32_t tLimitChanged = 0;       // millis() when limitRaw last changed
bool radioUp = false;        // RadioBegin() succeeded; if false, never arm (no telemetry possible)
uint16_t cmdSeq = 0;         // command counter (header.seq on WAKE/SLEEP/RESTART)
uint16_t lastSeq = 0;        // seq of the last valid frame, set by ReadPacket()
int missCounter = 0;         // gaps in seq, counted by ReadPacket()
uint32_t tMissWindow = 0;    // millis() when the missCounter window started




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
uint16_t dcmBattMv      = 0;       // battery voltage from the current protocol, 0 = unavailable
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
bool ArmedStatus = false;       // Armed light: ARMED and the 15 s are up (Deploy allowed)
bool ErrorStatus = false;       // Error light: SOFTWARE_ERROR, DEPLOY_ERROR, DEPLOY_FAILURE
bool DeployStatus = false;      // status: DEPLOYED (success)
bool DeployFailStatus = false;  // status: DEPLOY_FAILURE

uint32_t ddsBatteryMv = 0;      // DDS LiPo voltage, read every loop

bool LimitClosed = false;     // true = drone present. Debounced, updated by ReadLimitSwitch() every loop

// END: GLOBALS USED IN INTERFACE



// BEGIN: Web connection settings.
constexpr char kDdsWifiSsid[] = "DDI_MAPDS";
constexpr char kDdsWifiPassword[] = "12345678";
WebServer cockpitServer(80);
bool webServerUp = false, webHasTelemetry = false, webResetPending = false;
uint32_t tWebResetRequested = 0;
// END: Web connection settings.





/** Name of a state, for serial prints. */
const char* StateName(State s){
    switch(s){
        case LOAD:           return "LOAD";
        case STANDBY:        return "STANDBY";
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
    EndCommand = AckCommand = false;   // one-shot commands only count in the state they're pressed in
}

bool StateChange(State to){
    State from = state;
    switch(from){
        case STANDBY:
            if(to == LOAD)       { UpdateState(to); Serial.println("  [servo -> LOAD_POS]"); return true; }
            if(to == ARMED)    { UpdateState(to); Serial.println("  [send WAKE]"); Transmit(Command::WAKE); return true; }
            break;
        case LOAD:
            if(to == STANDBY)    { UpdateState(to); Serial.println("  [servo -> LOCK_POS]"); return true; }
            break;
        case ARMED:
            if(to == STANDBY)        { UpdateState(to); Serial.println("  [send SLEEP]"); Transmit(Command::SLEEP); return true; }
            if(to == SOFTWARE_ERROR) { UpdateState(to); Serial.println("  [send RESTART]"); Transmit(Command::RESTART); return true; }
            if(to == DEPLOYING)      { UpdateState(to); Serial.println("  [servo -> RELEASE_POS]"); return true; }
            break;
        case SOFTWARE_ERROR:
            if(to == STANDBY)    { UpdateState(to); Serial.println("  [send SLEEP]"); Transmit(Command::SLEEP); return true; }
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
            if(to == STANDBY)    { UpdateState(to); Serial.println("  [send SLEEP, switches off]"); Transmit(Command::SLEEP); return true; }
            break;
        case DEPLOY_FAILURE:
            if(to == STANDBY)    { UpdateState(to); Serial.println("  [send SLEEP, switches off]"); Transmit(Command::SLEEP); return true; }
            break;
    }
    Serial.printf("No arrow %s -> %s, ignored\n", StateName(from), StateName(to));
    return false;
}

// STATES
void Load(){
    servo.WritePosEx(kServoId, LOAD_POS, kServoSpeed, kServoAcc);
    if (!LoadCommand && LimitClosed) StateChange(STANDBY);
}

void Standby(){
    servo.WritePosEx(kServoId, LOCK_POS, kServoSpeed, kServoAcc);

    if (LoadCommand) StateChange(LOAD);

    else if (ArmCommand && LimitClosed && radioUp) StateChange(ARMED);   // no radio = no telemetry: never arm

}

void Armed(){
    // exits: Arm off -> STANDBY, software fault -> SOFTWARE_ERROR, Deploy on and 15 s up -> DEPLOYING
    bool softwareError = false;
    if (!ArmCommand){
        ArmedStatus = false;
        StateChange(STANDBY);
        return;
    }

    if (millis() - tMissWindow >= kMissWindowMs) {
        missCounter = 0;
        tMissWindow = millis();
    }
    if (millis() - TimeStateEntered < kArmDelayMs) {
        DeployCommand = false;   // Deploy only counts once the Armed light is on
        return;
    }
    ArmedStatus = true;
    if (millis() - tLastPacket > kNoPacketMs) softwareError = true;
    if (missCounter > kMaxMissed) softwareError = true;
    if (!dcmPmuOk || !dcmImuOk || !dcmMagOk || !dcmBmeOk) softwareError = true;

    if (softwareError){
        ArmedStatus = false;
        StateChange(SOFTWARE_ERROR);
        return;
    }
    if (DeployCommand) {
        ArmedStatus = false;
        StateChange(DEPLOYING);
        return;
    }
}

void SoftwareError(){
    // exits: Arm off -> STANDBY, 1 min up -> STANDBY
    ErrorStatus = true;
    if (!ArmCommand || millis() - TimeStateEntered >= kSoftwareErrorMs){
        ErrorStatus = false;
        StateChange(STANDBY);
    }
}

void Deploying(){
    // exits: limit OPEN -> DEPLOYED, 10 s up and still CLOSED -> DEPLOY_ERROR
    servo.WritePosEx(kServoId, RELEASE_POS, kServoSpeed, kServoAcc);
    if (!LimitClosed) { StateChange(DEPLOYED); return; }
    if (millis() - TimeStateEntered < kDeployTimeoutMs) return;

    StateChange(DEPLOY_ERROR);
}

void DeployError(){
    // exits: limit OPEN -> DEPLOYED, still CLOSED -> DEPLOY_FAILURE
    ErrorStatus = true;
    if (!LimitClosed) { ErrorStatus = false; StateChange(DEPLOYED); return; }

    // One move per kJiggleStepMs: +, -, +, - ... (kJiggleTimes pairs), then one step back at RELEASE_POS.
    int step = (millis() - TimeStateEntered) / kJiggleStepMs;
    if (step < 2 * kJiggleTimes) {
        int16_t pos = (step % 2 == 0) ? RELEASE_POS + kJiggleDistance : RELEASE_POS - kJiggleDistance;
        servo.WritePosEx(kServoId, pos, kServoSpeed, kServoAcc);
    }
    else if (step == 2 * kJiggleTimes) servo.WritePosEx(kServoId, RELEASE_POS, kServoSpeed, kServoAcc);
    else StateChange(DEPLOY_FAILURE);
}

void Deployed(){
    // exits: END/SLEEP -> STANDBY
    DeployStatus = true;
    if (EndCommand) {                                        // cleared by UpdateState()
        LoadCommand = ArmCommand = DeployCommand = false;    // switches off, so STANDBY doesn't re-arm and re-fire
        DeployStatus = false;
        StateChange(STANDBY);
    }
}

void DeployFailure(){
    // exits: ACKNOWLEDGE FAILURE -> STANDBY
    DeployFailStatus = true;
    ErrorStatus = true;
    if (AckCommand) {                                        // cleared by UpdateState()
        LoadCommand = ArmCommand = DeployCommand = false;    // switches off, so STANDBY doesn't re-arm and re-fire
        DeployFailStatus = false;
        ErrorStatus = false;
        StateChange(STANDBY);
    }
}

// END STATES

void PowerOn(){
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);                  // power light

    pinMode(ADC_CTRL, OUTPUT);
    digitalWrite(ADC_CTRL, kAdcCtrlOn);           // battery divider on, and left on
    analogSetPinAttenuation(VBAT_PIN, ADC_11db);  // 0 to ~3.1 V; a full LiPo reads ~857 mV

    pinMode(LIMIT_PIN, INPUT_PULLUP);             // switch to GND: LOW = CLOSED
    LimitClosed = digitalRead(LIMIT_PIN) == LOW;  // start from the real level, so boot doesn't see a false OPEN
    limitRaw    = LimitClosed;
}

uint32_t ReadBatteryMv(){
    return analogReadMilliVolts(VBAT_PIN) * 49 / 10;   // undo the 390k/100k divider (x4.9)
}

/** Debounced read of the limit switch into LimitClosed. Call every loop. */
void ReadLimitSwitch(){
    bool raw = digitalRead(LIMIT_PIN) == LOW;          // LOW = CLOSED = drone present
    if (raw != limitRaw) {                             // changed (or still bouncing): restart the clock
        limitRaw = raw;
        tLimitChanged = millis();
    }
    if (millis() - tLimitChanged >= kLimitDebounceMs) {
        LimitClosed = limitRaw;                        // held steady long enough: accept it
    }
}

bool RadioBegin(){
    SPI.begin(RADIO_SCK, RADIO_MISO, RADIO_MOSI, RADIO_CS);
    int16_t status = radio.begin(mapds::kFreqMHz, mapds::kBandwidthKHz, mapds::kSpreadingFactor, mapds::kCodingRate,
                               mapds::kSyncWord, kTxPowerDbm, mapds::kPreamble, kTcxoVolts);
    if (status != RADIOLIB_ERR_NONE) {
        Serial.printf("Radio init failed (%d)\n", status);  // look the code up in RadioLib's status codes
        return false;
    }
    radio.startReceive();   // DDS listens by default; Transmit() switches to TX briefly
    return true;
}

bool ServoBegin(){
    Serial1.begin(kServoBaud, SERIAL_8N1, SERVO_RX, SERVO_TX);
    servo.pSerial = &Serial1;
    int pingResult = servo.Ping(kServoId);
    if (pingResult == -1) {
        Serial.printf("Servo %d not answering (check 12 V, wiring, UART mode)\n", kServoId);
        return false;
    }
    Serial.printf("Servo %d OK\n", kServoId);
    return true;
}

/** Sends a command to the DCM (as a bare Header, next cmdSeq), then goes back to listening. Returns false if it didn't go out. */
bool Transmit(Command cmd){
    if (!radioUp) { return false; }
    mapds::Header message = mapds::buildHeader(static_cast<uint8_t>(cmd), cmdSeq++);
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
    dcmBattMv      = f.battMv;
    dcmTempC       = f.tempC;
    dcmHumidityPct = f.humidityPct;
    for (int i = 0; i < 3; i++) {
        dcmAccelMps2[i] = f.accelMps2[i];
        dcmGyroDps[i]   = f.gyroDps[i];
        dcmMagGauss[i]  = f.magGauss[i];
    }
    if (f.header.seq != lastSeq + 1) missCounter++;
    lastSeq = f.header.seq;
    tLastPacket = millis();
    lastRssi    = rssi;
    lastSnr     = snr;
    return f.header.seq;
}
// -------------------------- WEB VARIABLE BRIDGE --------------------------

// Single-cell 3.7 V LiPo indication, interpolated between voltage points.
// Tune these bridge-only points against the installed batteries after measurements.
// Cell voltage range reference: learn.adafruit.com/li-ion-and-lipoly-batteries/voltages
int WebBatteryPercent(uint32_t mv){
    if (mv < 2500 || mv > 4350) return -1; // no usable battery reading
    static const uint16_t volts[] = {3000,3300,3500,3600,3700,3750,3800,3850,3900,3950,4000,4100,4200};
    static const uint8_t percent[] = {0,2,5,10,20,30,40,50,60,70,80,90,100};
    if (mv <= volts[0]) return 0;
    for (unsigned int i = 1; i < sizeof(volts)/sizeof(volts[0]); ++i) {
        if (mv <= volts[i]) return percent[i-1] + ((mv-volts[i-1])*(percent[i]-percent[i-1]) + (volts[i]-volts[i-1])/2)/(volts[i]-volts[i-1]);
    }
    return 100;
}

void WebFlag(String& json, const char* key, bool value){
    json += ",\""; json += key; json += "\":"; json += value ? "true" : "false";
}
void WebNumber(String& json, const char* key, double value, unsigned int digits){
    json += ",\""; json += key; json += "\":";
    json += isfinite(value) ? String(value, digits) : String("null");
}
void WebVector(String& json, const char* key, const float values[3]){
    json += ",\""; json += key; json += "\":[";
    for (int i = 0; i < 3; ++i) { if (i) json += ','; json += isfinite(values[i]) ? String(values[i], 3) : String("null"); }
    json += ']';
}
void CockpitStatus(){
    String json; json.reserve(2400);
    json = "{\"state\":\""; json += StateName(state); json += "\"";
    WebFlag(json, "dcmPresent", LimitClosed); WebFlag(json, "servoUp", servoUp); WebFlag(json, "radioUp", radioUp);
    WebNumber(json, "ddsBatteryPct", WebBatteryPercent(ddsBatteryMv), 0);
    WebNumber(json, "ddsBatteryMv", ddsBatteryMv, 0); WebNumber(json, "stateAgeMs", millis() - TimeStateEntered, 0);
    WebNumber(json, "missCounter", missCounter, 0);
    json += ",\"limits\":{\"armDelayMs\":"; json += String(kArmDelayMs);
    WebNumber(json, "noPacketMs", kNoPacketMs, 0); WebNumber(json, "maxMissed", kMaxMissed, 0);
    WebNumber(json, "softwareErrorMs", kSoftwareErrorMs, 0); json += '}';
    json += ",\"inputs\":{\"LoadCommand\":"; json += LoadCommand ? "true" : "false";
    WebFlag(json, "ArmCommand", ArmCommand); WebFlag(json, "DeployCommand", DeployCommand); json += '}';
    json += ",\"indicators\":{\"ArmedStatus\":"; json += ArmedStatus ? "true" : "false";
    WebFlag(json, "ErrorStatus", ErrorStatus); WebFlag(json, "DeployStatus", DeployStatus); WebFlag(json, "DeployFailStatus", DeployFailStatus); json += '}';
    json += ",\"telemetry\":{\"hasTelemetry\":"; json += webHasTelemetry ? "true" : "false";
    WebNumber(json, "ageMs", millis() - tLastPacket, 0); WebNumber(json, "lastTelemetrySeq", lastSeq, 0);
    WebFlag(json, "dcmPmuOk", dcmPmuOk); WebFlag(json, "dcmGnssOk", dcmGnssOk);
    WebFlag(json, "dcmImuOk", dcmImuOk); WebFlag(json, "dcmMagOk", dcmMagOk); WebFlag(json, "dcmBmeOk", dcmBmeOk);
    WebNumber(json, "dcmLatDeg", dcmLatDeg, 7); WebNumber(json, "dcmLonDeg", dcmLonDeg, 7);
    WebNumber(json, "dcmGpsAltM", dcmGpsAltM, 1); WebNumber(json, "dcmHdop", dcmHdop, 2);
    WebNumber(json, "dcmBattMv", dcmBattMv, 0); WebNumber(json, "dcmBattPct", webHasTelemetry && dcmPmuOk ? WebBatteryPercent(dcmBattMv) : -1, 0); WebNumber(json, "dcmTempC", dcmTempC, 1);
    WebNumber(json, "dcmHumidityPct", dcmHumidityPct, 1); WebNumber(json, "dcmPressurePa", dcmPressurePa, 1);
    WebVector(json, "dcmAccelMps2", dcmAccelMps2); WebVector(json, "dcmGyroDps", dcmGyroDps); WebVector(json, "dcmMagGauss", dcmMagGauss);
    WebNumber(json, "lastRssi", lastRssi, 1); WebNumber(json, "lastSnr", lastSnr, 1); json += "}}";
    cockpitServer.sendHeader("Cache-Control", "no-store"); cockpitServer.send(200, "application/json", json);
}
void CockpitCommand(){
    const String command = cockpitServer.arg("command"), value = cockpitServer.arg("value");
    bool* input = command == "LOAD" ? &LoadCommand : command == "ARM" ? &ArmCommand : command == "DEPLOY" ? &DeployCommand : nullptr;
    if (input && (value == "true" || value == "false")) *input = value == "true";
    else if (command == "CHECK_BATTERY") {
        const bool request = state == STANDBY || state == LOAD;
        if (request && !Transmit(Command::CHECK_BATTERY)) { cockpitServer.send(503, "application/json", "{\"ok\":false,\"message\":\"Battery request could not be transmitted.\"}"); return; }
        cockpitServer.send(202, "application/json", request ? "{\"ok\":true,\"requested\":true}" : "{\"ok\":true,\"requested\":false}"); return;
    }
    else if (command == "END") EndCommand = true; // existing Deployed() handles it
    else if (command == "ACK") AckCommand = true; // existing DeployFailure() handles it
    else { cockpitServer.send(400, "application/json", "{\"ok\":false,\"message\":\"Unknown command or missing ON/OFF value.\"}"); return; }
    cockpitServer.send(202, "application/json", "{\"ok\":true}");
}
void CockpitReset(){
    if (cockpitServer.arg("confirmed") != "true") { cockpitServer.send(400, "application/json", "{\"ok\":false}"); return; }
    cockpitServer.send(202, "application/json", "{\"ok\":true}");
    tWebResetRequested = millis(); webResetPending = true;
}
void StartCockpitServer(){
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(kDdsWifiSsid, kDdsWifiPassword)) { Serial.println("DDS Wi-Fi failed to start."); return; }
    cockpitServer.on("/", HTTP_GET, [](){ cockpitServer.sendHeader("Cache-Control", "no-store"); cockpitServer.send_P(200, "text/html; charset=utf-8", WEB_MAIN_HTML); });
    cockpitServer.on("/main.html", HTTP_GET, [](){ cockpitServer.sendHeader("Cache-Control", "no-store"); cockpitServer.send_P(200, "text/html; charset=utf-8", WEB_MAIN_HTML); });
    cockpitServer.on("/cockpit_style.css", HTTP_GET, [](){ cockpitServer.sendHeader("Cache-Control", "no-store"); cockpitServer.send_P(200, "text/css; charset=utf-8", WEB_COCKPIT_CSS); });
    cockpitServer.on("/interface_logic.js", HTTP_GET, [](){ cockpitServer.sendHeader("Cache-Control", "no-store"); cockpitServer.send_P(200, "application/javascript; charset=utf-8", WEB_INTERFACE_JS); });
    cockpitServer.on("/api/status", HTTP_GET, CockpitStatus);
    cockpitServer.on("/api/command", HTTP_POST, CockpitCommand);
    cockpitServer.on("/api/reset", HTTP_POST, CockpitReset);
    cockpitServer.enableDelay(false); cockpitServer.begin(); webServerUp = true;
    Serial.print("DDS Wi-Fi: "); Serial.println(kDdsWifiSsid);
    Serial.print("Cockpit address: http://"); Serial.println(WiFi.softAPIP());
}
void ServiceCockpit(){
    if (tLastPacket != 0) webHasTelemetry = true; 
    if (webResetPending) { if (millis() - tWebResetRequested >= 750) ESP.restart(); return; }
    // Regular WebServer still has synchronous socket waits. Defer it during the timed jiggle.
    if (webServerUp && state != DEPLOY_ERROR) cockpitServer.handleClient();
}
// ------------------------ END WEB VARIABLE BRIDGE ------------------------

void setup() {
    PowerOn();
    Serial.begin(115200);
    radioUp = RadioBegin();
    servoUp = ServoBegin();
    StartCockpitServer();
}

void loop() {
    ReadPacket();   // keeps the dcm* globals fresh in every state
    ReadLimitSwitch();    // keeps LimitClosed fresh in every state
    ddsBatteryMv = ReadBatteryMv();

    switch(state){
        case LOAD:           Load();          break;
        case STANDBY:        Standby();       break;
        case ARMED:          Armed();         break;
        case SOFTWARE_ERROR: SoftwareError(); break;
        case DEPLOYING:      Deploying();     break;
        case DEPLOY_ERROR:   DeployError();   break;
        case DEPLOYED:       Deployed();      break;
        case DEPLOY_FAILURE: DeployFailure(); break;
    }
    ServiceCockpit();
}