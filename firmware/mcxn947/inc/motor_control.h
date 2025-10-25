/**
 * @file motor_control.h
 * @brief Motor control for omnidirectional robot
 * @author Ivan Sigala
 * @date 2025
 *
 * Motor control implementation for 4-wheel omnidirectional (mecanum/omni) robot.
 * Includes kinematics calculations and individual motor control.
 */

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "pid_controller.h"

// Number of motors for omnidirectional robot
#define NUM_MOTORS 4

/**
 * @brief Motor identification
 */
typedef enum {
    MOTOR_FRONT_LEFT = 0,
    MOTOR_FRONT_RIGHT = 1,
    MOTOR_BACK_LEFT = 2,
    MOTOR_BACK_RIGHT = 3
} Motor_ID_t;

/**
 * @brief Motor control structure
 */
typedef struct {
    Motor_ID_t id;              /**< Motor identifier */
    PID_Controller_t pid;       /**< PID controller for this motor */
    
    int32_t encoder_count;      /**< Current encoder count */
    float target_velocity;      /**< Target velocity (RPM or units/s) */
    float current_velocity;     /**< Current velocity */
    
    int16_t pwm_value;          /**< PWM output value (-100 to 100) */
    
    bool enabled;               /**< Motor enable flag */
} Motor_t;

/**
 * @brief Robot velocity command structure
 */
typedef struct {
    float vx;      /**< Velocity in X direction (strafe) */
    float vy;      /**< Velocity in Y direction (forward/back) */
    float omega;   /**< Rotational velocity (yaw rate) */
} Velocity_Command_t;

/**
 * @brief Initialize motor control system
 */
void Motor_Init(void);

/**
 * @brief Initialize individual motor
 * 
 * @param motor Pointer to motor structure
 * @param id Motor identifier
 * @param kp PID proportional gain
 * @param ki PID integral gain
 * @param kd PID derivative gain
 */
void Motor_InitSingle(Motor_t *motor, Motor_ID_t id, float kp, float ki, float kd);

/**
 * @brief Set velocity command for the robot
 * 
 * @param cmd Velocity command structure
 */
void Motor_SetVelocityCommand(const Velocity_Command_t *cmd);

/**
 * @brief Update motor control (call in main loop or timer ISR)
 * 
 * @param current_time Current time in milliseconds
 */
void Motor_Update(uint32_t current_time);

/**
 * @brief Emergency stop - immediately stop all motors
 */
void Motor_EmergencyStop(void);

/**
 * @brief Enable/disable all motors
 * 
 * @param enable true to enable, false to disable
 */
void Motor_Enable(bool enable);

/**
 * @brief Set PWM for individual motor (low-level control)
 * 
 * @param motor_id Motor identifier
 * @param pwm_value PWM value (-100 to 100)
 */
void Motor_SetPWM(Motor_ID_t motor_id, int16_t pwm_value);

/**
 * @brief Get motor structure pointer
 * 
 * @param motor_id Motor identifier
 * @return Pointer to motor structure
 */
Motor_t* Motor_Get(Motor_ID_t motor_id);

/**
 * @brief Update encoder reading for a motor
 * 
 * @param motor_id Motor identifier
 * @param encoder_count New encoder count
 */
void Motor_UpdateEncoder(Motor_ID_t motor_id, int32_t encoder_count);

/**
 * @brief Calculate individual motor velocities from robot velocity command
 *        Uses inverse kinematics for omnidirectional robot
 * 
 * @param cmd Robot velocity command
 * @param motor_velocities Output array for 4 motor velocities
 */
void Motor_CalculateKinematics(const Velocity_Command_t *cmd, float motor_velocities[NUM_MOTORS]);

#endif /* MOTOR_CONTROL_H */
