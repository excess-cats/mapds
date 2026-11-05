# MAPDS fail-case test procedure


## Setup for every test

1. Flash the DDS and the DCM.
2. Plug in the servo's 12 V **before** powering the DDS. The DDS checks the servo only at boot.
3. Wire the limit switch.
4. Connect a laptop or tablet to the DDS's WiFi. Open the address the DDS prints at boot (normally `http://192.168.4.1`).
5. Connect the DCM to a second laptop over USB and open a serial monitor at 115200. You'll type the DCM's fault keys here and see the commands it receives.



**Keys on the DCM serial monitor**

| Key | Effect |
|---|---|
| `i` | Report the motion sensor (IMU) as failed. Press again to undo. |
| `m` | Report the compass (magnetometer) as failed. Press again to undo. |
| `e` | Report the environmental sensor (BME) as failed. Press again to undo. |
| `p` | Report the power system (PMU) as failed. Press again to undo. |
| `x` | Drop the next packet |
| `X` | Drop every other packet. Press again to stop. |
| `w` / `s` / `r` | WAKE / SLEEP / RESTART, as if the DDS sent it |

A reboot clears every fault key.

**Arming** means: PRESENCE shows ATTACHED, the DCM is on, Arm/Disarm is on, and the "System checks" countdown has finished.

---

## DDS tests

### D1: Servo dead at boot
1. Switch off the 12 V and restart the DDS.
2. Close the limit switch and turn Arm on.
3. **Expected:** the DDS stays in STANDBY.
4. Switch on the 12 V and turn Arm on again.
5. **Expected:** the DDS still stays in STANDBY. Restart the DDS, and then it arms.

### D2: Lock fails, servo doesn't answer
1. Turn the Lock switch to UNLOCKED. **Expected:** UNLOCK.
2. Switch off the 12 V and close the limit switch.
3. Turn the Lock switch to LOCKED.
4. **Expected:** LOCKING, then UNLOCK about 2 s later. The Lock switch flips back to UNLOCKED and stays there. The lock-failed indicator turns on.
5. Switch the 12 V back on and turn the Lock switch to LOCKED.
6. **Expected:** STANDBY, and the lock-failed indicator is off.

### D3: Lock fails, hook blocked
1. Turn the Lock switch to UNLOCKED and close the limit switch.
2. Block the hook so it can't close.
3. Turn the Lock switch to LOCKED.
4. **Expected:** the same as D2 step 4.

### D4: Arm without a drone
1. Leave the limit switch open, or unplug it. **Expected:** PRESENCE shows NOT ATTACHED.
2. Turn Arm on.
3. **Expected:** the DDS stays in STANDBY.
4. Close the limit switch.
5. **Expected:** ARMED right away, and the DCM monitor prints `WAKE received`.

### D5: DCM never answers
1. Turn the DCM off. Turn Arm on.
2. **Expected:** ARMED with the countdown. When the countdown ends, the state goes to SOFTWARE ERROR.
3. Wait 60 s.
4. **Expected:** STANDBY, then ARMED again with a new countdown.
5. Turn Arm off during SOFTWARE ERROR.
6. **Expected:** STANDBY.

### D6: DCM goes quiet while armed
1. Arm the DDS.
2. Unplug the DCM.
3. **Expected:** SOFTWARE ERROR when "Last packet received" in Telemetry reaches about 10 s.

### D7: DCM sensor fails
1. Arm the DDS and open Telemetry.
2. On the DCM monitor, press `i`.
3. **Expected:**
   - Motion sensor shows FAULT, and the state goes to SOFTWARE ERROR.
   - The DCM monitor prints `RESTART received`.
   - About 60 s later the DDS re-arms, and every sensor shows OK.
4. Repeat with `m`, `e` and `p`.
5. **Expected:** "No GPS fix" in the log, which is normal indoors, never causes SOFTWARE ERROR.

### D8: Lost packets
1. Arm the DDS.
2. On the DCM monitor, press `x` 3 times, about 3 s apart.
3. **Expected:** the DDS stays ARMED.
4. Press `X`.
5. **Expected:** SOFTWARE ERROR within about 10 s.

### D9: Deploy pressed too early
1. Turn Arm on, and press DEPLOY → Yes, deploy before the countdown ends.
2. **Expected:** the DDS stays ARMED
3. Wait for the countdown to finish.
4. **Expected:** nothing happens until you press DEPLOY again.

### D10: Drone stuck, then frees
1. Hold the limit switch closed by hand.
2. Arm the DDS, then press DEPLOY → Yes, deploy.
3. **Expected:** DEPLOYING, then DEPLOY ERROR at 10 s, and the servo jiggles. The page may freeze for about 1.5 s during the jiggle.
4. Let go of the switch while the servo is jiggling.
5. **Expected:** DEPLOYED, and END / SLEEP becomes clickable.

### D11: Drone stuck for good
1. Tape the limit switch closed.
2. Arm the DDS, then press DEPLOY → Yes, deploy.
3. **Expected:** DEPLOY ERROR, then DEPLOY FAILURE, and the ACKNOWLEDGE FAILURE button appears.
4. Press ACKNOWLEDGE FAILURE.
5. **Expected:** STANDBY. Arm/Disarm is off, and the DDS doesn't re-arm.
6. Repeat with the 12 V switched off after arming. **Expected:** the same result.

### D12: Lost SLEEP
1. Arm the DDS and wait for packets to arrive.
2. Turn Arm off.
3. Check whether the DCM monitor prints `SLEEP received`.
4. Repeat steps 1–3 20 times and count the misses.
5. **Target:** 20/20. Expect a few misses for now. This is known gap G2.

---

## DCM tests

### C1: Sensors come back after sleep
1. Arm the DDS and open Telemetry.
2. **Expected:** every sensor shows OK.
3. Turn Arm off, and repeat steps 1–2 10 times.

### C2: No false sensor faults
1. Arm the DDS and leave it for 30 min.
2. **Expected:** the state stays ARMED the whole time.

### C3: No GPS fix
1. Arm the DDS indoors.
2. **Expected:** GPS fix shows NO FIX, and the state stays ARMED.
3. For reference, arm outdoors and time how long it takes to reach FIX.

### C4: Low battery
1. Unplug the DCM's USB.
2. Connect a bench supply, current-limited to 1 A, to the battery terminals. Set it to the cell's full voltage.
3. Arm the DDS.
4. Lower the supply in 0.1 V steps.
5. **Expected:** the DCM BATTERY reading turns orange, then red, and the state stays ARMED.
6. **Expected:** at about 3.0 V the DCM shuts off, and the state goes to SOFTWARE ERROR about 10 s later.

### C5: DCM reboots mid-flight
1. Arm the DDS.
2. Press the T-Beam's RST button.
3. **Expected:**
   - SOFTWARE ERROR after about 10 s.
   - About 60 s later the DDS re-arms.
   - Packet number starts again from 0.

### C6: RESTART command
1. On the DCM monitor, press `w`, then `r`.
2. **Expected:** the DCM reboots and comes back asleep.
3. Press `r` again while it's asleep.
4. **Expected:** the same result.

