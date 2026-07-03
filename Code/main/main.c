#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Pull in the RizzOtto poses header
#include "rizzotto_poses.h"

// ===== BRING-UP MODE =====
// 0 = stand & hold            (check/fit the neutral pose, power, wiring)
// 1 = per-leg direction test  (verify the signs in HIP_FWD[] / KNEE_UP[])
// 2 = run the routine         (walk 1 m -> rotate -> sit, repeat)
#define MODE 2

// ===== INSTANTIATE ENGINE GLOBALS =====
int frameDelay = 200;                // Default delay between frames (ms)
int walkCycles = 4;                 // Number of repetitions for walking actions
char currentCommand[32] = "";       // Tracks active state machine directive

// ===== HARDWARE PIN CONFIGURATION =====
#define PIN_R1   17   // Right Hip 1  (Horizontal)
#define PIN_R2   22   // Right Hip 2  (Horizontal)
#define PIN_L1   32   // Left Hip 1   (Horizontal)
#define PIN_L2   16   // Left Hip 2   (Horizontal)
#define PIN_R4   21   // Right Vert 4 (Vertical - Pair with R2)
#define PIN_R3   15   // Right Vert 3 (Vertical - Pair with R1)
#define PIN_L3   27   // Left Vert 3  (Vertical - Pair with L1)
#define PIN_L4   26   // Left Vert 4  (Vertical - Pair with L2)

// ===== PWM CONFIGURATION =====
#define SERVO_MIN_PULSE_US     500   
#define SERVO_MAX_PULSE_US     2500  
#define SERVO_MAX_DEGREE       180   
#define PWM_FREQ_HZ            50    
#define PWM_RES_BITS           LEDC_TIMER_14_BIT 
#define PWM_MAX_DUTY           ((1 << 14) - 1) 

// Array lookup to map the Pose indices (0-7) to physical hardware pins
const int SERVO_PINS[8] = {
    PIN_R1, PIN_R2, PIN_L1, PIN_L2,
    PIN_R4, PIN_R3, PIN_L3, PIN_L4
};

// Array lookup to map the Pose indices (0-7) to dedicated LEDC channels
const ledc_channel_t SERVO_CHANNELS[8] = {
    LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3,
    LEDC_CHANNEL_4, LEDC_CHANNEL_5, LEDC_CHANNEL_6, LEDC_CHANNEL_7
};

/**
 * @brief Converts target angle to LEDC duty value
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
 * @brief Interface function needed by your pose logic engine
 */
void set_servo_angle(int index, float angle) {
    if (index < 0 || index >= 8) return;
    
    uint32_t duty = angle_to_duty(angle);
    ledc_channel_t channel = SERVO_CHANNELS[index];
    
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
}

/**
 * @brief Keeps track of non-blocking interrupt flags
 */
bool pressingCheck(const char* cmd, int ms) {
    if (currentCommand[0] != '\0' && strcmp(currentCommand, cmd) != 0) {
        return false; 
    }
    vTaskDelay(pdMS_TO_TICKS(ms));
    return true;
}

void app_main(void)
{
    // ===== 1. TIMER CONFIGURATION =====
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = PWM_RES_BITS,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_config);

    // ===== 2. HARDWARE CHANNELS CONFIGURATION =====
    for (int i = 0; i < 8; i++) {
        ledc_channel_config_t channel_config = {
            .gpio_num = SERVO_PINS[i],
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .channel = SERVO_CHANNELS[i],
            .timer_sel = LEDC_TIMER_0,
            .duty = angle_to_duty(90) // Safe initialization midpoint
        };
        ledc_channel_config(&channel_config);
    }

    ESP_LOGI("main", "8-Servo LEDC Driver initialized for RizzOtto successfully.");

    // Start from the calibrated stand so the very first move is clean.
    // (At boot the channels sit at 90, which is NOT this robot's stand.)
    //standAndSettle();

    // ===== RIZZOTTO MAIN LOOP =====
    while (1) {
#if   MODE == 0
        //standAndSettle();
        runStandPose();
        vTaskDelay(pdMS_TO_TICKS(500));
#elif MODE == 1
        standAndSettle();                 // known pose first, so each move is clear on video
        for (int leg = 0; leg < 4; leg++) testLeg(leg);
        ESP_LOGI("main", "=== direction test pass done ===");
        vTaskDelay(pdMS_TO_TICKS(1500));
#else
        // ---- walk straight, then rotate, repeat ----
        walkForwardMeters(1.0f);   // go straight ~1 m
        rotateInPlace(+1);         // spin in place
        // sitDown();           // (parked)
        // frontPawWave();      // (parked)
#endif
    }
}