/**
 * DDS.ino — Drone Deployment System firmware (Heltec WiFi LoRa 32 V3)
 *
 * Holds the drone on the helicopter, releases it on command, and watches the
 * DCM's telemetry to decide whether the drone is healthy enough to deploy.
 * Design: boeing fsm v3(1).png at the repo root.
 *
 * The Heltec hosts its own Wi-Fi and cockpit page. LOAD requests enter the
 * tested loading FSM. ARM checks, deployment, recovery, and operator exits run on DDS.
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
 *   Ready display  ARMED, 15 s elapsed, fresh telemetry, and all mechanical/software checks passed
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
 *   Jiggle: 3 small inward/return cycles above RELEASE_POS (100 counts, clamped to 0-4095), TBD.
 *
 * ---------------------------------------------------------------- SERIAL (115200)
 *   Every transition prints "FROM -> TO" and its action. The temporary bench-test
 *   section near setup() accepts L/l to request LOAD; it uses the real limit switch.
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
 *   Libraries:  RadioLib, SCServo (FT&WS). WiFi is bundled with the ESP32 board package.
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
constexpr int16_t  LOAD_POS     = 0;      // 0 degrees: loading opening open
constexpr int16_t  LOCK_POS     = 2048;   // 180 degrees: loading opening closed
constexpr int16_t  RELEASE_POS  = 0;      // 0 degrees: hook open, drone falls free
constexpr uint16_t kServoSpeed  = 2900;   // requested steps/s; actual speed depends on servo configuration and load
constexpr uint8_t  kServoAcc    = 50;     // 0-150, lower = gentler (unit undocumented)
constexpr int16_t  kJiggleDistance = 100;   // recovery excursion above RELEASE_POS, clamped to 0-4095; TBD
constexpr int      kJiggleTimes  = 3;
constexpr uint32_t kJiggleStepMs = 200;   // time allowed per move before the next one, TBD

// ARM / DEPLOY timing: milliseconds, always checked across loop passes.
constexpr uint32_t kArmReadyMs = 15000;
constexpr uint32_t kTelemetryFaultMs = 10000;
constexpr uint32_t kTelemetryFreshMs = 3500;
constexpr uint32_t kSoftwareRecoveryMs = 60000;
constexpr uint32_t kDeployTimeoutMs = 10000;
constexpr uint32_t kServoCheckTimeoutMs = 250;
constexpr uint32_t kServoQueueTimeoutMs = 500;
constexpr uint32_t kRadioTxTimeoutMs = 2000;

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
    RESTART = mapds::kMsgRestart
};

State state = STANDBY;
uint32_t TimeStateEntered;
SX1262 radio = new Module(RADIO_CS, RADIO_DIO1, RADIO_RST, RADIO_BUSY);
SMS_STS servo;               // servo object
bool servoUp = false;
bool servoMovePending = false; // latest requested target; never queue repeated positions
int16_t servoTarget = LOCK_POS;
bool loadPresenceTiming = false;
uint32_t tLoadPresenceStarted = 0;
bool     limitRaw      = false;   // last raw switch reading, before debounce
uint32_t tLimitChanged = 0;       // millis() when limitRaw last changed
bool radioUp = false;        // RadioBegin() succeeded; if false, the WAKE/software check fails
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
int8_t   dcmBattPct     = -1;      // 0-100, or -1 with no battery
float    dcmTempC       = 0;
float    dcmHumidityPct = 0;
float    dcmAccelMps2[3] = {};     // x, y, z
float    dcmGyroDps[3]   = {};
float    dcmMagGauss[3]  = {};
uint32_t tLastPacket = 0;    // millis() when the last valid frame arrived
bool hasTelemetry = false; // millis() == 0 is a valid timestamp, not a validity flag
uint16_t lastTelemetrySeq = 0;
float    lastRssi    = 0;    // dBm, of the last valid frame
float    lastSnr     = 0;    // dB

bool LoadCommand = false;   // one-shot LOAD request; consumed when entering LOAD
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

// Check results are cached by the loop; expanding the GUI never runs a test.
enum CheckResult : uint8_t { CHECK_NOT_RUN, CHECK_RUNNING, CHECK_PASS, CHECK_FAIL, CHECK_NO_DATA, CHECK_STOPPED };
CheckResult checkPresence = CHECK_NOT_RUN, checkLock = CHECK_NOT_RUN;
CheckResult checkServo = CHECK_NOT_RUN, checkWake = CHECK_NOT_RUN;
CheckResult checkTelemetry = CHECK_NOT_RUN, checkPacketLoss = CHECK_NOT_RUN;
CheckResult checkSensors[4] = {CHECK_NOT_RUN, CHECK_NOT_RUN, CHECK_NOT_RUN, CHECK_NOT_RUN};
CheckResult checkArmTimer = CHECK_NOT_RUN;
const char* faultReason = "";

// One outgoing radio slot; a newer state command replaces an unsent old command.
// No retries or new packet types. A TX result is not an acknowledgment from DCM.
bool radioTxPending = false, radioTransmitting = false;
mapds::Header queuedRadioCommand = {}, activeRadioCommand = {};
uint32_t tRadioTxStarted = 0;
CheckResult lastRadioTxResult = CHECK_NOT_RUN;
int16_t lastRadioTxError = 0;

bool armWakeSent = false, armTelemetrySeen = false;
uint32_t tArmWakeSent = 0, tArmLastTelemetry = 0;
uint16_t armPreviousSeq = 0;
uint32_t missedPacketTimes[4] = {};
uint8_t missedPacketCount = 0, missedPacketIndex = 0;
bool sensorBadTiming[4] = {};
uint32_t sensorBadSince[4] = {};

// Non-blocking servo PING transaction using the existing servo bus protocol.
// No additional library; response is six bytes: FF FF ID 02 STATUS CHECKSUM.
enum ServoPingPhase : uint8_t { PING_IDLE, PING_DRAIN, PING_WAIT };
ServoPingPhase servoPingPhase = PING_IDLE;
uint32_t tServoPingStarted = 0;
uint8_t servoPingReply[6] = {};
uint8_t servoPingReplySize = 0;
uint8_t jiggleStep = 0;
bool jiggleCommandIssued = false;
uint32_t tJiggleStep = 0;

const char* CheckName(CheckResult result){
    switch (result) {
        case CHECK_RUNNING: return "RUNNING";
        case CHECK_PASS: return "PASS";
        case CHECK_FAIL: return "FAIL";
        case CHECK_NO_DATA: return "NO_DATA";
        case CHECK_STOPPED: return "STOPPED";
        default: return "NOT_RUN";
    }
}

CheckResult MechanicalResult(){
    if (checkPresence == CHECK_FAIL || checkLock == CHECK_FAIL || checkServo == CHECK_FAIL) return CHECK_FAIL;
    if (checkPresence == CHECK_PASS && checkLock == CHECK_PASS && checkServo == CHECK_PASS) return CHECK_PASS;
    if (checkServo == CHECK_STOPPED) return CHECK_STOPPED;
    return checkServo == CHECK_NOT_RUN ? CHECK_NOT_RUN : CHECK_RUNNING;
}

CheckResult SoftwareResult(){
    if (checkWake == CHECK_NOT_RUN) return CHECK_NOT_RUN;
    if (checkWake == CHECK_FAIL || checkTelemetry == CHECK_FAIL || checkPacketLoss == CHECK_FAIL || checkArmTimer == CHECK_FAIL) return CHECK_FAIL;
    for (int i = 0; i < 4; ++i) if (checkSensors[i] == CHECK_FAIL) return CHECK_FAIL;
    if (checkWake == CHECK_STOPPED || checkTelemetry == CHECK_STOPPED || checkPacketLoss == CHECK_STOPPED || checkArmTimer == CHECK_STOPPED) return CHECK_STOPPED;
    for (int i = 0; i < 4; ++i) if (checkSensors[i] == CHECK_STOPPED) return CHECK_STOPPED;
    if (checkWake != CHECK_PASS || checkTelemetry != CHECK_PASS || checkPacketLoss != CHECK_PASS || checkArmTimer != CHECK_PASS) return CHECK_RUNNING;
    for (int i = 0; i < 4; ++i) if (checkSensors[i] != CHECK_PASS) return CHECK_RUNNING;
    return CHECK_PASS;
}

bool ArmPrerequisites(){
    return LimitClosed && limitRaw && servoTarget == LOCK_POS && !servoMovePending && !LoadCommand;
}

bool CanRequestArm(){
    return state == STANDBY && !ArmCommand && ArmPrerequisites() && !ddsResetPending;
}

bool CanRequestDeploy(){
    return state == ARMED && ArmCommand && !DeployCommand && ArmPrerequisites() &&
           armWakeSent && armTelemetrySeen && millis() - tArmLastTelemetry <= kTelemetryFreshMs &&
           MechanicalResult() == CHECK_PASS && SoftwareResult() == CHECK_PASS &&
           !radioTransmitting && !radioTxPending && !ddsResetPending;
}

const char* ArmBlockedReason(){
    if (ddsResetPending) return "DDS is restarting.";
    if (state != STANDBY) return "ARM requires STANDBY. Finish loading or end the current operation.";
    if (!LimitClosed || !limitRaw) return "No drone is attached. The presence switch must be closed.";
    if (servoTarget != LOCK_POS) return "The lock must be commanded closed before arming.";
    return "Wait for the loading movement or pending command to finish.";
}

const char* DeployBlockedReason(){
    if (state != ARMED || !ArmCommand) return "Arm DDS before requesting deployment.";
    if (!LimitClosed || !limitRaw) return "The drone is no longer attached.";
    if (servoTarget != LOCK_POS || servoMovePending) return "The lock must be closed and its command sent.";
    if (MechanicalResult() == CHECK_FAIL) return "Mechanical check failed. See its details, then disarm and retry.";
    if (MechanicalResult() != CHECK_PASS) return "Wait for the mechanical check to finish.";
    if (!armWakeSent) return "Waiting for the WAKE transmission to finish.";
    if (!armTelemetrySeen || millis() - tArmLastTelemetry > kTelemetryFreshMs) return "Waiting for current DCM telemetry.";
    if (SoftwareResult() == CHECK_FAIL) return "Software check failed. See its details.";
    if (millis() - tArmWakeSent < kArmReadyMs) return "The 15-second readiness period is still running.";
    return "Wait for all software checks and pending commands to finish.";
}

void ClearOperatorCommands(){
    LoadCommand = ArmCommand = DeployCommand = EndCommand = AckCommand = false;
}

void StopPendingChecks(){
    CheckResult* pending[] = {&checkServo, &checkWake, &checkTelemetry, &checkPacketLoss, &checkArmTimer,
                             &checkSensors[0], &checkSensors[1], &checkSensors[2], &checkSensors[3]};
    for (CheckResult* result : pending) if (*result == CHECK_RUNNING) *result = CHECK_STOPPED;
}

void BeginArmChecks(){
    checkPresence = LimitClosed ? CHECK_PASS : CHECK_FAIL;
    checkLock = servoTarget == LOCK_POS ? CHECK_PASS : CHECK_FAIL;
    checkServo = checkWake = checkTelemetry = checkArmTimer = CHECK_RUNNING;
    checkPacketLoss = CHECK_NO_DATA;
    for (int i = 0; i < 4; ++i) {
        checkSensors[i] = CHECK_NO_DATA;
        sensorBadTiming[i] = false;
    }
    armWakeSent = armTelemetrySeen = false;
    missedPacketCount = missedPacketIndex = 0;
    DeployCommand = false;
    faultReason = "";
    servoPingReplySize = 0;
    servoPingPhase = PING_DRAIN;
    tServoPingStarted = millis();
}

void ServiceServoCheck(){
    if (servoPingPhase == PING_IDLE) return;
    if (millis() - tServoPingStarted >= kServoCheckTimeoutMs) {
        checkServo = CHECK_FAIL;
        servoUp = false;
        servoPingPhase = PING_IDLE;
        return;
    }
    if (servoPingPhase == PING_DRAIN) {
        // Drain at most 24 stale bytes per pass; never spin waiting for input.
        for (int i = 0; i < 24 && Serial1.available() > 0; ++i) Serial1.read();
        if (Serial1.available() > 0 || Serial1.availableForWrite() < 6) return;
        uint8_t ping[] = {0xFF, 0xFF, kServoId, 2, 1, static_cast<uint8_t>(~(kServoId + 3))};
        Serial1.write(ping, sizeof(ping));
        servoPingPhase = PING_WAIT;
        return;
    }
    for (int i = 0; i < 24 && Serial1.available() > 0; ++i) {
        servoPingReply[servoPingReplySize++] = static_cast<uint8_t>(Serial1.read());
        if (servoPingReplySize < sizeof(servoPingReply)) continue;
        const uint8_t checksum = static_cast<uint8_t>(~(servoPingReply[2] + servoPingReply[3] + servoPingReply[4]));
        if (servoPingReply[0] == 0xFF && servoPingReply[1] == 0xFF &&
            servoPingReply[2] == kServoId && servoPingReply[3] == 2 && servoPingReply[5] == checksum) {
            // Communication success does not prove physical hook position.
            servoUp = servoPingReply[4] == 0;
            checkServo = servoUp ? CHECK_PASS : CHECK_FAIL;
            servoPingPhase = PING_IDLE;
            return;
        }
        memmove(servoPingReply, servoPingReply + 1, sizeof(servoPingReply) - 1);
        servoPingReplySize = sizeof(servoPingReply) - 1;
    }
}

uint8_t RecentMissedPackets(){
    uint8_t count = 0;
    for (int i = 0; i < missedPacketCount; ++i) {
        if (millis() - missedPacketTimes[i] < kTelemetryFaultMs) ++count;
    }
    return count; // capped at four: enough to detect more than three
}

bool TrackArmTelemetry(uint16_t sequence){
    if (state != ARMED || !armWakeSent) return true;
    if (armTelemetrySeen) {
        uint16_t delta = static_cast<uint16_t>(sequence - armPreviousSeq);
        if (delta == 0) return false; // duplicates must not keep the health timer alive
        // Normal uint16 rollover is forward. A backward jump/boot sequence zero
        // resynchronizes. The current wire format has no independent boot ID.
        if (delta >= 0x8000 || (sequence == 0 && armPreviousSeq != 65535)) delta = 1;
        const uint16_t missed = delta - 1;
        for (int i = 0; i < missed && i < 4; ++i) {
            missedPacketTimes[missedPacketIndex] = millis();
            missedPacketIndex = (missedPacketIndex + 1) % 4;
            if (missedPacketCount < 4) ++missedPacketCount;
        }
    }
    armPreviousSeq = sequence;
    tArmLastTelemetry = millis();
    armTelemetrySeen = true;
    return true;
}

void UpdateArmHealth(){
    if (!armWakeSent) return;
    const uint32_t now = millis();
    const uint32_t age = now - (armTelemetrySeen ? tArmLastTelemetry : tArmWakeSent);
    checkTelemetry = age >= kTelemetryFaultMs ? CHECK_FAIL :
        (armTelemetrySeen && age <= kTelemetryFreshMs ? CHECK_PASS : CHECK_RUNNING);
    const bool fresh = armTelemetrySeen && age <= kTelemetryFreshMs;
    checkPacketLoss = RecentMissedPackets() > 3 ? CHECK_FAIL : (fresh ? CHECK_PASS : CHECK_NO_DATA);
    checkArmTimer = now - tArmWakeSent >= kArmReadyMs ? CHECK_PASS : CHECK_RUNNING;
    if (!fresh) {
        for (int i = 0; i < 4; ++i) {
            if (checkSensors[i] != CHECK_FAIL) checkSensors[i] = CHECK_NO_DATA;
            sensorBadTiming[i] = false;
        }
        return;
    }
    const bool flags[] = {dcmPmuOk, dcmImuOk, dcmMagOk, dcmBmeOk};
    for (int i = 0; i < 4; ++i) {
        if (flags[i]) {
            sensorBadTiming[i] = false;
            checkSensors[i] = CHECK_PASS;
        } else {
            if (!sensorBadTiming[i]) { sensorBadTiming[i] = true; sensorBadSince[i] = now; }
            checkSensors[i] = now - sensorBadSince[i] >= kTelemetryFaultMs ? CHECK_FAIL : CHECK_RUNNING;
        }
    }
}

const char* SoftwareFaultReason(){
    if (checkWake == CHECK_FAIL) return "WAKE transmission failed. This is not a DCM acknowledgment.";
    if (checkTelemetry == CHECK_FAIL) return "No valid DCM telemetry for 10 seconds.";
    if (checkPacketLoss == CHECK_FAIL) return "More than three missed packets within 10 seconds.";
    for (int i = 0; i < 4; ++i) if (checkSensors[i] == CHECK_FAIL) return "A required DCM sensor reported failure for 10 seconds.";
    return "";
}


// Shared by the serial test and web adapter; only called by the firmware loop.
bool CanRequestLoad(){
    return state == STANDBY && servoUp && !servoMovePending && !LoadCommand;
}

bool RequestLoad(){
    if (!CanRequestLoad()) return false;
    LoadCommand = true;
    return true;
}




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
    return false;
}

// STATES
void Load(){
    // Start timing only after the opening command has left our pending slot.
    // A raw release resets immediately, including during debounce settling.
    if (servoMovePending || !LimitClosed || !limitRaw) {
        loadPresenceTiming = false;
        return;
    }
    const uint32_t now = millis();
    if (!loadPresenceTiming) {
        tLoadPresenceStarted = now;
        loadPresenceTiming = true;
    }
    if (now - tLoadPresenceStarted >= kLoadConfirmMs) {
        StateChange(STANDBY); // request closing; does not assert physical closure
    }
}

void Standby(){
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
    servo.IOTimeOut = kServoIoTimeoutMs;
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

void ServiceRadioCommands(){
    if (!radioUp) return;
    if (radioTransmitting) {
        const bool finished = digitalRead(RADIO_DIO1) == HIGH && (radio.getIrqFlags() & RADIOLIB_SX126X_IRQ_TX_DONE);
        const bool timedOut = millis() - tRadioTxStarted >= kRadioTxTimeoutMs;
        if (!finished && !timedOut) return;
        const int16_t finishResult = radio.finishTransmit();
        const int16_t receiveResult = radio.startReceive(); // clears TX IRQ before any RX read
        radioTransmitting = false;
        lastRadioTxResult = finished && finishResult == RADIOLIB_ERR_NONE && receiveResult == RADIOLIB_ERR_NONE ? CHECK_PASS : CHECK_FAIL;
        lastRadioTxError = timedOut ? RADIOLIB_ERR_TX_TIMEOUT : (finishResult != RADIOLIB_ERR_NONE ? finishResult : receiveResult);
        if (activeRadioCommand.msg == mapds::kMsgWake && state == ARMED) {
            checkWake = lastRadioTxResult;
            if (checkWake == CHECK_PASS) { armWakeSent = true; tArmWakeSent = millis(); }
        }
        if (receiveResult != RADIOLIB_ERR_NONE) radioUp = false;
    }
    if (!radioTxPending || !radioUp) return;
    activeRadioCommand = queuedRadioCommand;
    radioTxPending = false;
    const int16_t result = radio.startTransmit(reinterpret_cast<uint8_t*>(&activeRadioCommand), sizeof(activeRadioCommand));
    if (result != RADIOLIB_ERR_NONE) {
        lastRadioTxResult = CHECK_FAIL;
        lastRadioTxError = result;
        if (radio.startReceive() != RADIOLIB_ERR_NONE) radioUp = false;
        if (activeRadioCommand.msg == mapds::kMsgWake && state == ARMED) checkWake = CHECK_FAIL;
        return;
    }
    radioTransmitting = true;
    tRadioTxStarted = millis();
}

/**
 * Checks the radio for a TELEMETRY frame from our DCM. If one arrived, stores it in
 * the dcm* globals, tLastPacket, lastRssi and lastSnr, and returns its seq (0-65535).
 * Returns -1 for nothing, junk, or someone else's packet.
 * Only the first call after a packet arrives sees it: reading clears it.
 */
