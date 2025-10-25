# WALL-E System Architecture

Detailed system architecture documentation for the WALL-E omnidirectional robot project.

## System Overview

WALL-E is a comprehensive embedded systems project implementing an omnidirectional robot with:
- Real-time PID-based motion control
- Wireless remote control via ESP-NOW
- Multi-layer software architecture
- Safety-critical features

## Software Architecture

### Layered Architecture

```
┌─────────────────────────────────────────────────┐
│         Application Layer                       │
│  - Main control loop                           │
│  - Command processing                          │
│  - Safety monitoring                           │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│         Control Layer                           │
│  - PID controllers                             │
│  - Motor control                               │
│  - Kinematics calculations                     │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│         Communication Layer                     │
│  - UART protocol                               │
│  - ESP-NOW protocol                            │
│  - Message parsing                             │
└────────────────┬────────────────────────────────┘
                 │
┌────────────────▼────────────────────────────────┐
│         Hardware Abstraction Layer              │
│  - PWM control                                 │
│  - GPIO operations                             │
│  - Timer/Counter                               │
│  - UART driver                                 │
└─────────────────────────────────────────────────┘
```

## Control Flow

### Main Control Loop (MCXN947)

```
1. Initialize system
   ├─> Configure peripherals
   ├─> Initialize PID controllers
   ├─> Initialize motor control
   └─> Start communication

2. Main Loop (∞):
   ├─> Update system time
   ├─> Check for incoming commands
   ├─> Process commands
   ├─> Calculate motor velocities (kinematics)
   ├─> Update PID controllers
   ├─> Apply motor outputs
   ├─> Monitor safety conditions
   └─> Repeat
```

### Communication Flow

```
Remote Control → ESP-NOW → ESP32C3 → UART → MCXN947

┌──────────────┐
│   Remote     │  Joystick input
│  (ESP32)     │  Button press
└──────┬───────┘
       │ ESP-NOW packet
       │ {cmd, data, length}
       ▼
┌──────────────┐
│   ESP32C3    │  Protocol bridge
│  (Robot)     │  Format conversion
└──────┬───────┘
       │ UART packet
       │ [START|CMD|LEN|PAYLOAD|CHK|END]
       ▼
┌──────────────┐
│   MCXN947    │  Parse command
│ (Controller) │  Execute action
└──────────────┘
```

## PID Control System

### Control Loop

```
Setpoint (desired velocity)
    │
    ▼
┌───────────┐     ┌─────────┐     ┌────────┐
│    PID    │────>│  Motor  │────>│ Wheel  │
│Controller │     │ Driver  │     │        │
└─────▲─────┘     └─────────┘     └────┬───┘
      │                                  │
      │           ┌─────────┐            │
      └───────────│ Encoder │◄───────────┘
                  └─────────┘
         (feedback - current velocity)
```

### PID Algorithm

```
error = setpoint - measured_value

P_term = Kp × error
I_term = Ki × Σ(error)
D_term = Kd × Δ(error)

output = P_term + I_term + D_term
output = clamp(output, min, max)
```

### Anti-Windup

The integral term is limited to prevent windup:
```
if (integral > integral_max):
    integral = integral_max
if (integral < integral_min):
    integral = integral_min
```

## Omnidirectional Kinematics

### Mecanum Wheel Configuration

```
        Front
    FL ─────── FR
    │          │
    │  Robot   │
    │          │
    BL ─────── BR
        Back
```

### Inverse Kinematics

Given desired robot velocities (vx, vy, ω), calculate wheel velocities:

```
v_FL = ( vx + vy + (lx + ly) × ω) / r
v_FR = (-vx + vy - (lx + ly) × ω) / r
v_BL = (-vx + vy + (lx + ly) × ω) / r
v_BR = ( vx + vy - (lx + ly) × ω) / r

Where:
  vx = lateral velocity (strafe)
  vy = forward velocity
  ω  = rotational velocity (yaw rate)
  lx = half of wheel base width
  ly = half of wheel base length
  r  = wheel radius
```

### Movement Patterns

**Forward:**
```
vx=0, vy=+v, ω=0
→ All wheels forward at same speed
```

**Strafe Right:**
```
vx=+v, vy=0, ω=0
→ FL,BR forward; FR,BL backward
```

**Rotate CW:**
```
vx=0, vy=0, ω=+ω
→ Left wheels forward; Right wheels backward
```

**Diagonal:**
```
vx=+v, vy=+v, ω=0
→ FL,BR fast; FR,BL slow/stopped
```

## Communication Protocol

### Packet Structure

```
┌─────┬─────┬────────┬──────────┬──────────┬─────┐
│START│ CMD │ LENGTH │ PAYLOAD  │ CHECKSUM │ END │
├─────┼─────┼────────┼──────────┼──────────┼─────┤
│ AA  │ XX  │   XX   │ [data]   │    XX    │ 55  │
└─────┴─────┴────────┴──────────┴──────────┴─────┘
  1     1       1      0-32 bytes     1       1
```

### Checksum Calculation

```
checksum = 0
checksum ^= cmd
checksum ^= length
for each byte in payload:
    checksum ^= byte
```

### Command Processing State Machine

