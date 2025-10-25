/**
 * @file comm_protocol.h
 * @brief Communication protocol between MCXN947 and ESP32C3
 * @author Ivan Sigala
 * @date 2025
 *
 * Defines the protocol for UART communication between the MCXN947 
 * main controller and ESP32C3 communication module.
 */

#ifndef COMM_PROTOCOL_H
#define COMM_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

// Protocol constants
#define COMM_START_BYTE     0xAA
#define COMM_END_BYTE       0x55
#define COMM_MAX_PAYLOAD    32
#define COMM_TIMEOUT_MS     1000

/**
 * @brief Command types
 */
typedef enum {
    CMD_VELOCITY = 0x01,        /**< Velocity command */
    CMD_EMERGENCY_STOP = 0x02,  /**< Emergency stop */
    CMD_ENABLE = 0x03,          /**< Enable/disable motors */
    CMD_SET_PID = 0x04,         /**< Set PID parameters */
    CMD_HEARTBEAT = 0x05,       /**< Heartbeat/keepalive */
    CMD_STATUS_REQUEST = 0x10,  /**< Request status */
    CMD_STATUS_RESPONSE = 0x11, /**< Status response */
} Command_Type_t;

/**
 * @brief Communication packet structure
 */
typedef struct __attribute__((packed)) {
    uint8_t start;              /**< Start byte (0xAA) */
    uint8_t cmd;                /**< Command type */
    uint8_t length;             /**< Payload length */
    uint8_t payload[COMM_MAX_PAYLOAD]; /**< Payload data */
    uint8_t checksum;           /**< Simple checksum */
    uint8_t end;                /**< End byte (0x55) */
} Comm_Packet_t;

/**
 * @brief Velocity command payload
 */
typedef struct __attribute__((packed)) {
    float vx;       /**< X velocity */
    float vy;       /**< Y velocity */
    float omega;    /**< Rotational velocity */
} Velocity_Payload_t;

/**
 * @brief PID parameters payload
 */
typedef struct __attribute__((packed)) {
    uint8_t motor_id;   /**< Motor ID (0-3 or 0xFF for all) */
    float kp;           /**< Proportional gain */
    float ki;           /**< Integral gain */
    float kd;           /**< Derivative gain */
} PID_Payload_t;

/**
 * @brief Status response payload
 */
typedef struct __attribute__((packed)) {
    uint8_t enabled;        /**< Motors enabled flag */
    float battery_voltage;  /**< Battery voltage */
    int16_t motor_pwm[4];   /**< PWM values for each motor */
    float motor_velocity[4]; /**< Velocity for each motor */
} Status_Payload_t;

/**
 * @brief Initialize communication protocol
 */
void Comm_Init(void);

/**
 * @brief Send a packet
 * 
 * @param cmd Command type
 * @param payload Pointer to payload data
 * @param length Payload length
 * @return true if sent successfully
 */
bool Comm_SendPacket(Command_Type_t cmd, const void *payload, uint8_t length);

/**
 * @brief Process received data
 * 
 * @param data Received byte
 * @return true if complete packet received
 */
bool Comm_ProcessByte(uint8_t data);

/**
 * @brief Get last received packet
 * 
 * @return Pointer to received packet
 */
const Comm_Packet_t* Comm_GetReceivedPacket(void);

/**
 * @brief Check if communication is alive
 * 
 * @param current_time Current time in milliseconds
 * @return true if communication is active
 */
bool Comm_IsAlive(uint32_t current_time);

/**
 * @brief Update last communication time (call when packet received)
 * 
 * @param current_time Current time in milliseconds
 */
void Comm_UpdateLastTime(uint32_t current_time);

/**
 * @brief Calculate checksum for packet
 * 
 * @param packet Pointer to packet
 * @return Calculated checksum
 */
uint8_t Comm_CalculateChecksum(const Comm_Packet_t *packet);

#endif /* COMM_PROTOCOL_H */