int32_t ReadPacket(){
    if (!radioUp || radioTransmitting) { return -1; }
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

    if (!TrackArmTelemetry(f.header.seq)) return -1;

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
    if (f.header.seq != lastSeq + 1) missCounter++;
    lastSeq = f.header.seq;
    tLastPacket = millis();
    hasTelemetry = true;
    lastTelemetrySeq = f.header.seq;
    lastRssi    = rssi;
    lastSnr     = snr;
    return f.header.seq;
}
// ==================== BEGIN TEMPORARY LOAD BENCH TEST ====================
// Set to 0 to disable. To remove completely, delete this block and the two
// matching #if DDS_LOAD_BENCH_TEST hook blocks in setup() and loop().
// Serial Monitor: 115200 baud, send L or l (any line-ending setting).
// This only requests LOAD through the normal FSM. No simulated switch,
// direct servo movement, ARM, or DEPLOY commands are provided here.
#define DDS_LOAD_BENCH_TEST 1

#if DDS_LOAD_BENCH_TEST
void BenchLoadBegin(){
    const char message[] = "[BENCH] Send L to open; hold real limit switch 1.5 s to close.\n";
    if (Serial.availableForWrite() >= static_cast<int>(sizeof(message) - 1)) {
        Serial.print(message);
    }
}

