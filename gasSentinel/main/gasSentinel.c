#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"

// Define the GPIO pins where the buzzer and LED are connected
#define BUZZER_GPIO_PIN 19
#define BUZZER_CHANNEL LEDC_CHANNEL_0
#define BUZZER_TIMER LEDC_TIMER_0
#define LED_GPIO_PIN 20

void configure_led(void)
{
    gpio_reset_pin(LED_GPIO_PIN);
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT);
}

void turn_on_led(void)
{
    gpio_set_level(LED_GPIO_PIN, 1);
}

void turn_off_led(void)
{
    gpio_set_level(LED_GPIO_PIN, 0);
}

void app_main(void)
{
    // Configure the LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE, // Use LEDC_LOW_SPEED_MODE
        .timer_num        = BUZZER_TIMER,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .freq_hz          = 2000,  // Set the PWM frequency to 2 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Configure the LEDC channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE, // Use LEDC_LOW_SPEED_MODE
        .channel        = BUZZER_CHANNEL,
        .timer_sel      = BUZZER_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = BUZZER_GPIO_PIN,
        .duty           = 4096, // Set duty cycle to 50%
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);

    // Configure the LED
    configure_led();

    // Start buzzer with 50% duty cycle and turn on LED
    printf("Buzzer started with 50%% duty cycle, LED on\n");
    turn_on_led();
    vTaskDelay(pdMS_TO_TICKS(5000)); // Buzz for 5 seconds

    // Change to 25% duty cycle
    //duty cycle does not really matter
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL, 2048);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL);
    printf("Buzzer changed to 25%% duty cycle\n");
    vTaskDelay(pdMS_TO_TICKS(5000)); // Buzz for another 5 seconds

    // Stop buzzer and turn off LED
    ledc_stop(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL, 0);
    turn_off_led();
    printf("Buzzer stopped, LED off\n");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Keep the task running
    }
}
