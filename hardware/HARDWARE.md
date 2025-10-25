# WALL-E Hardware Documentation

This document describes the hardware components, connections, and assembly instructions for the WALL-E omnidirectional robot.

## Bill of Materials (BOM)

### Main Components

| Component | Description | Quantity | Notes |
|-----------|-------------|----------|-------|
| MCXN947 | NXP MCXN947 Development Board | 1 | Main controller |
| ESP32C3 Super Mini | ESP32C3 communication module | 1 | Robot side |
| ESP32 Dev Board | ESP32 development board | 1 | Remote control |
| DC Motors | 12V DC motors with encoders | 4 | Recommend 100-200 RPM |
| Motor Drivers | L298N or DRV8833 H-Bridge | 2-4 | 2A+ per channel |
| Mecanum Wheels | 60-100mm diameter | 4 | Or omni wheels |
| Battery | 7.4V-12V LiPo (2S-3S) | 1 | 2000-5000mAh recommended |
| Buck Converter | 5V/3A output | 1 | For logic power |
| Voltage Regulator | 3.3V/1A output | 1 | For ESP32C3 |
| Chassis | Custom or off-the-shelf | 1 | Aluminum or acrylic |

### Electronics

| Component | Description | Quantity |
|-----------|-------------|----------|
| Analog Joystick | Dual-axis module | 1 |
| Push Buttons | Tactile switches | 2 |
| LED | 5mm LED | 2-4 |
| Resistors | 220Ω for LEDs | 4 |
| Capacitors | 100µF electrolytic | 4 |
| Diodes | 1N4007 | 8 |
| Connectors | JST-XH or similar | Various |
| Heat Shrink | Various sizes | As needed |
| Wire | 22-24 AWG | As needed |

### Mechanical

| Component | Description | Quantity |
|-----------|-------------|----------|
| Mounting Brackets | For motors | 4 |
| Standoffs | M3 x 10mm | 20 |
| Screws | M3 x 6mm | 40 |
| Nuts | M3 | 40 |
| Battery Holder | For selected battery | 1 |
| Switch | Power switch (10A) | 1 |

## System Architecture

```
┌─────────────────────────────────────────────────┐
│                   WALL-E Robot                   │
├─────────────────────────────────────────────────┤
│  ┌──────────────────────────────────────────┐   │
│  │           MCXN947 Controller             │   │
│  │  - PID Control                           │   │
│  │  - Motor Control                         │   │
│  │  - Sensor Processing                     │   │
│  └───────┬──────────────────────────────────┘   │
│          │ UART (115200 baud)                    │
│  ┌───────▼──────────────────────────────────┐   │
│  │           ESP32C3 Module                 │   │
│  │  - ESP-NOW Communication                 │   │
│  │  - Protocol Bridge                       │   │
│  └──────────────────────────────────────────┘   │
│          │                                        │
│          │ ESP-NOW (2.4GHz)                      │
└──────────┼────────────────────────────────────┬─┘
           │                                    │
           │                                    │
    ┌──────▼────────────────┐          ┌───────▼────────┐
    │  Remote Control       │          │   4x Motors    │
    │  - ESP32              │          │   + Encoders   │
    │  - Joystick           │          └────────────────┘
    │  - Buttons            │
    └───────────────────────┘
```

## Pin Connections

### MCXN947 Pin Assignment

#### Motor Control
```
Motor 1 (Front Left):
  PWM:        PWM0_CH0 (Pin PX_XX)
  Direction:  GPIO_XX (Pin PX_XX)
  Encoder A:  FTM0_CH0 (Pin PX_XX)
  Encoder B:  FTM0_CH1 (Pin PX_XX)

Motor 2 (Front Right):
  PWM:        PWM0_CH1 (Pin PX_XX)
  Direction:  GPIO_XX (Pin PX_XX)
  Encoder A:  FTM1_CH0 (Pin PX_XX)
  Encoder B:  FTM1_CH1 (Pin PX_XX)

Motor 3 (Back Left):
  PWM:        PWM0_CH2 (Pin PX_XX)
  Direction:  GPIO_XX (Pin PX_XX)
  Encoder A:  FTM2_CH0 (Pin PX_XX)
  Encoder B:  FTM2_CH1 (Pin PX_XX)

Motor 4 (Back Right):
  PWM:        PWM0_CH3 (Pin PX_XX)
  Direction:  GPIO_XX (Pin PX_XX)
  Encoder A:  FTM3_CH0 (Pin PX_XX)
  Encoder B:  FTM3_CH1 (Pin PX_XX)
```

