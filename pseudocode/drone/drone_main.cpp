/*
DRONE MAIN

1. Start the drone-attached module.

2. Initialize the GPS sensor.

3. Initialize the IMU sensor.

4. Initialize battery and module-status monitoring.

5. Initialize the LoRa transmitter.

6. Check that the required sensors and communication hardware are working.

7. Begin the main operating loop.

8. Repeatedly:
   - Read the GPS data.
   - Read the IMU data.
   - Read battery and module status.
   - Check that the sensor data is valid.
   - Send the collected data to the telemetry module.
   - Build the telemetry message.
   - Send the telemetry message to the launcher through LoRa.
   - Wait for the next update cycle.

9. Continue this loop while the drone module is powered.
*/