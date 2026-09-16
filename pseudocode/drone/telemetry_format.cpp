/*
LORA TRANSMITTER

1. Initialize the LoRa communication hardware.

2. Check that the LoRa transmitter is ready.

3. Receive the completed telemetry message.

4. Transmit the message to the launcher.

5. Record whether the transmission was successful.

6. If transmission fails:
   - Record a communication fault.
   - Continue trying during the next update cycle.

7. Repeat each time new telemetry is available.
*/