void BenchLoadInput(){
    // At most one already-buffered byte per loop: never wait for a full line.
    if (Serial.available() <= 0) return;
    const int key = Serial.read();
    if (key != 'L' && key != 'l') return;

    if (!RequestLoad()) {
        const char message[] = "[BENCH] LOAD ignored: needs STANDBY and ready servo.\n";
        if (Serial.availableForWrite() >= static_cast<int>(sizeof(message) - 1)) {
            Serial.print(message);
        }
        return;
    }
}
#endif
// ===================== END TEMPORARY LOAD BENCH TEST =====================

void StartDdsWifi(){
    // Startup initialization only; no delay or loop waiting for a client.
    if (!WiFi.mode(WIFI_AP) || !WiFi.softAP(kDdsWifiSsid, kDdsWifiPassword)) {
        Serial.println("DDS Wi-Fi startup failed; hardware loop continues.");
        return;
    }
    Serial.print("DDS Wi-Fi network: ");
    Serial.println(kDdsWifiSsid);
    Serial.print("DDS Wi-Fi address: ");
    Serial.println(WiFi.softAPIP());
}

// --------------------------- COCKPIT WEB SERVER ---------------------------
// Reads existing values only; no sensor queries, radio exchanges, or motion waits.
void WebTelemetryNumber(String& json, const char* name, double value, bool valid, unsigned int decimals){
    json += ",\"";
    json += name;
    json += "\":";
    json += valid && isfinite(value) ? String(value, decimals) : String("null");
}

