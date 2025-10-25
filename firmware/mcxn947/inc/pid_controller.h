/**
 * @file pid_controller.h
 * @brief PID Controller for motor control
 * @author Ivan Sigala
 * @date 2025
 *
 * PID (Proportional-Integral-Derivative) controller implementation
 * for precise motor control in the WALL-E omnidirectional robot.
 */

#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief PID Controller structure
 */
typedef struct {
    float kp;              /**< Proportional gain */
    float ki;              /**< Integral gain */
    float kd;              /**< Derivative gain */
    
    float setpoint;        /**< Target value */
    float integral;        /**< Integral accumulator */
    float prev_error;      /**< Previous error for derivative */
    
    float output_min;      /**< Minimum output limit */
    float output_max;      /**< Maximum output limit */
    
    float integral_min;    /**< Minimum integral limit (anti-windup) */
    float integral_max;    /**< Maximum integral limit (anti-windup) */
    
    uint32_t sample_time_ms; /**< Sample time in milliseconds */
    uint32_t last_time;    /**< Last execution time */
} PID_Controller_t;

/**
 * @brief Initialize PID controller
 * 
 * @param pid Pointer to PID controller structure
 * @param kp Proportional gain
 * @param ki Integral gain
 * @param kd Derivative gain
 * @param sample_time_ms Sample time in milliseconds
 */
void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd, uint32_t sample_time_ms);

/**
 * @brief Set PID output limits
 * 
 * @param pid Pointer to PID controller structure
 * @param min Minimum output value
 * @param max Maximum output value
 */
void PID_SetOutputLimits(PID_Controller_t *pid, float min, float max);

/**
 * @brief Set integral limits for anti-windup
 * 
 * @param pid Pointer to PID controller structure
 * @param min Minimum integral value
 * @param max Maximum integral value
 */
void PID_SetIntegralLimits(PID_Controller_t *pid, float min, float max);

/**
 * @brief Set PID setpoint
 * 
 * @param pid Pointer to PID controller structure
 * @param setpoint Target value
 */
void PID_SetSetpoint(PID_Controller_t *pid, float setpoint);

/**
 * @brief Compute PID output
 * 
 * @param pid Pointer to PID controller structure
 * @param input Current process variable
 * @param current_time Current time in milliseconds
 * @return PID controller output
 */
float PID_Compute(PID_Controller_t *pid, float input, uint32_t current_time);

/**
 * @brief Reset PID controller state
 * 
 * @param pid Pointer to PID controller structure
 */
void PID_Reset(PID_Controller_t *pid);

/**
 * @brief Update PID gains
 * 
 * @param pid Pointer to PID controller structure
 * @param kp Proportional gain
 * @param ki Integral gain
 * @param kd Derivative gain
 */
void PID_SetGains(PID_Controller_t *pid, float kp, float ki, float kd);

#endif /* PID_CONTROLLER_H */
