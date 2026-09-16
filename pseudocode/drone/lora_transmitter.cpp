/*
TELEMETRY FORMAT

1. Receive the latest drone sensor data.

2. Create one telemetry message containing:
   - GPS latitude
   - GPS longitude
   - IMU data
   - Battery level
   - Module status
   - Sensor status

3. Add a message number or time value so new data can be identified.

4. Keep the telemetry message in the same format every time.

5. Pass the completed telemetry message to the LoRa transmitter.
*/