void WebTelemetryFlag(String& json, const char* name, bool value, bool fresh){
    json += ",\"";
    json += name;
    json += "\":";
    json += !fresh ? "null" : (value ? "true" : "false");
}

void WebTelemetryVector(String& json, const char* name, const float values[3], bool valid){
    json += ",\"";
    json += name;
    json += "\":";
    if (!valid || !isfinite(values[0]) || !isfinite(values[1]) || !isfinite(values[2])) {
        json += "null";
        return;
    }
    json += '[';
    for (int i = 0; i < 3; ++i) {
        if (i) json += ',';
        json += String(values[i], 3);
    }
    json += ']';
}

void CockpitStatus(){
    const uint32_t now = millis();
    const bool fresh = hasTelemetry && now - tLastPacket <= kTelemetryFreshMs;
    const bool fix = fresh && dcmGnssOk;
    String json;
    json.reserve(2000);
    json += "{\"state\":\"";
    json += StateName(state);
    json += "\",\"lock\":\"";
    json += !servoUp ? "UNKNOWN" : (servoTarget == LOCK_POS ? "CLOSED_TARGET" : (servoTarget == LOAD_POS || servoTarget == RELEASE_POS ? "OPEN_TARGET" : "UNKNOWN"));
    json += "\",\"dcmPresent\":";
    json += LimitClosed ? "true" : "false";
    json += ",\"mechanical\":\"";
    json += CheckName(MechanicalResult());
    json += "\",\"software\":\"";
    json += CheckName(SoftwareResult());
    json += "\",\"loadAllowed\":";
    json += CanRequestLoad() ? "true" : "false";
    json += ",\"armAllowed\":";
    json += CanRequestArm() ? "true" : "false";
    json += ",\"deployAllowed\":";
    json += CanRequestDeploy() ? "true" : "false";
    json += ",\"armWaitMs\":";
    json += String(state == ARMED && armWakeSent && now - tArmWakeSent < kArmReadyMs ? kArmReadyMs - (now - tArmWakeSent) : 0);
    json += ",\"recoveryWaitMs\":";
    json += String(state == SOFTWARE_ERROR && now - TimeStateEntered < kSoftwareRecoveryMs ? kSoftwareRecoveryMs - (now - TimeStateEntered) : 0);
    json += ",\"fault\":\"";
    json += faultReason;
    json += "\",\"armBlockedReason\":\"";
    json += CanRequestArm() ? "" : ArmBlockedReason();
    json += "\",\"deployBlockedReason\":\"";
    json += CanRequestDeploy() ? "" : DeployBlockedReason();
    json += "\",\"checks\":{\"presence\":\"";
    json += CheckName(checkPresence);
    json += "\",\"lock\":\""; json += CheckName(checkLock);
    json += "\",\"servo\":\""; json += CheckName(checkServo);
    json += "\",\"wake\":\""; json += CheckName(checkWake);
    json += "\",\"telemetry\":\""; json += CheckName(checkTelemetry);
    json += "\",\"packetLoss\":\""; json += CheckName(checkPacketLoss);
    json += "\",\"pmu\":\""; json += CheckName(checkSensors[0]);
    json += "\",\"imu\":\""; json += CheckName(checkSensors[1]);
    json += "\",\"mag\":\""; json += CheckName(checkSensors[2]);
    json += "\",\"bme\":\""; json += CheckName(checkSensors[3]);
    json += "\",\"timer\":\""; json += CheckName(checkArmTimer);
    json += "\"},\"radioCommand\":{\"result\":\"";
    json += CheckName(lastRadioTxResult);
    json += "\",\"error\":"; json += String(lastRadioTxError);
    json += "}";
    json += ",\"servoReady\":";
    json += servoUp ? "true" : "false";
    json += ",\"radioConnected\":";
    json += fresh ? "true" : "false";
    json += ",\"telemetryStatus\":\"";
    json += !radioUp ? "RADIO_ERROR" : (!hasTelemetry ? "NO_DATA" : (fresh ? "RECEIVING" : "STALE"));
    json += "\",\"telemetry\":{\"hasTelemetry\":";
    json += hasTelemetry ? "true" : "false";
    WebTelemetryNumber(json, "ageMs", millis() - tLastPacket, hasTelemetry, 0);
    WebTelemetryNumber(json, "lastTelemetrySeq", lastTelemetrySeq, fresh, 0);
    WebTelemetryFlag(json, "dcmPmuOk", dcmPmuOk, fresh);
    WebTelemetryFlag(json, "dcmGnssOk", dcmGnssOk, fresh);
    WebTelemetryFlag(json, "dcmImuOk", dcmImuOk, fresh);
    WebTelemetryFlag(json, "dcmMagOk", dcmMagOk, fresh);
    WebTelemetryFlag(json, "dcmBmeOk", dcmBmeOk, fresh);
    WebTelemetryNumber(json, "dcmLatDeg", dcmLatDeg, fix, 7);
    WebTelemetryNumber(json, "dcmLonDeg", dcmLonDeg, fix, 7);
    WebTelemetryNumber(json, "dcmGpsAltM", dcmGpsAltM, fix, 1);
    WebTelemetryNumber(json, "dcmHdop", dcmHdop, fix, 2);
    WebTelemetryNumber(json, "dcmBattPct", dcmBattPct, fresh && dcmPmuOk && dcmBattPct >= 0 && dcmBattPct <= 100, 0);
    WebTelemetryNumber(json, "dcmPressurePa", dcmPressurePa, fresh && dcmBmeOk, 1);
    WebTelemetryNumber(json, "dcmTempC", dcmTempC, fresh && dcmBmeOk, 1);
    WebTelemetryNumber(json, "dcmHumidityPct", dcmHumidityPct, fresh && dcmBmeOk, 1);
    WebTelemetryVector(json, "dcmAccelMps2", dcmAccelMps2, fresh && dcmImuOk);
    WebTelemetryVector(json, "dcmGyroDps", dcmGyroDps, fresh && dcmImuOk);
    WebTelemetryVector(json, "dcmMagGauss", dcmMagGauss, fresh && dcmMagOk);
    WebTelemetryNumber(json, "lastRssi", lastRssi, fresh, 1);
    WebTelemetryNumber(json, "lastSnr", lastSnr, fresh, 1);
    json += "}}";
    cockpitServer.sendHeader("Cache-Control", "no-store");
    cockpitServer.send(200, "application/json", json);
}

