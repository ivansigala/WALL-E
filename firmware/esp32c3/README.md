# ESP32C3 Communication Firmware

Communication module firmware for the WALL-E robot using ESP32C3 Super Mini board with ESP-NOW protocol.

## Features

- **ESP-NOW Protocol**: Low-latency wireless communication
- **Bidirectional Communication**: Commands from remote, status to remote
- **UART Bridge**: Seamless communication with MCXN947 controller
- **Auto-Reconnection**: Handles connection drops gracefully
- **Heartbeat Monitoring**: Ensures communication health

## Hardware Requirements

- ESP32C3 Super Mini board
- UART connection to MCXN947 (TX, RX, GND)
- Power supply (3.3V or 5V via USB)
- ESP32-based remote control

## Pin Configuration

### ESP32C3 Super Mini Default Pins
- **UART1 TX**: GPIO21 (to MCXN947 RX)
- **UART1 RX**: GPIO20 (to MCXN947 TX)
- **Status LED**: GPIO8 (built-in)
- **USB**: GPIO18 (D-), GPIO19 (D+)

## Project Structure

```
esp32c3/
├── src/
│   ├── wall-e-esp32c3.ino    # Main ESP32C3 firmware
│   └── remote-control.ino     # Remote control firmware
└── README.md                  # This file
```

## Building and Flashing

### Using Arduino IDE

1. **Install ESP32 Board Support**
   ```
   - Open Arduino IDE
   - File → Preferences
   - Add to "Additional Board Manager URLs":
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   - Tools → Board → Boards Manager
   - Search "esp32" and install "esp32 by Espressif Systems"
   ```

2. **Select Board**
   ```
   - Tools → Board → ESP32 Arduino → ESP32C3 Dev Module
   ```

3. **Configure Settings**
   ```
   - Upload Speed: 921600
   - USB CDC On Boot: Enabled
   - CPU Frequency: 160MHz
   - Flash Size: 4MB
   - Partition Scheme: Default
   ```

4. **Install Libraries**
   - ESP-NOW is included in ESP32 core (no additional library needed)

5. **Open Sketch**
   - Open `src/wall-e-esp32c3.ino`

6. **Upload**
   - Connect ESP32C3 via USB-C
   - Select the correct COM port
   - Click Upload button
   - Hold BOOT button if upload fails

### Using PlatformIO

1. **Create platformio.ini**
   ```ini
   [env:esp32-c3-devkitm-1]
   platform = espressif32
   board = esp32-c3-devkitm-1
   framework = arduino
   monitor_speed = 115200
   upload_speed = 921600
   ```

2. **Build and Upload**
   ```bash
   pio run -t upload
   pio device monitor
   ```

## Configuration

### WALL-E ESP32C3 Module

1. **Flash Firmware**
   - Upload `wall-e-esp32c3.ino` to ESP32C3 Super Mini on robot

2. **Get MAC Address**
   - Open Serial Monitor (115200 baud)
   - Note the MAC address displayed on startup
   - Example: `24:6F:28:XX:XX:XX`

3. **Configure UART**
   - Default baud rate: 115200
   - Connect to MCXN947:
     - ESP32 GPIO21 (TX) → MCXN947 RX
     - ESP32 GPIO20 (RX) → MCXN947 TX
     - GND → GND

### Remote Control

1. **Flash Firmware**
   - Upload `remote-control.ino` to remote ESP32

2. **Configure MAC Address**
   - Edit `remote-control.ino`
   - Replace `walleMacAddress` with WALL-E's MAC address
   - Example: `{0x24, 0x6F, 0x28, 0xXX, 0xXX, 0xXX}`

3. **Connect Hardware**
   - Analog joystick to pins 34, 35
   - Emergency stop button to pin 25
   - Enable button to pin 26

4. **Pair Devices**
   - Both devices will auto-pair on first communication
   - LED will indicate connection status

## ESP-NOW Protocol

### Message Structure
```c
typedef struct {
  uint8_t cmd;        // Command type
  uint8_t data[32];   // Payload data
  uint8_t length;     // Payload length
} ESPNowMessage_t;
```

### Command Types
- `0x01`: Velocity Command (vx, vy, omega)
- `0x02`: Emergency Stop
- `0x03`: Enable/Disable Motors
- `0x04`: Set PID Parameters
- `0x05`: Heartbeat
- `0x10`: Status Request
- `0x11`: Status Response

