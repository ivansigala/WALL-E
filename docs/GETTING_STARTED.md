# Getting Started with WALL-E

Quick start guide to get your WALL-E omnidirectional robot up and running.

## Prerequisites

### Hardware
- ✓ All components from the Bill of Materials (see hardware/HARDWARE.md)
- ✓ Assembled robot with all connections made
- ✓ Charged battery
- ✓ USB cables for programming

### Software
- ✓ MCUXpresso IDE (for MCXN947)
- ✓ Arduino IDE or PlatformIO (for ESP32)
- ✓ USB-to-Serial drivers installed
- ✓ Git (to clone the repository)

### Knowledge
- Basic embedded systems concepts
- C programming
- Arduino/ESP32 basics
- Electronic circuit basics

## Step-by-Step Setup

### 1. Clone the Repository

```bash
git clone https://github.com/ivansigala/WALL-E.git
cd WALL-E
```

### 2. Build and Flash MCXN947 Firmware

#### Using MCUXpresso IDE

1. Open MCUXpresso IDE
2. File → Import → Existing Projects
3. Browse to `firmware/mcxn947`
4. Click Finish
5. Right-click project → Build Project
6. Connect MCXN947 via debugger
7. Right-click project → Debug As → MCUXpresso IDE LinkServer
8. Click Run

#### Initial Configuration

Before building, you may need to adjust:
- Pin assignments in HAL functions
- Robot physical parameters (wheel base, radius)
- PID initial gains
- UART configuration

See `firmware/mcxn947/README.md` for details.

### 3. Build and Flash ESP32C3 Firmware (Robot)

#### Using Arduino IDE

1. Open Arduino IDE
2. Install ESP32 board support:
   - File → Preferences
   - Add URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager
   - Install "esp32"

3. Configure board:
   - Tools → Board → ESP32C3 Dev Module
   - Tools → Upload Speed → 921600
   - Tools → USB CDC On Boot → Enabled

4. Open sketch:
   - File → Open → `firmware/esp32c3/src/wall-e-esp32c3.ino`

5. Upload:
   - Connect ESP32C3 via USB-C
   - Select correct COM port
   - Click Upload
   - Hold BOOT button if needed

6. Get MAC address:
   - Open Serial Monitor (115200 baud)
   - Note the MAC address displayed
   - Example: `24:6F:28:XX:XX:XX`

### 4. Build and Flash Remote Control Firmware

#### Using Arduino IDE

1. Open `firmware/esp32c3/src/remote-control.ino`

2. Configure MAC address:
   ```cpp
   // Replace with your WALL-E ESP32C3 MAC address
   uint8_t walleMacAddress[] = {0x24, 0x6F, 0x28, 0xXX, 0xXX, 0xXX};
   ```

3. Upload to remote ESP32

4. Connect hardware:
   - Joystick to pins 34, 35
   - E-Stop button to pin 25
   - Enable button to pin 26
   - LED to pin 2

### 5. Hardware Verification

Before powering on, verify:

- [ ] All motor connections secure
- [ ] Power connections correct polarity
- [ ] No shorts between power and ground
- [ ] UART connections (TX-RX, RX-TX)
- [ ] Battery charged and secured
- [ ] Emergency stop accessible
- [ ] All screws tightened

### 6. Power On Sequence

1. **Remote Control First**
   - Power on remote
   - Wait for LED blink (ready indicator)
   - Serial monitor should show "ESP-NOW ready"

2. **Robot Second**
   - Turn on main power switch
   - LEDs should illuminate
   - Listen for motors (should be quiet)
   - Check serial output from ESP32C3

3. **Verify Communication**
   - ESP32C3 serial should show heartbeat
   - No error messages
   - LED pattern indicates status

### 7. First Movement Test

⚠️ **Safety First:**
- Test in open area
- Have emergency stop ready
- Start with low speed
- Keep hands clear of wheels

#### Test Procedure

1. **Enable System**
   - Press Enable button on remote
   - LED should turn on steady
   - Robot should be ready

2. **Test Forward/Backward**
   - Gently push joystick forward
   - Robot should move forward
   - Release joystick - robot stops
   - Pull joystick backward
   - Robot should move backward

3. **Test Strafe**
   - Push joystick left
   - Robot should strafe left
   - Push joystick right
   - Robot should strafe right

4. **Test Rotation** (if second joystick available)
   - Test clockwise rotation
   - Test counter-clockwise rotation

5. **Test Emergency Stop**
   - Press E-Stop button
   - Robot should stop immediately
   - Press Enable again to reset

### 8. PID Tuning

Initial PID values may need tuning:

#### Tuning Process

