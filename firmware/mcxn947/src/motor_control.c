/**
 * @file motor_control.c
 * @brief Motor control implementation
 * @author Ivan Sigala
 * @date 2025
 */

#include "motor_control.h"
#include <string.h>
#include <math.h>

// Robot physical parameters (adjust based on actual robot)
#define WHEEL_BASE_WIDTH  0.20f   // Distance between left and right wheels (meters)
#define WHEEL_BASE_LENGTH 0.20f   // Distance between front and back wheels (meters)
#define WHEEL_RADIUS      0.05f   // Wheel radius (meters)

// Global motor array
static Motor_t motors[NUM_MOTORS];

// Hardware abstraction functions (to be implemented based on MCXN947 HAL)
// These are placeholders and should be replaced with actual hardware calls
static void HAL_SetMotorPWM(Motor_ID_t id, int16_t pwm);
static void HAL_SetMotorDirection(Motor_ID_t id, bool forward);

void Motor_Init(void) {
    // Initialize all motors with default PID parameters
    // These values should be tuned for your specific robot
    Motor_InitSingle(&motors[MOTOR_FRONT_LEFT], MOTOR_FRONT_LEFT, 1.0f, 0.1f, 0.05f);
    Motor_InitSingle(&motors[MOTOR_FRONT_RIGHT], MOTOR_FRONT_RIGHT, 1.0f, 0.1f, 0.05f);
    Motor_InitSingle(&motors[MOTOR_BACK_LEFT], MOTOR_BACK_LEFT, 1.0f, 0.1f, 0.05f);
    Motor_InitSingle(&motors[MOTOR_BACK_RIGHT], MOTOR_BACK_RIGHT, 1.0f, 0.1f, 0.05f);
}

void Motor_InitSingle(Motor_t *motor, Motor_ID_t id, float kp, float ki, float kd) {
    if (motor == NULL) return;
    
    memset(motor, 0, sizeof(Motor_t));
    motor->id = id;
    
    // Initialize PID controller with 10ms sample time
    PID_Init(&motor->pid, kp, ki, kd, 10);
    PID_SetOutputLimits(&motor->pid, -100.0f, 100.0f);
    PID_SetIntegralLimits(&motor->pid, -50.0f, 50.0f);
    
    motor->enabled = false;
}

void Motor_CalculateKinematics(const Velocity_Command_t *cmd, float motor_velocities[NUM_MOTORS]) {
    if (cmd == NULL || motor_velocities == NULL) return;
    
    // Mecanum wheel inverse kinematics
    // Each wheel contributes to vx, vy, and omega differently
    // For standard mecanum configuration:
    
    float lx = WHEEL_BASE_WIDTH / 2.0f;
    float ly = WHEEL_BASE_LENGTH / 2.0f;
    
    // Calculate wheel velocities (m/s)
    // Front Left: +vx +vy +omega
    motor_velocities[MOTOR_FRONT_LEFT] = (cmd->vx + cmd->vy + (lx + ly) * cmd->omega) / WHEEL_RADIUS;
    
    // Front Right: -vx +vy -omega  
    motor_velocities[MOTOR_FRONT_RIGHT] = (-cmd->vx + cmd->vy - (lx + ly) * cmd->omega) / WHEEL_RADIUS;
    
    // Back Left: -vx +vy +omega
    motor_velocities[MOTOR_BACK_LEFT] = (-cmd->vx + cmd->vy + (lx + ly) * cmd->omega) / WHEEL_RADIUS;
    
    // Back Right: +vx +vy -omega
    motor_velocities[MOTOR_BACK_RIGHT] = (cmd->vx + cmd->vy - (lx + ly) * cmd->omega) / WHEEL_RADIUS;
}

void Motor_SetVelocityCommand(const Velocity_Command_t *cmd) {
    if (cmd == NULL) return;
    
    float motor_velocities[NUM_MOTORS];
    
    // Calculate individual motor velocities
    Motor_CalculateKinematics(cmd, motor_velocities);
    
    // Set target velocities for each motor
    for (int i = 0; i < NUM_MOTORS; i++) {
        motors[i].target_velocity = motor_velocities[i];
        PID_SetSetpoint(&motors[i].pid, motor_velocities[i]);
    }
}

void Motor_Update(uint32_t current_time) {
    for (int i = 0; i < NUM_MOTORS; i++) {
        if (!motors[i].enabled) {
            motors[i].pwm_value = 0;
            HAL_SetMotorPWM(motors[i].id, 0);
            continue;
        }
        
        // Calculate current velocity from encoder
        // This is simplified - actual implementation needs time-based velocity calculation
        // motors[i].current_velocity = calculate_velocity_from_encoder(motors[i].encoder_count);
        
        // Compute PID output
        float pid_output = PID_Compute(&motors[i].pid, motors[i].current_velocity, current_time);
        
        // Set PWM value
        motors[i].pwm_value = (int16_t)pid_output;
        
        // Apply to hardware
        bool forward = motors[i].pwm_value >= 0;
        int16_t pwm_magnitude = (motors[i].pwm_value < 0) ? -motors[i].pwm_value : motors[i].pwm_value;
        
        HAL_SetMotorDirection(motors[i].id, forward);
        HAL_SetMotorPWM(motors[i].id, pwm_magnitude);
    }
}

void Motor_EmergencyStop(void) {
    for (int i = 0; i < NUM_MOTORS; i++) {
        motors[i].enabled = false;
        motors[i].pwm_value = 0;
        motors[i].target_velocity = 0;
        PID_Reset(&motors[i].pid);
        HAL_SetMotorPWM(motors[i].id, 0);
    }
}

void Motor_Enable(bool enable) {
    for (int i = 0; i < NUM_MOTORS; i++) {
        motors[i].enabled = enable;
        if (!enable) {
            motors[i].pwm_value = 0;
            HAL_SetMotorPWM(motors[i].id, 0);
            PID_Reset(&motors[i].pid);
        }
    }
}

void Motor_SetPWM(Motor_ID_t motor_id, int16_t pwm_value) {
    if (motor_id >= NUM_MOTORS) return;
    
    motors[motor_id].pwm_value = pwm_value;
    
    // Clamp PWM value
    if (pwm_value > 100) pwm_value = 100;
    if (pwm_value < -100) pwm_value = -100;
    
    bool forward = pwm_value >= 0;
    int16_t pwm_magnitude = (pwm_value < 0) ? -pwm_value : pwm_value;
    
    HAL_SetMotorDirection(motor_id, forward);
    HAL_SetMotorPWM(motor_id, pwm_magnitude);
}

Motor_t* Motor_Get(Motor_ID_t motor_id) {
    if (motor_id >= NUM_MOTORS) return NULL;
    return &motors[motor_id];
}

void Motor_UpdateEncoder(Motor_ID_t motor_id, int32_t encoder_count) {
    if (motor_id >= NUM_MOTORS) return;
    motors[motor_id].encoder_count = encoder_count;
}

// Hardware abstraction layer implementations (placeholders)
// These should be replaced with actual MCXN947 HAL calls

static void HAL_SetMotorPWM(Motor_ID_t id, int16_t pwm) {
    // TODO: Implement with MCXN947 PWM peripheral
    // Example: Set PWM duty cycle for motor driver
    // PWM_SetDutyCycle(motor_pwm_channels[id], pwm);
}

static void HAL_SetMotorDirection(Motor_ID_t id, bool forward) {
    // TODO: Implement with MCXN947 GPIO
    // Example: Set direction pins for H-bridge motor driver
    // GPIO_PinWrite(motor_dir_pins[id], forward ? 1 : 0);
}