#### Communication
```
UART to ESP32C3:
  TX:         UART0_TX (Pin PX_XX)
  RX:         UART0_RX (Pin PX_XX)
```

#### Sensors and Status
```
Battery Voltage:  ADC0_CH0 (Pin PX_XX)
Status LED:       GPIO_XX (Pin PX_XX)
```

### ESP32C3 Super Mini Pin Assignment

```
UART to MCXN947:
  TX (GPIO21) → MCXN947 RX
  RX (GPIO20) → MCXN947 TX
  
Power:
  3.3V → MCXN947 3.3V (or separate regulator)
  GND → MCXN947 GND

Status LED:
  GPIO8 (Built-in LED)
```

### Remote Control ESP32 Pin Assignment

```
Joystick:
  VRx → GPIO34 (ADC1_CH6)
  VRy → GPIO35 (ADC1_CH7)
  VCC → 3.3V
  GND → GND

Buttons:
  Emergency Stop → GPIO25 (INPUT_PULLUP)
  Enable/Disable → GPIO26 (INPUT_PULLUP)

Status LED:
  Anode → GPIO2 via 220Ω resistor
  Cathode → GND
```

## Motor Driver Connections

### Using L298N (Dual H-Bridge)

**Driver 1 (Motors 1 & 2):**
```
L298N              MCXN947
-----              -------
IN1            →   Motor1_DIR
IN2            →   GND (or inverse DIR)
ENA            →   Motor1_PWM
IN3            →   Motor2_DIR
IN4            →   GND (or inverse DIR)
ENB            →   Motor2_PWM

OUT1, OUT2     →   Motor 1
OUT3, OUT4     →   Motor 2

12V            →   Battery +
GND            →   Battery -
5V             →   Not used (or 5V supply)
```

**Driver 2 (Motors 3 & 4):**
Similar configuration for motors 3 and 4.

### Protection Components

Add to each motor:
- 1N4007 diode across motor terminals (flyback protection)
- 100µF capacitor across motor terminals (noise filtering)

## Power Distribution

```
Battery (7.4V-12V)
    │
    ├──[Switch]──┬──> Motor Drivers (Raw battery voltage)
    │            │
    │            └──[Fuse 5A]──> Emergency power cutoff
    │
    └──[Buck Converter to 5V]──┬──> MCXN947 (5V input)
                                │
                                └──[LDO to 3.3V]──> ESP32C3
```

### Power Requirements

| Component | Voltage | Current (typ) | Current (max) |
|-----------|---------|---------------|---------------|
| MCXN947 | 5V | 200mA | 500mA |
| ESP32C3 | 3.3V | 80mA | 300mA |
| Motors (each) | 12V | 500mA | 2A |
| Motor Drivers | 12V | 100mA | 200mA |
| **Total** | - | ~2.5A | ~8.5A |

**Recommended battery:** 3S LiPo (11.1V nominal, 12.6V full, 9V empty) with 3000mAh capacity for ~1 hour runtime.

## Assembly Instructions

### 1. Chassis Preparation
1. Mount motor brackets to chassis
2. Install motors with mecanum wheels
3. Ensure wheels are oriented correctly for omnidirectional movement
4. Secure all mounting hardware

### 2. Electronics Mounting
1. Mount MCXN947 board on chassis using standoffs
2. Mount motor drivers near motors
3. Mount ESP32C3 near MCXN947
4. Mount buck converter and voltage regulator
5. Install battery holder
6. Mount power switch in accessible location

### 3. Motor Wiring
1. Connect motors to motor drivers
2. Add flyback diodes to each motor
3. Connect motor driver inputs to MCXN947
4. Connect encoder outputs to MCXN947
5. Test each motor individually

### 4. Power Wiring
1. Connect battery to switch
2. Wire switch to motor drivers and buck converter
3. Connect buck converter output to MCXN947
4. Connect voltage regulator output to ESP32C3
5. Add bulk capacitors near each power connection
6. Use appropriate gauge wire for motor power

