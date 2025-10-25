# MCXN947 Firmware

Main controller firmware for the WALL-E omnidirectional robot using the NXP MCXN947 microcontroller.

## Features

- **PID Control**: Advanced PID controller for precise motor velocity control
- **Omnidirectional Kinematics**: Inverse kinematics for 4-wheel mecanum/omni drive
- **Motor Control**: PWM-based control for 4 independent motors with encoder feedback
- **Communication**: UART protocol for communication with ESP32C3 module
- **Safety**: Emergency stop, communication timeout, and watchdog features

## Hardware Requirements

- MCXN947 Development Board
- 4x DC Motors with Encoders
- 4x H-Bridge Motor Driver Modules (e.g., L298N, DRV8833)
- Power Supply (7.4V - 12V LiPo battery recommended)
- UART connection to ESP32C3 module

## Pin Configuration

### Motor PWM Outputs
- Motor 1 (Front Left): PWM0 - Pin XX
- Motor 2 (Front Right): PWM1 - Pin XX
- Motor 3 (Back Left): PWM2 - Pin XX
- Motor 4 (Back Right): PWM3 - Pin XX

### Motor Direction Outputs
- Motor 1 Direction: GPIO - Pin XX
- Motor 2 Direction: GPIO - Pin XX
- Motor 3 Direction: GPIO - Pin XX
- Motor 4 Direction: GPIO - Pin XX

### Encoder Inputs
- Motor 1 Encoder: Timer0 - Pins XX, XX
- Motor 2 Encoder: Timer1 - Pins XX, XX
- Motor 3 Encoder: Timer2 - Pins XX, XX
- Motor 4 Encoder: Timer3 - Pins XX, XX

### UART Communication
- UART TX: Pin XX (to ESP32C3 RX)
- UART RX: Pin XX (from ESP32C3 TX)
- Baud Rate: 115200

### Other
- Battery Voltage Monitor: ADC - Pin XX
- Status LED: GPIO - Pin XX

## Project Structure

```
mcxn947/
├── inc/
│   ├── pid_controller.h      # PID controller interface
│   ├── motor_control.h       # Motor control interface
│   └── comm_protocol.h       # Communication protocol
├── src/
│   ├── main.c               # Main application
│   ├── pid_controller.c     # PID implementation
│   ├── motor_control.c      # Motor control implementation
│   └── comm_protocol.c      # Protocol implementation
└── README.md               # This file
```

## Building the Firmware

### Using MCUXpresso IDE

1. **Import Project**
   - Open MCUXpresso IDE
   - File → Import → Existing Projects into Workspace
   - Select the `mcxn947` directory
   - Click Finish

2. **Configure SDK**
   - Install MCXN947 SDK if not already installed
   - Project → Properties → MCUXpresso SDK
   - Select appropriate SDK version

3. **Build**
   - Right-click on project
   - Build Project
   - Or use Ctrl+B

4. **Flash**
   - Connect debugger (J-Link, LPC-Link2, etc.)
   - Right-click on project → Debug As → MCUXpresso IDE LinkServer
   - Or use the Debug button

### Using Command Line (with MCUXpresso tools)

```bash
# Build
cd mcxn947
make all

# Flash (adjust for your debugger)
make flash
```

## Configuration

### PID Tuning

Default PID values are set in `motor_control.c`:
```c
// Kp, Ki, Kd
Motor_InitSingle(&motors[MOTOR_FRONT_LEFT], MOTOR_FRONT_LEFT, 1.0f, 0.1f, 0.05f);
```

To tune PID parameters:
1. Start with P-only control (Ki=0, Kd=0)
2. Increase Kp until system oscillates
3. Add derivative (Kd) to dampen oscillations
4. Add integral (Ki) to eliminate steady-state error
5. Adjust via serial commands during runtime if needed

### Robot Physical Parameters

Adjust in `motor_control.c`:
```c
#define WHEEL_BASE_WIDTH  0.20f   // meters
#define WHEEL_BASE_LENGTH 0.20f   // meters
#define WHEEL_RADIUS      0.05f   // meters
```

### Communication Settings

Adjust in `comm_protocol.h`:
```c
#define COMM_TIMEOUT_MS     1000  // Communication timeout
```

## Hardware Abstraction Layer

The firmware includes placeholder HAL functions that need to be implemented for your specific hardware setup:

1. **PWM Functions** (`motor_control.c`)
   - `HAL_SetMotorPWM()` - Set PWM duty cycle
   - `HAL_SetMotorDirection()` - Set motor direction

2. **UART Functions** (`comm_protocol.c`)
   - `HAL_UART_Transmit()` - Send data over UART

3. **Timer Functions** (`main.c`)
   - `System_GetTime()` - Get system time in milliseconds
   - Encoder input capture

Refer to the MCXN947 SDK documentation for implementing these functions.

## Testing

### Motor Test
1. Flash firmware to MCXN947
2. Connect motors and power supply
3. Send velocity commands via UART
4. Verify motor response

### PID Test
1. Connect encoders
2. Send velocity setpoint
3. Monitor encoder feedback
4. Verify PID control is working

### Communication Test
1. Connect ESP32C3 via UART
2. Send test commands
3. Verify response
4. Check timeout handling

## Troubleshooting

### Motors not responding
- Check power supply voltage
- Verify PWM signals with oscilloscope
- Check H-bridge connections
- Verify motor driver enable pins

### Communication issues
- Verify UART baud rate (115200)
- Check TX/RX connections (not crossed)
- Test with loopback
- Add debug prints

### PID not stable
- Reduce Kp gain
- Add Kd damping
- Check encoder connections
- Verify velocity calculation

## Safety Notes

- Always test with motors disconnected first
- Use current-limited power supply during development
- Implement emergency stop button
- Monitor battery voltage to prevent over-discharge
- Add fuses for overcurrent protection

## Further Development

- [ ] Implement encoder velocity calculation
- [ ] Add battery voltage monitoring
- [ ] Implement motor current sensing
- [ ] Add calibration routine
- [ ] Implement data logging
- [ ] Add debug/diagnostic mode

## License

See main LICENSE file in repository root.
