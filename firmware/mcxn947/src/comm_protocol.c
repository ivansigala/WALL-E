/**
 * @file comm_protocol.c
 * @brief Communication protocol implementation
 * @author Ivan Sigala
 * @date 2025
 */

#include "comm_protocol.h"
#include <string.h>

// Reception state machine
typedef enum {
    STATE_WAIT_START,
    STATE_WAIT_CMD,
    STATE_WAIT_LENGTH,
    STATE_WAIT_PAYLOAD,
    STATE_WAIT_CHECKSUM,
    STATE_WAIT_END
} Rx_State_t;

// Static variables
static Comm_Packet_t rx_packet;
static Comm_Packet_t last_packet;
static Rx_State_t rx_state = STATE_WAIT_START;
static uint8_t rx_payload_index = 0;
static uint32_t last_comm_time = 0;

// Hardware abstraction (to be implemented)
static void HAL_UART_Transmit(const uint8_t *data, uint16_t length);

void Comm_Init(void) {
    rx_state = STATE_WAIT_START;
    rx_payload_index = 0;
    last_comm_time = 0;
    memset(&rx_packet, 0, sizeof(Comm_Packet_t));
    memset(&last_packet, 0, sizeof(Comm_Packet_t));
}

uint8_t Comm_CalculateChecksum(const Comm_Packet_t *packet) {
    if (packet == NULL) return 0;
    
    uint8_t checksum = 0;
    checksum ^= packet->cmd;
    checksum ^= packet->length;
    
    for (int i = 0; i < packet->length && i < COMM_MAX_PAYLOAD; i++) {
        checksum ^= packet->payload[i];
    }
    
    return checksum;
}

bool Comm_SendPacket(Command_Type_t cmd, const void *payload, uint8_t length) {
    if (length > COMM_MAX_PAYLOAD) return false;
    
    Comm_Packet_t packet;
    packet.start = COMM_START_BYTE;
    packet.cmd = cmd;
    packet.length = length;
    
    if (payload != NULL && length > 0) {
        memcpy(packet.payload, payload, length);
    }
    
    packet.checksum = Comm_CalculateChecksum(&packet);
    packet.end = COMM_END_BYTE;
    
    // Calculate total packet size
    uint16_t packet_size = 5 + length; // start + cmd + length + checksum + end + payload
    
    // Transmit packet
    HAL_UART_Transmit((const uint8_t*)&packet, packet_size);
    
    return true;
}

bool Comm_ProcessByte(uint8_t data) {
    switch (rx_state) {
        case STATE_WAIT_START:
            if (data == COMM_START_BYTE) {
                rx_packet.start = data;
                rx_state = STATE_WAIT_CMD;
            }
            break;
            
        case STATE_WAIT_CMD:
            rx_packet.cmd = data;
            rx_state = STATE_WAIT_LENGTH;
            break;
            
        case STATE_WAIT_LENGTH:
            if (data <= COMM_MAX_PAYLOAD) {
                rx_packet.length = data;
                rx_payload_index = 0;
                
                if (data == 0) {
                    rx_state = STATE_WAIT_CHECKSUM;
                } else {
                    rx_state = STATE_WAIT_PAYLOAD;
                }
            } else {
                // Invalid length, reset
                rx_state = STATE_WAIT_START;
            }
            break;
            
        case STATE_WAIT_PAYLOAD:
            rx_packet.payload[rx_payload_index++] = data;
            
            if (rx_payload_index >= rx_packet.length) {
                rx_state = STATE_WAIT_CHECKSUM;
            }
            break;
            
        case STATE_WAIT_CHECKSUM:
            rx_packet.checksum = data;
            rx_state = STATE_WAIT_END;
            break;
            
        case STATE_WAIT_END:
            if (data == COMM_END_BYTE) {
                rx_packet.end = data;
                
                // Verify checksum
                uint8_t calculated_checksum = Comm_CalculateChecksum(&rx_packet);
                
                if (calculated_checksum == rx_packet.checksum) {
                    // Valid packet received
                    memcpy(&last_packet, &rx_packet, sizeof(Comm_Packet_t));
                    rx_state = STATE_WAIT_START;
                    return true;
                }
            }
            
            // Invalid end byte or checksum mismatch, reset
            rx_state = STATE_WAIT_START;
            break;
    }
    
    return false;
}

const Comm_Packet_t* Comm_GetReceivedPacket(void) {
    return &last_packet;
}

bool Comm_IsAlive(uint32_t current_time) {
    if (last_comm_time == 0) return false;
    
    uint32_t elapsed = current_time - last_comm_time;
    return elapsed < COMM_TIMEOUT_MS;
}

void Comm_UpdateLastTime(uint32_t current_time) {
    last_comm_time = current_time;
}

// Hardware abstraction implementation (placeholder)
static void HAL_UART_Transmit(const uint8_t *data, uint16_t length) {
    // TODO: Implement with MCXN947 UART peripheral
    // Example: UART_WriteBlocking(UART0, data, length);
}
