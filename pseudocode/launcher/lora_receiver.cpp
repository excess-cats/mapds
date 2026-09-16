/*
LORA RECEIVER

1. Initialize the LoRa receiver.

2. Listen for telemetry from the deployed drone module.

3. When telemetry is received:
   - Check that the message is valid.
   - Read GPS data.
   - Read IMU data.
   - Read drone battery status.
   - Read module and sensor status.

4. Pass valid telemetry to launcher_main.

5. Track communication status.

6. Report communication loss if telemetry is not received for too long.
*/