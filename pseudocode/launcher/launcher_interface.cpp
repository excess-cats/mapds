/*
LAUNCHER INTERFACE

1. Receive the latest launcher and drone information.

2. Send updated information to the cockpit interface:
   - System state
   - GPS
   - Battery levels
   - LoRa status
   - Faults
   - Drone presence

3. Receive operator commands from the cockpit interface.

4. Pass Arm and Deploy requests to launcher_main.

5. Do not directly control the release hardware.
*/