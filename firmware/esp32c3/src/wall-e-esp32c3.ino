/**
 * @file wall-e-esp32c3.ino
 * @brief ESP32C3 Communication Module for WALL-E Robot
 * @author Ivan Sigala
 * @date 2025
 *
 * ESP32C3 Super Mini firmware for wireless communication between
 * remote control and MCXN947 main controller using ESP-NOW protocol.
 */

#include <esp_now.h>
#include <WiFi.h>
#include <HardwareSerial.h>

// Configuration
#define UART_BAUD_RATE      115200
#define ESPNOW_CHANNEL      1
#define MAX_RETRIES         3
#define HEARTBEAT_INTERVAL  500  // ms

// Communication protocol constants (must match MCXN947 firmware)
#define COMM_START_BYTE     0xAA
#define COMM_END_BYTE       0x55
#define COMM_MAX_PAYLOAD    32

// Command types
#define CMD_VELOCITY        0x01
#define CMD_EMERGENCY_STOP  0x02
#define CMD_ENABLE          0x03
#define CMD_SET_PID         0x04
#define CMD_HEARTBEAT       0x05
#define CMD_STATUS_REQUEST  0x10
#define CMD_STATUS_RESPONSE 0x11

// MAC address of the remote control (to be paired)
// Set to broadcast address initially for pairing
uint8_t remoteMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Packet structure
typedef struct __attribute__((packed)) {
  uint8_t start;
  uint8_t cmd;
  uint8_t length;
  uint8_t payload[COMM_MAX_PAYLOAD];
  uint8_t checksum;
  uint8_t end;
} CommPacket_t;

// ESP-NOW data structure
typedef struct {
  uint8_t cmd;
  uint8_t data[32];
  uint8_t length;
} ESPNowMessage_t;

// Global variables
bool espnowReady = false;
bool uartReady = false;
unsigned long lastHeartbeat = 0;
unsigned long lastDataReceived = 0;

// Function prototypes
void setupESPNow();
void setupUART();
void onESPNowDataReceived(const uint8_t *mac, const uint8_t *data, int len);
void onESPNowDataSent(const uint8_t *mac, esp_now_send_status_t status);
void processUARTData();
void sendPacketToMCU(uint8_t cmd, const uint8_t *payload, uint8_t length);
void sendPacketToRemote(uint8_t cmd, const uint8_t *payload, uint8_t length);
uint8_t calculateChecksum(const CommPacket_t *packet);
void forwardToMCU(const ESPNowMessage_t *msg);
void forwardToRemote(const CommPacket_t *packet);
void sendHeartbeat();

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nWALL-E ESP32C3 Communication Module");
  Serial.println("====================================");
  
  // Initialize UART to MCXN947
  setupUART();
  
  // Initialize ESP-NOW
  setupESPNow();
  
  Serial.println("Setup complete!");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // Process incoming UART data from MCXN947
  processUARTData();
  
  // Send periodic heartbeat
  unsigned long now = millis();
  if (now - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    sendHeartbeat();
    lastHeartbeat = now;
  }
  
  // Check for communication timeout
  if (espnowReady && (now - lastDataReceived > 5000)) {
    // No data received for 5 seconds - could indicate problem
    // Optional: implement reconnection logic
  }
  
  delay(1); // Small delay to prevent watchdog issues
}

void setupUART() {
  // UART1 for communication with MCXN947
  // TX: GPIO21, RX: GPIO20 (default for ESP32C3 Super Mini)
  Serial1.begin(UART_BAUD_RATE);
  uartReady = true;
  Serial.println("UART initialized");
}

void setupESPNow() {
  // Set WiFi mode to STA
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  Serial.print("Initializing ESP-NOW...");
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Failed!");
    return;
  }
  Serial.println("Success!");
  
  // Register callbacks
  esp_now_register_recv_cb(onESPNowDataReceived);
  esp_now_register_send_cb(onESPNowDataSent);
  
  // Add peer (broadcast for initial pairing)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, remoteMacAddress, 6);
  peerInfo.channel = ESPNOW_CHANNEL;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  espnowReady = true;
  Serial.println("ESP-NOW ready for communication");
}

void onESPNowDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
  if (!uartReady || len < 2) return;
  
  lastDataReceived = millis();
  
  // Parse ESP-NOW message
  ESPNowMessage_t msg;
  if (len > sizeof(ESPNowMessage_t)) len = sizeof(ESPNowMessage_t);
  memcpy(&msg, data, len);
  
  // Forward to MCXN947
  forwardToMCU(&msg);
  
  Serial.print("Received from remote: CMD=0x");
  Serial.print(msg.cmd, HEX);
  Serial.print(" Length=");
  Serial.println(msg.length);
}

void onESPNowDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("ESP-NOW send success");
  } else {
    Serial.println("ESP-NOW send failed");
  }
}

void forwardToMCU(const ESPNowMessage_t *msg) {
  if (!uartReady || msg == NULL) return;
  
  sendPacketToMCU(msg->cmd, msg->data, msg->length);
}

void forwardToRemote(const CommPacket_t *packet) {
  if (!espnowReady || packet == NULL) return;
  
  ESPNowMessage_t msg;
  msg.cmd = packet->cmd;
  msg.length = packet->length;
  if (packet->length > 0 && packet->length <= 32) {
    memcpy(msg.data, packet->payload, packet->length);
  }
  
  sendPacketToRemote(msg.cmd, msg.data, msg.length);
}

void sendPacketToMCU(uint8_t cmd, const uint8_t *payload, uint8_t length) {
  if (!uartReady) return;
  
  CommPacket_t packet;
  packet.start = COMM_START_BYTE;
  packet.cmd = cmd;
  packet.length = length;
  
  if (payload != NULL && length > 0 && length <= COMM_MAX_PAYLOAD) {
    memcpy(packet.payload, payload, length);
  }
  
  packet.checksum = calculateChecksum(&packet);
  packet.end = COMM_END_BYTE;
  
  // Send via UART
  uint16_t packet_size = 5 + length;
  Serial1.write((uint8_t*)&packet, packet_size);
}

void sendPacketToRemote(uint8_t cmd, const uint8_t *payload, uint8_t length) {
  if (!espnowReady) return;
  
  ESPNowMessage_t msg;
  msg.cmd = cmd;
  msg.length = length;
  
  if (payload != NULL && length > 0 && length <= 32) {
    memcpy(msg.data, payload, length);
  }
  
  // Send via ESP-NOW
  esp_err_t result = esp_now_send(remoteMacAddress, (uint8_t*)&msg, 2 + length);
  
  if (result != ESP_OK) {
    Serial.println("Error sending ESP-NOW packet");
  }
}

uint8_t calculateChecksum(const CommPacket_t *packet) {
  if (packet == NULL) return 0;
  
  uint8_t checksum = 0;
  checksum ^= packet->cmd;
  checksum ^= packet->length;
  
  for (int i = 0; i < packet->length && i < COMM_MAX_PAYLOAD; i++) {
    checksum ^= packet->payload[i];
  }
  
  return checksum;
}

void processUARTData() {
  static uint8_t state = 0;
  static CommPacket_t rxPacket;
  static uint8_t payloadIndex = 0;
  
  while (Serial1.available()) {
    uint8_t data = Serial1.read();
    
    switch (state) {
      case 0: // Wait for start byte
        if (data == COMM_START_BYTE) {
          rxPacket.start = data;
          state = 1;
        }
        break;
        
      case 1: // Receive command
        rxPacket.cmd = data;
        state = 2;
        break;
        
      case 2: // Receive length
        if (data <= COMM_MAX_PAYLOAD) {
          rxPacket.length = data;
          payloadIndex = 0;
          state = (data == 0) ? 3 : 4;
        } else {
          state = 0;
        }
        break;
        
      case 3: // Receive checksum (no payload)
        rxPacket.checksum = data;
        state = 5;
        break;
        
      case 4: // Receive payload
        rxPacket.payload[payloadIndex++] = data;
        if (payloadIndex >= rxPacket.length) {
          state = 3;
        }
        break;
        
      case 5: // Receive end byte
        if (data == COMM_END_BYTE) {
          rxPacket.end = data;
          
          // Verify checksum
          if (calculateChecksum(&rxPacket) == rxPacket.checksum) {
            // Valid packet - forward to remote
            forwardToRemote(&rxPacket);
            
            Serial.print("Forwarding to remote: CMD=0x");
            Serial.println(rxPacket.cmd, HEX);
          }
        }
        state = 0;
        break;
    }
  }
}

void sendHeartbeat() {
  if (!uartReady) return;
  
  // Send heartbeat to MCU
  sendPacketToMCU(CMD_HEARTBEAT, NULL, 0);
}
