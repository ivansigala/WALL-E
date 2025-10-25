/**
 * @file remote-control.ino
 * @brief ESP32 Remote Control for WALL-E Robot
 * @author Ivan Sigala
 * @date 2025
 *
 * Simple remote control using ESP32 with analog joystick or buttons.
 * Communicates with WALL-E robot via ESP-NOW protocol.
 */

#include <esp_now.h>
#include <WiFi.h>

// Configuration
#define JOYSTICK_X_PIN      34  // Analog pin for X axis
#define JOYSTICK_Y_PIN      35  // Analog pin for Y axis
#define BUTTON_ESTOP_PIN    25  // Emergency stop button
#define BUTTON_ENABLE_PIN   26  // Enable button
#define LED_PIN             2   // Status LED

#define ESPNOW_CHANNEL      1
#define SEND_INTERVAL       50  // ms (20 Hz update rate)
#define JOYSTICK_DEADZONE   200 // ADC units
#define JOYSTICK_CENTER     2048 // 12-bit ADC center
#define MAX_VELOCITY        1.0  // m/s

// MAC address of WALL-E ESP32C3 module (to be configured)
// Get this from the WALL-E ESP32C3 serial output
uint8_t walleMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Command types
#define CMD_VELOCITY        0x01
#define CMD_EMERGENCY_STOP  0x02
#define CMD_ENABLE          0x03

// ESP-NOW data structure
typedef struct {
  uint8_t cmd;
  uint8_t data[32];
  uint8_t length;
} ESPNowMessage_t;

// Velocity command payload
typedef struct __attribute__((packed)) {
  float vx;
  float vy;
  float omega;
} VelocityPayload_t;

// Global variables
bool espnowReady = false;
bool robotEnabled = false;
unsigned long lastSendTime = 0;
bool lastEstopState = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nWALL-E Remote Control");
  Serial.println("=====================");
  
  // Initialize pins
  pinMode(JOYSTICK_X_PIN, INPUT);
  pinMode(JOYSTICK_Y_PIN, INPUT);
  pinMode(BUTTON_ESTOP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ENABLE_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("Configure this MAC in WALL-E ESP32C3 firmware!");
  
  // Initialize ESP-NOW
  setupESPNow();
  
  // Blink LED to indicate ready
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  
  Serial.println("Remote control ready!");
  Serial.println("Press Enable button to start");
}

void loop() {
  unsigned long now = millis();
  
  // Read buttons
  bool estopPressed = digitalRead(BUTTON_ESTOP_PIN) == LOW;
  bool enablePressed = digitalRead(BUTTON_ENABLE_PIN) == LOW;
  
  // Handle emergency stop
  if (estopPressed && !lastEstopState) {
    Serial.println("EMERGENCY STOP!");
    sendEmergencyStop();
    robotEnabled = false;
    digitalWrite(LED_PIN, LOW);
  }
  lastEstopState = estopPressed;
  
  // Handle enable button
  static bool lastEnableState = false;
  if (enablePressed && !lastEnableState) {
    robotEnabled = !robotEnabled;
    Serial.print("Robot ");
    Serial.println(robotEnabled ? "ENABLED" : "DISABLED");
    sendEnable(robotEnabled);
    digitalWrite(LED_PIN, robotEnabled ? HIGH : LOW);
  }
  lastEnableState = enablePressed;
  
  // Send velocity commands at regular intervals
  if (robotEnabled && (now - lastSendTime >= SEND_INTERVAL)) {
    sendVelocityCommand();
    lastSendTime = now;
  }
  
  delay(10);
}

void setupESPNow() {
  Serial.print("Initializing ESP-NOW...");
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Failed!");
    return;
  }
  Serial.println("Success!");
  
  // Register send callback
  esp_now_register_send_cb(onESPNowDataSent);
  
  // Add peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, walleMacAddress, 6);
  peerInfo.channel = ESPNOW_CHANNEL;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  espnowReady = true;
  Serial.println("ESP-NOW ready");
}

void onESPNowDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  // Optional: Handle send status
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("Send failed");
  }
}

void sendVelocityCommand() {
  if (!espnowReady) return;
  
  // Read joystick values
  int rawX = analogRead(JOYSTICK_X_PIN);
  int rawY = analogRead(JOYSTICK_Y_PIN);
  
  // Apply deadzone and normalize
  float x = applyDeadzoneAndNormalize(rawX);
  float y = applyDeadzoneAndNormalize(rawY);
  
  // Create velocity command
  VelocityPayload_t vel;
  vel.vx = x * MAX_VELOCITY;      // Strafe velocity
  vel.vy = y * MAX_VELOCITY;      // Forward velocity
  vel.omega = 0.0f;                // No rotation (add second joystick if needed)
  
  // Send via ESP-NOW
  ESPNowMessage_t msg;
  msg.cmd = CMD_VELOCITY;
  msg.length = sizeof(VelocityPayload_t);
  memcpy(msg.data, &vel, sizeof(VelocityPayload_t));
  
  esp_now_send(walleMacAddress, (uint8_t*)&msg, 2 + msg.length);
  
  // Optional: Print for debugging
  if (abs(x) > 0.1 || abs(y) > 0.1) {
    Serial.print("Velocity: vx=");
    Serial.print(vel.vx, 2);
    Serial.print(" vy=");
    Serial.println(vel.vy, 2);
  }
}

void sendEmergencyStop() {
  if (!espnowReady) return;
  
  ESPNowMessage_t msg;
  msg.cmd = CMD_EMERGENCY_STOP;
  msg.length = 0;
  
  esp_now_send(walleMacAddress, (uint8_t*)&msg, 2);
}

void sendEnable(bool enable) {
  if (!espnowReady) return;
  
  ESPNowMessage_t msg;
  msg.cmd = CMD_ENABLE;
  msg.length = 1;
  msg.data[0] = enable ? 1 : 0;
  
  esp_now_send(walleMacAddress, (uint8_t*)&msg, 3);
}

float applyDeadzoneAndNormalize(int raw) {
  // Center the value
  int centered = raw - JOYSTICK_CENTER;
  
  // Apply deadzone
  if (abs(centered) < JOYSTICK_DEADZONE) {
    return 0.0f;
  }
  
  // Normalize to -1.0 to 1.0
  float normalized;
  if (centered > 0) {
    normalized = (float)(centered - JOYSTICK_DEADZONE) / (float)(JOYSTICK_CENTER - JOYSTICK_DEADZONE);
  } else {
    normalized = (float)(centered + JOYSTICK_DEADZONE) / (float)(JOYSTICK_CENTER - JOYSTICK_DEADZONE);
  }
  
  // Clamp to -1.0 to 1.0
  if (normalized > 1.0f) normalized = 1.0f;
  if (normalized < -1.0f) normalized = -1.0f;
  
  return normalized;
}
