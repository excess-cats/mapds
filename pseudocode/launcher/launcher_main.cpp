/*
LAUNCHER MAIN

1. Start the launcher system.

2. Initialize:
   - Launcher hardware
   - LoRa receiver
   - Cockpit interface connection

3. Set the system state to Standby.

4. Repeatedly:
   - Read launcher hardware status.
   - Receive drone telemetry.
   - Read operator commands.
   - Run safety checks.
   - Update the system state.
   - Command deployment when allowed.
   - Send updated system information to the cockpit interface.

5. Continue while the launcher is powered.
*/