```
┌──────────────┐
│ WAIT_START   │◄────────────┐
└──────┬───────┘             │
       │ (0xAA)              │
┌──────▼───────┐             │
│  WAIT_CMD    │             │
└──────┬───────┘             │
       │                     │
┌──────▼───────┐             │
│ WAIT_LENGTH  │             │
└──────┬───────┘             │
       │                     │
┌──────▼───────┐             │
│ WAIT_PAYLOAD │ (if len>0)  │
└──────┬───────┘             │
       │                     │
┌──────▼───────┐             │
│WAIT_CHECKSUM │             │
└──────┬───────┘             │
       │                     │
┌──────▼───────┐             │
│  WAIT_END    │             │
└──────┬───────┘             │
       │ (0x55, valid)       │
       │ Process packet      │
       └─────────────────────┘
```

## Safety Systems

### Multi-Layer Safety

1. **Communication Timeout**
   - If no heartbeat received for >1s
   - Action: Emergency stop

2. **Emergency Stop Command**
   - Immediate stop via dedicated button
   - Action: Disable all motors, reset PID

3. **Hardware Emergency Stop**
   - Physical switch cutting motor power
   - Action: Immediate power cutoff

4. **Watchdog Timer**
   - Detects firmware hang
   - Action: System reset

5. **Voltage Monitoring**
   - Battery voltage too low
   - Action: Controlled shutdown

### Safety State Machine

```
┌─────────┐
│  INIT   │
└────┬────┘
     │
┌────▼────┐     Emergency Stop
│ ENABLED │────────────────┐
└────┬────┘                │
     │                     │
     │ Normal operation    │
     │                     │
┌────▼────┐                │
│ RUNNING │                │
└────┬────┘                │
     │                     │
     │ Timeout/E-Stop      │
     └─────────────────────┤
                           │
                      ┌────▼────┐
                      │ STOPPED │
                      └────┬────┘
                           │
                           │ Reset command
                           └─────────┐
                                     │
                              ┌──────▼─────┐
                              │  DISABLED  │
                              └──────┬─────┘
                                     │
                                     │ Enable
                                     └────────> ENABLED
```

## Performance Characteristics

### Timing Requirements

| Task | Period | Deadline | Priority |
|------|--------|----------|----------|
| PID Update | 10ms | 10ms | High |
| Motor PWM | 20kHz | N/A | Critical |
| Communication | 50ms | 100ms | Medium |
| Safety Check | 100ms | 100ms | High |
| Telemetry | 500ms | 1s | Low |

### Latency Analysis

```
User Input → Robot Response:

Joystick movement           : 0ms
ESP32 ADC sampling          : 1-2ms
ESP-NOW transmission        : 5-10ms
ESP32C3 processing          : 1ms
UART transmission           : 2ms
MCXN947 processing          : 1ms
PID computation             : 1ms
Motor response              : 10-20ms
────────────────────────────────────
Total latency               : 21-37ms
```

### Bandwidth Requirements

- **ESP-NOW**: 250 kbps (actual: ~10 kbps for control)
- **UART**: 115200 baud (~11.5 kB/s)
- **Command rate**: 20 Hz (50ms period)
- **Packet size**: ~40 bytes per command
- **Bandwidth usage**: 20 Hz × 40 bytes = 800 bytes/s (< 1 kB/s)

## Error Handling

### Error Detection

1. **Communication Errors**
   - Checksum mismatch
   - Timeout
   - Invalid packet format

2. **Control Errors**
   - Encoder failure
   - Motor stall
   - Setpoint saturation

3. **System Errors**
   - Low battery
   - Overcurrent
   - Overtemperature

### Recovery Strategies

| Error Type | Detection | Recovery |
|------------|-----------|----------|
| Comm timeout | Timer | Emergency stop → retry |
| Checksum fail | Checksum | Ignore packet → request resend |
| Motor stall | Encoder | Stop motor → alert user |
| Low battery | ADC | Reduce speed → shutdown |
| Overcurrent | Driver | Shutdown → investigate |

## Testing Strategy

### Unit Tests

- PID controller functionality
- Kinematics calculations
- Protocol parsing
- Checksum validation

### Integration Tests

- Motor control with PID
- Communication end-to-end
- Safety system triggers
- Complete control loop

### System Tests

- Full movement patterns
- Remote control operation
- Safety scenarios
- Stress testing
- Battery life testing

## Development Workflow

```
1. Design
   └─> Architecture
   └─> Interface definitions

2. Implementation
   └─> Module development
   └─> Unit testing

3. Integration
   └─> Module integration
   └─> Integration testing

4. Validation
   └─> System testing
   └─> Performance validation

5. Deployment
   └─> Documentation
   └─> Release
```

## Future Enhancements

### Planned Features

1. **Advanced Control**
   - Model Predictive Control (MPC)
   - Trajectory planning
   - Obstacle avoidance

2. **Sensors**
   - IMU for orientation
   - Ultrasonic for distance
   - Lidar for mapping

3. **Autonomy**
   - Waypoint navigation
   - Path planning
   - Localization (SLAM)

4. **Interface**
   - Mobile app control
   - Web dashboard
   - Voice commands

### Scalability

The architecture supports:
- Additional sensors (I2C, SPI)
- More complex control algorithms
- Multiple communication protocols
- Extended telemetry
- Data logging and analysis

## References

1. PID Control Theory
2. Mecanum Wheel Kinematics
3. ESP-NOW Protocol Specification
4. MCXN947 Reference Manual
5. Real-Time Systems Design

## License

See main LICENSE file in repository root.
