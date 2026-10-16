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
 * ---------------------------------------------------------------- SERIAL STAND-IN (115200)
 *   l = toggle Load      a = toggle Arm      d = toggle Deploy
 *   e = END/SLEEP        k = ACKNOWLEDGE FAILURE
 *   p = toggle a simulated limit switch (until the real one has a GPIO)
 *   Print every transition ("ARMED -> SOFTWARE_ERROR: no telemetry 10 s") and a
 *   status line once a second: state, lights, warnings, telemetry age, missed count.
 *
 * ---------------------------------------------------------------- PROTOCOL
 *   ../shared/protocol.h: radio settings, Header, TelemetryFrame, buildWake/Sleep/Restart().
 *
 * ---------------------------------------------------------------- HARDWARE
 *   Power (no PMU on the Heltec: everything below is plain GPIO, from the V3.1 and V3.2 schematics)
 *   - VDD_3V3            always on. Powers the ESP32 and the SX1262 (no rail to enable for the radio).
 *   - Vext (GPIO 36)     switched 3.3 V: OLED + the "Vext" header pin. LOW = on, HIGH = off (pull-up, P-FET).
 *                        Leave it off unless something is wired to it. Don't power the limit switch from it.
 *   - Battery sense      ADC on GPIO 1, through a 390k/100k divider: VBAT = pin voltage * 4.9.
 *                        The divider is switched by ADC_Ctrl (GPIO 37), to save the battery between reads:
 *                          V3.0/V3.1: LOW = divider on    V3.2: HIGH = divider on   (check the board's silkscreen)
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

bool LoadCommand = false;
bool ArmCommand = false;
bool DeployCommand = false;
bool EndCommand = false;    
bool AckCommand = false;    
bool LimitClosed = true;     // TESTING: simulated limit switch until it has a GPIO (true = drone present)

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
            if(to == ARMED)      { UpdateState(to); Serial.println("  [send WAKE, reset health]"); return true; }
            break;
        case LOAD:
            if(to == STANDBY)    { UpdateState(to); Serial.println("  [servo -> LOCK_POS]"); return true; }
            break;
        case NOT_LOADED:
            if(to == STANDBY)    { UpdateState(to); return true; }
            if(to == LOAD)       { UpdateState(to); Serial.println("  [servo -> LOAD_POS]"); return true; }
            break;
        case ARMED:
            if(to == STANDBY)        { UpdateState(to); Serial.println("  [send SLEEP]"); return true; }
            if(to == NOT_LOADED)     { UpdateState(to); Serial.println("  [send SLEEP]"); return true; }
            if(to == SOFTWARE_ERROR) { UpdateState(to); Serial.println("  [send RESTART]"); return true; }
            if(to == DEPLOYING)      { UpdateState(to); Serial.println("  [servo -> RELEASE_POS]"); return true; }
            break;
        case SOFTWARE_ERROR:
            if(to == ARMED)      { UpdateState(to); Serial.println("  [send WAKE, reset health]"); return true; }
            if(to == STANDBY)    { UpdateState(to); Serial.println("  [send SLEEP]"); return true; }
            if(to == NOT_LOADED) { UpdateState(to); Serial.println("  [send SLEEP]"); return true; }
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
            if(to == STANDBY)    { UpdateState(to); Serial.println("  [send SLEEP, switches off]"); return true; }
            break;
        case DEPLOY_FAILURE:
            if(to == STANDBY)    { UpdateState(to); Serial.println("  [send SLEEP, switches off]"); return true; }
            break;
    }
    Serial.printf("No arrow %s -> %s, ignored\n", StateName(from), StateName(to));
    return false;
}

// ---------------------------------------------------------------- state functions
// One per state, called every loop(). Each only decides when to leave, via StateChange().

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

// ---------------------------------------------------------------- TESTING ONLY
// Serial stand-in for the web UI and the limit switch. Remove once those exist.
//   l / a / d  toggle Load / Arm / Deploy
//   e          END/SLEEP            k   ACKNOWLEDGE FAILURE
//   p          toggle the simulated limit switch

void ReadSerialTest(){
    while(Serial.available()){
        char c = Serial.read();
        switch(c){
            case 'l': LoadCommand   = !LoadCommand;   Serial.printf("TEST: Load %s\n",   LoadCommand   ? "on" : "off"); break;
            case 'a': ArmCommand    = !ArmCommand;    Serial.printf("TEST: Arm %s\n",    ArmCommand    ? "on" : "off"); break;
            case 'd': DeployCommand = !DeployCommand; Serial.printf("TEST: Deploy %s\n", DeployCommand ? "on" : "off"); break;
            case 'e': EndCommand = true;              Serial.println("TEST: END/SLEEP"); break;
            case 'k': AckCommand = true;              Serial.println("TEST: ACKNOWLEDGE FAILURE"); break;
            case 'p': LimitClosed = !LimitClosed;     Serial.printf("TEST: limit %s\n",  LimitClosed   ? "CLOSED" : "OPEN"); break;
            default: break;   // ignore newlines and anything else
        }
    }
}

void setup() {
    Serial.begin(115200);
}

void loop() {
    ReadSerialTest();   // TESTING ONLY

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