void CockpitCommand(){
    if (ddsResetPending) {
        cockpitServer.send(409, "application/json", "{\"ok\":false,\"message\":\"DDS is restarting.\"}");
        return;
    }
    const String body = cockpitServer.arg("plain");
    bool accepted = false;
    const char* message = "Unknown command.";
    if (body == "{\"command\":\"LOAD\"}") {
        accepted = RequestLoad();
        message = "LOAD needs STANDBY and a ready servo.";
    } else if (body == "{\"command\":\"ARM\"}") {
        accepted = CanRequestArm();
        if (accepted) ArmCommand = true;
        message = ArmBlockedReason();
    } else if (body == "{\"command\":\"DISARM\"}") {
        accepted = state == ARMED || state == SOFTWARE_ERROR;
        if (accepted) { ArmCommand = false; DeployCommand = false; }
        message = "DISARM is available while armed or recovering from a software error.";
    } else if (body == "{\"command\":\"DEPLOY\"}") {
        accepted = CanRequestDeploy();
        if (accepted) DeployCommand = true;
        message = DeployBlockedReason();
    } else if (body == "{\"command\":\"END\"}") {
        accepted = state == DEPLOYED;
        if (accepted) EndCommand = true;
        message = "END / SLEEP is available after deployment.";
    } else if (body == "{\"command\":\"ACK\"}") {
        accepted = state == DEPLOY_FAILURE;
        if (accepted) AckCommand = true;
        message = "Failure acknowledgment is available after deployment failure.";
    }
    String response = accepted ? "{\"ok\":true,\"message\":\"Command accepted by DDS.\"}" : String("{\"ok\":false,\"message\":\"") + message + "\"}";
    cockpitServer.send(accepted ? 202 : 409, "application/json", response);
}