### 5. Communication Wiring
1. Connect MCXN947 UART to ESP32C3
2. Ensure proper TX-RX connection
3. Connect common ground
4. Keep wires short and away from motor wiring

### 6. Remote Control Assembly
1. Mount joystick to enclosure
2. Install buttons in accessible positions
3. Connect components to ESP32
4. Add status LED
5. Install battery and power switch

### 7. Final Assembly
1. Route all wires neatly
2. Secure with zip ties or wire management
3. Add labels to connections
4. Test continuity of all connections
5. Verify no shorts before powering on

## Safety Considerations

1. **Fuses**: Add fuses on battery positive line (5A recommended)
2. **Emergency Stop**: Hardware emergency stop that cuts motor power
3. **Reverse Polarity Protection**: Add diode or dedicated IC
4. **Overcurrent Protection**: Motor drivers should have built-in protection
5. **Battery Protection**: Use LiPo battery with protection circuit
6. **Thermal**: Ensure adequate cooling for motor drivers
7. **Mechanical**: Add safety barriers if needed

## Testing Procedure

### Pre-Power Tests
1. ✓ Check all connections with multimeter
2. ✓ Verify no shorts between power and ground
3. ✓ Confirm correct polarity
4. ✓ Check motor connections

### Power-On Tests
1. ✓ Power on with motors disconnected
2. ✓ Verify 5V and 3.3V rails
3. ✓ Check controller boot
4. ✓ Verify ESP32C3 communication
5. ✓ Test emergency stop

### Motor Tests
1. ✓ Test each motor individually
2. ✓ Verify direction control
3. ✓ Check PWM speed control
4. ✓ Verify encoder feedback
5. ✓ Test all wheels together

### System Tests
1. ✓ Test remote control communication
2. ✓ Verify command execution
3. ✓ Test PID control
4. ✓ Test omnidirectional movement
5. ✓ Verify safety features

## Troubleshooting

### Motors not responding
- Check power connections
- Verify motor driver enable pins
- Test motor driver outputs with multimeter
- Check PWM signals with oscilloscope

### Encoders not working
- Verify encoder power (usually 5V)
- Check signal integrity with oscilloscope
- Ensure proper pull-up/pull-down resistors
- Verify timer configuration in firmware

### Communication issues
- Check TX/RX wiring (not swapped)
- Verify baud rate matches
- Check ground connection
- Test with loopback

### Erratic behavior
- Check for loose connections
- Verify power supply stability
- Add decoupling capacitors
- Shield sensitive signals from motor noise

## Mechanical Considerations

### Wheel Orientation (Mecanum)
```
Front View:
    ╱╲  ╱╲
   ╱  ╲╱  ╲
  ╱   FL FR ╲
 │          │
  ╲   BL BR ╱
   ╲  ╱╲  ╱
    ╲╱  ╲╱

FL: Left-handed (╱)
FR: Right-handed (╲)
BL: Right-handed (╲)
BR: Left-handed (╱)
```

### Center of Mass
- Place battery low and centered
- Keep heavy components near center
- Balance weight distribution for stable movement

### Clearance
- Ensure adequate ground clearance (20mm minimum)
- Verify no component interference during movement
- Check wire routing doesn't impede wheel motion

## Maintenance

1. **Regular Checks**
   - Battery voltage and health
   - Wire connections and strain relief
   - Motor brush wear (if applicable)
   - Wheel alignment
   - Encoder operation

2. **Periodic Maintenance**
   - Clean encoders and sensors
   - Lubricate motor bearings if needed
   - Tighten all mechanical fasteners
   - Check for wire chafing
   - Update firmware as needed

## References

- MCXN947 Reference Manual
- ESP32C3 Datasheet
- L298N Motor Driver Datasheet
- Mecanum Wheel Kinematics Papers

## Appendix: Calculations

### Motor Speed Calculation
```
ω (rad/s) = (RPM × 2π) / 60
v (m/s) = ω × wheel_radius
```

### Battery Runtime Estimation
```
Runtime (hours) = Battery_Capacity (mAh) / Average_Current (mA)
Example: 3000mAh / 2500mA = 1.2 hours
```

### Power Dissipation
```
P (W) = V (V) × I (A)
Example: Motor driver at 12V, 2A = 24W
```

## License

See main LICENSE file in repository root.