### Communication Flow
```
Remote → [ESP-NOW] → ESP32C3 → [UART] → MCXN947
MCXN947 → [UART] → ESP32C3 → [ESP-NOW] → Remote
```

## Testing

### Serial Monitor Test
1. Open Serial Monitor at 115200 baud
2. Verify "ESP-NOW ready" message
3. Check MAC address display
4. Send test commands from remote
5. Verify message forwarding

### Communication Test
1. Power on robot and remote
2. Check LED indicators
3. Press enable button on remote
4. Move joystick and verify motor response
5. Test emergency stop

### Range Test
1. Enable robot
2. Gradually increase distance
3. ESP-NOW typical range: 100-200m (line of sight)
4. Expect degradation with obstacles

## Troubleshooting

### ESP-NOW initialization failed
- Check WiFi is set to STA mode
- Verify ESP32 board support is installed correctly
- Try power cycling the device

### No communication with MCXN947
- Verify UART connections (TX-RX, RX-TX)
- Check baud rate matches (115200)
- Use logic analyzer or oscilloscope to verify signals
- Test UART loopback (connect TX to RX)

### Cannot upload firmware
- Hold BOOT button during upload
- Try different USB cable
- Check COM port selection
- Reduce upload speed to 115200

### Remote control not responding
- Verify MAC addresses are configured correctly
- Check ESP-NOW channel matches
- Ensure both devices are on same WiFi channel
- Test with broadcast address (0xFF:FF:FF:FF:FF:FF)

### Intermittent connection
- Check antenna orientation
- Reduce distance
- Remove obstacles
- Add external antenna if needed

## Advanced Features

### Custom Commands
Add new command types in both firmware files:
```cpp
#define CMD_CUSTOM 0x20

// In remote control
ESPNowMessage_t msg;
msg.cmd = CMD_CUSTOM;
msg.data[0] = custom_value;
msg.length = 1;
esp_now_send(walleMacAddress, (uint8_t*)&msg, 3);
```

### Status Monitoring
Request and display robot status:
```cpp
// Send status request
msg.cmd = CMD_STATUS_REQUEST;
msg.length = 0;
esp_now_send(walleMacAddress, (uint8_t*)&msg, 2);

// Receive and parse status response
void onESPNowDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
  ESPNowMessage_t *msg = (ESPNowMessage_t*)data;
  if (msg->cmd == CMD_STATUS_RESPONSE) {
    // Parse status data
  }
}
```

### Encryption (Optional)
Enable ESP-NOW encryption for secure communication:
```cpp
peerInfo.encrypt = true;
memcpy(peerInfo.lmk, "YourEncryptionKey", 16);
```

## Hardware Wiring

### WALL-E ESP32C3 to MCXN947
```
ESP32C3          MCXN947
-------          -------
GPIO21 (TX) ---> RX Pin
GPIO20 (RX) <--- TX Pin
GND         ---> GND
3.3V/5V     ---> 3.3V/5V
```

### Remote Control ESP32
```
Component        ESP32 Pin
---------        ---------
Joystick VRx --> GPIO34 (ADC)
Joystick VRy --> GPIO35 (ADC)
E-Stop Button -> GPIO25 (INPUT_PULLUP)
Enable Button -> GPIO26 (INPUT_PULLUP)
Status LED    -> GPIO2 (OUTPUT)
Power         -> 3.3V/5V
Ground        -> GND
```

## Performance

- **Latency**: ~10-20ms (ESP-NOW + UART + processing)
- **Update Rate**: 20 Hz (configurable)
- **Range**: 100-200m (line of sight)
- **Packet Loss**: <1% in good conditions

## Safety Notes

- Always test emergency stop before operation
- Monitor battery voltage on remote
- Implement communication timeout on robot
- Add physical emergency stop button
- Test in safe environment first

## Future Enhancements

- [ ] Add OLED display for status
- [ ] Implement telemetry logging
- [ ] Add battery voltage monitoring on remote
- [ ] Multi-remote support
- [ ] OTA firmware updates
- [ ] WiFi configuration portal
- [ ] Mobile app control via BLE

## License

See main LICENSE file in repository root.
