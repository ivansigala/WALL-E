/**
 * @file pid_controller.c
 * @brief PID Controller implementation
 * @author Ivan Sigala
 * @date 2025
 */

#include "pid_controller.h"
#include <string.h>

void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd, uint32_t sample_time_ms) {
    if (pid == NULL) return;
    
    memset(pid, 0, sizeof(PID_Controller_t));
    
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->sample_time_ms = sample_time_ms;
    
    // Default output limits
    pid->output_min = -100.0f;
    pid->output_max = 100.0f;
    
    // Default integral limits (anti-windup)
    pid->integral_min = -50.0f;
    pid->integral_max = 50.0f;
}

void PID_SetOutputLimits(PID_Controller_t *pid, float min, float max) {
    if (pid == NULL) return;
    
    pid->output_min = min;
    pid->output_max = max;
}

void PID_SetIntegralLimits(PID_Controller_t *pid, float min, float max) {
    if (pid == NULL) return;
    
    pid->integral_min = min;
    pid->integral_max = max;
}

void PID_SetSetpoint(PID_Controller_t *pid, float setpoint) {
    if (pid == NULL) return;
    
    pid->setpoint = setpoint;
}

float PID_Compute(PID_Controller_t *pid, float input, uint32_t current_time) {
    if (pid == NULL) return 0.0f;
    
    // Check if enough time has passed
    uint32_t time_diff = current_time - pid->last_time;
    if (time_diff < pid->sample_time_ms && pid->last_time != 0) {
        return 0.0f; // Not time to compute yet
    }
    
    // Calculate error
    float error = pid->setpoint - input;
    
    // Proportional term
    float p_term = pid->kp * error;
    
    // Integral term (with anti-windup)
    pid->integral += error;
    
    // Clamp integral
    if (pid->integral > pid->integral_max) {
        pid->integral = pid->integral_max;
    } else if (pid->integral < pid->integral_min) {
        pid->integral = pid->integral_min;
    }
    
    float i_term = pid->ki * pid->integral;
    
    // Derivative term
    float derivative = error - pid->prev_error;
    float d_term = pid->kd * derivative;
    
    // Calculate total output
    float output = p_term + i_term + d_term;
    
    // Clamp output
    if (output > pid->output_max) {
        output = pid->output_max;
    } else if (output < pid->output_min) {
        output = pid->output_min;
    }
    
    // Save for next iteration
    pid->prev_error = error;
    pid->last_time = current_time;
    
    return output;
}

void PID_Reset(PID_Controller_t *pid) {
    if (pid == NULL) return;
    
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->last_time = 0;
}

void PID_SetGains(PID_Controller_t *pid, float kp, float ki, float kd) {
    if (pid == NULL) return;
    
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}