1. **Start with P-only** (Ki=0, Kd=0)
   ```cpp
   Motor_InitSingle(&motor, id, 1.0f, 0.0f, 0.0f);
   ```

2. **Increase Kp** until oscillation occurs
   - Too low: Slow response
   - Too high: Oscillation
   - Target: ~70% of oscillation point

3. **Add Derivative (Kd)**
   ```cpp
   Motor_InitSingle(&motor, id, Kp, 0.0f, 0.05f);
   ```
   - Reduces oscillation
   - Improves stability

4. **Add Integral (Ki)**
   ```cpp
   Motor_InitSingle(&motor, id, Kp, 0.1f, Kd);
   ```
   - Eliminates steady-state error
   - Start with small value

5. **Fine-tune** all three gains
   - Test different movement patterns
   - Adjust for smooth operation

#### Example Values

Start with these and adjust:
```cpp
Kp = 1.0
Ki = 0.1
Kd = 0.05
```

## Troubleshooting

### Robot doesn't move

**Check:**
- Motors enabled (Enable button pressed)
- Power connections
- Motor driver outputs
- PWM signals (oscilloscope)

**Try:**
- Re-enable system
- Check serial output for errors
- Test motors individually

### Movement is jerky

**Likely cause:** PID tuning
- Reduce Kp gain
- Add more Kd damping
- Check encoder signals

### Communication lost

**Check:**
- Distance between remote and robot
- ESP32C3 serial output
- UART connections
- Power supply stability

**Try:**
- Move closer
- Power cycle both devices
- Re-pair devices

### Motors turn but robot doesn't move

**Check:**
- Wheel orientation (mecanum)
- Motor connections (may be reversed)
- All four motors working
- Adequate power supply

**Try:**
- Test each motor individually
- Verify kinematic calculations

## Next Steps

### Optimization
1. Fine-tune PID parameters
2. Adjust velocity limits
3. Calibrate joystick deadzone
4. Test different movement patterns

### Enhancements
1. Add sensors (IMU, ultrasonic)
2. Implement autonomous features
3. Add telemetry logging
4. Create mobile app interface

### Testing
1. Test at different speeds
2. Measure battery life
3. Test communication range
4. Stress test with obstacles

## Tips and Best Practices

### Development
- Use version control (git)
- Comment your code
- Keep backups of working firmware
- Document changes and tuning

### Testing
- Always test in safe environment
- Start with low power/speed
- Verify safety features work
- Test edge cases

### Maintenance
- Check connections regularly
- Monitor battery health
- Keep firmware updated
- Clean encoders and sensors

## Common Commands

### Git
```bash
# Get latest code
git pull origin main

# Create branch for changes
git checkout -b feature/my-feature

# Commit changes
git add .
git commit -m "Description"
git push origin feature/my-feature
```

### Arduino CLI (Alternative)
```bash
# Compile
arduino-cli compile --fqbn esp32:esp32:esp32c3

# Upload
arduino-cli upload -p COM3 --fqbn esp32:esp32:esp32c3

# Monitor
arduino-cli monitor -p COM3 -c baudrate=115200
```

### Debugging
```bash
# Serial monitor
screen /dev/ttyUSB0 115200

# Or using minicom
minicom -D /dev/ttyUSB0 -b 115200
```

## Resources

### Documentation
- [Architecture](docs/ARCHITECTURE.md) - System design
- [Hardware](hardware/HARDWARE.md) - Wiring and assembly
- [MCXN947 Firmware](firmware/mcxn947/README.md)
- [ESP32C3 Firmware](firmware/esp32c3/README.md)

### External Resources
- [MCXN947 SDK](https://www.nxp.com/mcxn947)
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/)
- [ESP-NOW Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- [PID Tuning Guide](http://www.pidtuningguide.com/)

### Community
- GitHub Issues: Report bugs and request features
- Discussions: Share your builds and ask questions

## Safety Reminders

- ⚠️ Always have emergency stop accessible
- ⚠️ Test in safe, open area
- ⚠️ Monitor battery voltage
- ⚠️ Keep firmware updated with safety fixes
- ⚠️ Follow proper electrical safety procedures

## Success Checklist

Before considering your robot complete:

- [ ] All firmware compiled and flashed
- [ ] Hardware connections verified
- [ ] Power system tested
- [ ] Communication working
- [ ] All motors responding correctly
- [ ] Emergency stop functional
- [ ] PID tuned for smooth motion
- [ ] All four movement directions work
- [ ] Safety features verified
- [ ] Documentation reviewed

## License

See main LICENSE file in repository root.

---

**Congratulations!** You now have a working omnidirectional robot. Enjoy experimenting and learning!