void CockpitReset(){
    if (cockpitServer.arg("plain") != "{\"confirmed\":true}") {
        cockpitServer.send(400, "application/json", "{\"ok\":false,\"message\":\"Reset confirmation required.\"}");
        return;
    }
    if (ddsResetPending) {
        cockpitServer.send(409, "application/json", "{\"ok\":false,\"message\":\"DDS is already restarting.\"}");
        return;
    }
    cockpitServer.sendHeader("Cache-Control", "no-store");
    cockpitServer.send(202, "application/json", "{\"ok\":true,\"message\":\"DDS restart scheduled.\"}");
    tDdsResetRequested = millis();
    ddsResetPending = true;
}

void ServiceDdsReset(){
    // No delay(), flush(), or waiting for a client acknowledgment. A lost
    // response must not prevent an already confirmed restart from happening.
    if (ddsResetPending && millis() - tDdsResetRequested >= kDdsResetResponseMs) {
        ESP.restart();
    }
}

void StartCockpitServer(){
    if (WiFi.getMode() != WIFI_AP || WiFi.softAPIP() == IPAddress(0, 0, 0, 0)) return;
    cockpitServer.on("/", HTTP_GET, [](){ cockpitServer.send_P(200, "text/html; charset=utf-8", WEB_MAIN_HTML); });
    cockpitServer.on("/main.html", HTTP_GET, [](){ cockpitServer.send_P(200, "text/html; charset=utf-8", WEB_MAIN_HTML); });
    cockpitServer.on("/cockpit_style.css", HTTP_GET, [](){ cockpitServer.send_P(200, "text/css; charset=utf-8", WEB_COCKPIT_CSS); });
    cockpitServer.on("/interface_logic.js", HTTP_GET, [](){
        cockpitServer.sendHeader("Cache-Control", "no-store");
        cockpitServer.send_P(200, "application/javascript; charset=utf-8", WEB_INTERFACE_JS);
    });
    cockpitServer.on("/api/status", HTTP_GET, CockpitStatus);
    cockpitServer.on("/api/command", HTTP_POST, CockpitCommand);
    cockpitServer.on("/api/reset", HTTP_POST, CockpitReset);
    cockpitServer.onNotFound([](){ cockpitServer.send(404, "text/plain", "Not found"); });
    cockpitServer.enableDelay(false); // removes idle delay, not synchronous socket waits
    cockpitServer.begin();
    cockpitServerUp = true;
    Serial.println("Open http://192.168.4.1/ after connecting to DDS Wi-Fi.");
}

