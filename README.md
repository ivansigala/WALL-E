# WALL-E - Omnidirectional Robot

Final embedded systems project: An omnidirectional robot with PID control, featuring wireless remote control capabilities.

## Overview

WALL-E is an embedded systems project that implements an omnidirectional robot platform with the following key features:
- **PID-based motion control** for precise movement
- **Wireless remote control** using ESP-NOW protocol
- **MCXN947 NXP board** as the main controller
- **ESP32C3 Super Mini** for wireless communication
- **Omnidirectional movement** with 3-4 mecanum/omni wheels

## Hardware Components

- **Main Controller**: MCXN947 NXP Development Board
- **Communication Module**: ESP32C3 Super Mini
- **Motors**: 3-4 DC motors with encoders (for omnidirectional movement)
- **Motor Drivers**: H-Bridge motor driver modules
- **Power Supply**: LiPo battery with voltage regulation
- **Remote Control**: ESP32-based remote controller

## Project Structure

```
WALL-E/
├── firmware/
│   ├── mcxn947/          # Main controller firmware
│   │   ├── src/          # Source files
│   │   ├── inc/          # Header files
│   │   └── README.md     # Build instructions
│   └── esp32c3/          # ESP32C3 communication firmware
│       ├── src/          # Arduino/ESP-IDF source
│       └── README.md     # Build instructions
├── hardware/             # Hardware schematics and pinouts
├── docs/                 # Additional documentation
└── README.md            # This file
```

## Features

### Motion Control
- PID controller for velocity and position control
- Support for omnidirectional movement (forward, backward, strafe, rotation)
- Encoder feedback for closed-loop control
- Configurable PID parameters

### Communication
- ESP-NOW protocol for low-latency wireless control
- Bidirectional communication (commands and telemetry)
- Auto-reconnection and error handling
- Real-time status monitoring

### Safety Features
- Emergency stop functionality
- Timeout-based motor cutoff
- Battery voltage monitoring
- Overcurrent protection

## Getting Started

### Prerequisites
- MCUXpresso IDE (for MCXN947 development)
- Arduino IDE or PlatformIO (for ESP32C3 development)
- USB-to-Serial adapter for programming
- ST-Link or J-Link debugger (for MCXN947)

### Building and Flashing

#### MCXN947 Firmware
See [firmware/mcxn947/README.md](firmware/mcxn947/README.md) for detailed instructions.

#### ESP32C3 Firmware
See [firmware/esp32c3/README.md](firmware/esp32c3/README.md) for detailed instructions.

## Usage

1. Power on the robot
2. Power on the remote control
3. Wait for automatic pairing via ESP-NOW
4. Use remote control to send movement commands
5. Robot responds with PID-controlled motion

## Control Commands

- **Forward/Backward**: Linear movement along Y-axis
- **Strafe Left/Right**: Linear movement along X-axis
- **Rotate CW/CCW**: Rotational movement
- **Combined movements**: Simultaneous translation and rotation
- **Emergency Stop**: Immediate motor cutoff

## Configuration

Key parameters can be configured in the firmware:
- PID gains (Kp, Ki, Kd)
- Motor limits and safety thresholds
- Communication timeouts
- Wheel configuration and kinematics

## Contributing

This is an academic project. Feel free to fork and adapt for your own embedded systems projects.

## License

See LICENSE file for details.

## Authors

- Ivan Sigala

## Acknowledgments

- NXP for MCXN947 development tools and SDK
- Espressif for ESP32 platform and ESP-NOW protocol
