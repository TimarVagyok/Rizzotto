#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// ===== HARDWARE PIN CONFIGURATION =====
#define SERVO_A_PIN            21   // L2
#define SERVO_B_PIN            22   // L4

// ===== SERVO CONFIGURATION =====
#define SERVO_MIN_PULSE_US     500   // Pulse width for 0 degrees (in microseconds)
#define SERVO_MAX_PULSE_US     2500  // Pulse width for 180 degrees (in microseconds)
#define SERVO_MAX_DEGREE       180   // Maximum angle of the servo

// ===== PWM CONFIGURATION =====
#define PWM_FREQ_HZ            50    // 50Hz frequency (20ms period)
#define PWM_RES_BITS           LEDC_TIMER_14_BIT // 14-bit resolution (0 to 16383)
#define PWM_MAX_DUTY           ((1 << 14) - 1)   // 16383

/**
 * @brief Converts a target angle to the corresponding LEDC duty value
 */
uint32_t angle_to_duty(float angle) {
    if (angle < 0) angle = 0;
    if (angle > SERVO_MAX_DEGREE) angle = SERVO_MAX_DEGREE;

    uint32_t pulse_us = SERVO_MIN_PULSE_US + 
                        ((angle / SERVO_MAX_DEGREE) * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US));

    uint32_t period_us = 1000000 / PWM_FREQ_HZ; 
    return (pulse_us * PWM_MAX_DUTY) / period_us;
}

/**
 * @brief Helper function to update a specific channel's angle
 */
void set_servo_angle(ledc_channel_t channel, float angle) {
    uint32_t duty = angle_to_duty(angle);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
}

void app_main(void)
{
    // ===== 1. TIMER CONFIGURATION (Shared by both servos) =====
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = PWM_RES_BITS,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_config);

    // ===== 2. CHANNEL CONFIGURATION (Servo A - Pin 21) =====
    ledc_channel_config_t channel_a_config = {
        .gpio_num = SERVO_A_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = angle_to_duty(0) // Start at 0 degrees
    };
    ledc_channel_config(&channel_a_config);

    // ===== 3. CHANNEL CONFIGURATION (Servo B - Pin 22) =====
    ledc_channel_config_t channel_b_config = {
        .gpio_num = SERVO_B_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_1, // Must be a unique channel
        .timer_sel = LEDC_TIMER_0, // Points to the same timer
        .duty = angle_to_duty(0) // Start at 0 degrees
    };
    ledc_channel_config(&channel_b_config);

    ESP_LOGI("main", "Dual Servo PWM initialised successfully.");

    // ===== DEMO: Control both servos independently =====
    while (1) {
        // Example 1: Move L2 to 0 and L4 to 180
        ESP_LOGI("Position Zero", "Moving L2 to 135, L4 to 0");
        set_servo_angle(LEDC_CHANNEL_0, 135);
        set_servo_angle(LEDC_CHANNEL_1, 0);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Example 2: Move only L2 to 45 and L4 to 130 (standing position)
        ESP_LOGI("Position 1", "On its legs");
        set_servo_angle(LEDC_CHANNEL_0, 45);
        set_servo_angle(LEDC_CHANNEL_1, 130);
        vTaskDelay(pdMS_TO_TICKS(2000));

        /* Example 3: Swap them (L2 to 180, L4 to 0)
        ESP_LOGI("main", "Moving A to 180, B to 0");
        set_servo_angle(LEDC_CHANNEL_0, 180);
        set_servo_angle(LEDC_CHANNEL_1, 0);
        vTaskDelay(pdMS_TO_TICKS(2000));*/
    }
}