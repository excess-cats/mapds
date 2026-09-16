# MAPDS Software

This repository contains the software framework and pseudocode for the Modular Autonomous Payload Deployment System (MAPDS).

The software is organized into four sections:

- **Drone** - collects sensor data and sends telemetry through LoRa.
- **Launcher** - handles system control, safety checks, hardware, and received telemetry.
- **Interface** - displays system information and handles operator commands.
- **Shared** - keeps common system states and telemetry definitions consistent.

The current `.cpp` files contain pseudocode inside comment blocks. These will later be replaced with the final implementation while keeping the same general structure.