void ServiceCockpit(){
    if (!cockpitServerUp || ddsResetPending || radioTransmitting || servoPingPhase != PING_IDLE) return; // no more synchronous HTTP waits before restart
    // The regular WebServer can block on slow requests/writes. Protect the
    // continuous switch-hold interval by deferring HTTP until it ends/resets.
    // Other HTTP work is NOT guaranteed non-blocking and needs bench validation.
    if (state == LOAD && (loadPresenceTiming || limitRaw || LimitClosed)) return;
    if (state == DEPLOY_ERROR) return; // short timed recovery movements take priority
    cockpitServer.handleClient();
}

void setup() {
    PowerOn();
    Serial.begin(115200);
    radioUp = RadioBegin();
    servoUp = ServoBegin();
    if (servoUp) {
        servoTarget = LOCK_POS; // startup requests 180 degrees, independent of presence
        servoMovePending = true;
        ServiceServo();
    }
#if DDS_LOAD_BENCH_TEST
    BenchLoadBegin(); // TEMPORARY LOAD BENCH TEST hook
#endif
    StartDdsWifi();
    StartCockpitServer();
}

void loop() {
    ServiceDdsReset();
    ServiceRadioCommands();
    ServiceServoCheck();
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
    ServiceServo();
    ServiceRadioCommands();
    ServiceCockpit();
}
