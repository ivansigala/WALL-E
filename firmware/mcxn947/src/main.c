/**
 * @file main.c
 * @brief Main application for WALL-E robot controller
 * @author Ivan Sigala
 * @date 2025
 *
 * Main control loop for the MCXN947-based omnidirectional robot.
 * Integrates PID control, motor control, and communication with ESP32C3.
 */

#include "pid_controller.h"
#include "motor_control.h"
#include "comm_protocol.h"
#include <stdint.h>
#include <stdbool.h>

// System timing (placeholder - use actual timer)
static uint32_t system_time_ms = 0;

// Function prototypes
static void System_Init(void);
static void System_GetTime(void);
static void Process_Commands(void);
static void Safety_Check(void);

/**
 * @brief Main function
 */
int main(void) {
    // Initialize system
    System_Init();
    
    // Initialize subsystems
    Motor_Init();
    Comm_Init();
    
    // Enable motors
    Motor_Enable(true);
    
    // Main loop
    while (1) {
        // Update system time
        System_GetTime();
        
        // Process incoming commands from ESP32C3
        Process_Commands();
        
        // Update motor control with PID
        Motor_Update(system_time_ms);
        
        // Safety checks
        Safety_Check();
        
        // Small delay or use RTOS task delay
        // delay_ms(1);
    }
    
    return 0;
}

/**
 * @brief Initialize system peripherals
 */
static void System_Init(void) {
    // TODO: Initialize MCXN947 peripherals
    // - Clock configuration
    // - GPIO for motor drivers
    // - PWM for motor control
    // - UART for ESP32C3 communication
    // - Timers for encoders and system time
    // - ADC for battery monitoring
    
    // Example initialization sequence:
    // BOARD_InitPins();
    // BOARD_InitBootClocks();
    // BOARD_InitDebugConsole();
    // PWM_Init();
    // UART_Init();
    // TIMER_Init();
}

/**
 * @brief Get current system time
 */
static void System_GetTime(void) {
    // TODO: Get time from hardware timer
    // system_time_ms = TIMER_GetMilliseconds();
    
    // Placeholder - increment (this would be handled by timer ISR)
    system_time_ms++;
}

/**
 * @brief Process commands received from ESP32C3
 */
static void Process_Commands(void) {
    const Comm_Packet_t *packet = Comm_GetReceivedPacket();
    
    if (packet == NULL) return;
    
    switch (packet->cmd) {
        case CMD_VELOCITY: {
            if (packet->length == sizeof(Velocity_Payload_t)) {
                Velocity_Payload_t *vel_cmd = (Velocity_Payload_t*)packet->payload;
                
                Velocity_Command_t cmd;
                cmd.vx = vel_cmd->vx;
                cmd.vy = vel_cmd->vy;
                cmd.omega = vel_cmd->omega;
                
                Motor_SetVelocityCommand(&cmd);
                Comm_UpdateLastTime(system_time_ms);
            }
            break;
        }
        
        case CMD_EMERGENCY_STOP: {
            Motor_EmergencyStop();
            Comm_UpdateLastTime(system_time_ms);
            break;
        }
        
        case CMD_ENABLE: {
            if (packet->length >= 1) {
                bool enable = packet->payload[0] != 0;
                Motor_Enable(enable);
                Comm_UpdateLastTime(system_time_ms);
            }
            break;
        }
        
        case CMD_SET_PID: {
            if (packet->length == sizeof(PID_Payload_t)) {
                PID_Payload_t *pid_params = (PID_Payload_t*)packet->payload;
                
                // Apply to specific motor or all motors
                if (pid_params->motor_id == 0xFF) {
                    // Apply to all motors
                    for (int i = 0; i < NUM_MOTORS; i++) {
                        Motor_t *motor = Motor_Get((Motor_ID_t)i);
                        if (motor != NULL) {
                            PID_SetGains(&motor->pid, pid_params->kp, pid_params->ki, pid_params->kd);
                        }
                    }
                } else if (pid_params->motor_id < NUM_MOTORS) {
                    // Apply to specific motor
                    Motor_t *motor = Motor_Get((Motor_ID_t)pid_params->motor_id);
                    if (motor != NULL) {
                        PID_SetGains(&motor->pid, pid_params->kp, pid_params->ki, pid_params->kd);
                    }
                }
                Comm_UpdateLastTime(system_time_ms);
            }
            break;
        }
        
        case CMD_HEARTBEAT: {
            Comm_UpdateLastTime(system_time_ms);
            break;
        }
        
        case CMD_STATUS_REQUEST: {
            // Send status response
            Status_Payload_t status;
            status.enabled = Motor_Get(MOTOR_FRONT_LEFT)->enabled ? 1 : 0;
            status.battery_voltage = 12.0f; // TODO: Read from ADC
            
            for (int i = 0; i < NUM_MOTORS; i++) {
                Motor_t *motor = Motor_Get((Motor_ID_t)i);
                if (motor != NULL) {
                    status.motor_pwm[i] = motor->pwm_value;
                    status.motor_velocity[i] = motor->current_velocity;
                }
            }
            
            Comm_SendPacket(CMD_STATUS_RESPONSE, &status, sizeof(Status_Payload_t));
            Comm_UpdateLastTime(system_time_ms);
            break;
        }
        
        default:
            break;
    }
}

/**
 * @brief Safety checks
 */
static void Safety_Check(void) {
    // Check communication timeout
    if (!Comm_IsAlive(system_time_ms)) {
        // Communication lost - emergency stop
        Motor_EmergencyStop();
    }
    
    // TODO: Add more safety checks
    // - Battery voltage monitoring
    // - Overcurrent detection
    // - Temperature monitoring
    // - Motor stall detection
}

// UART receive callback (to be called from UART ISR)
void UART_RxCallback(uint8_t data) {
    if (Comm_ProcessByte(data)) {
        // Complete packet received, will be processed in main loop
    }
}
