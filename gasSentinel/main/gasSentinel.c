#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"  // Include the FreeRTOS timer header
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_err.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

// Just to print MAC addresses
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

// Define the GPIO pins
#define BUZZER_GPIO_PIN 19
#define BUZZER_CHANNEL LEDC_CHANNEL_0
#define BUZZER_TIMER LEDC_TIMER_0
#define LED_GPIO_PIN 20
#define YELLOW_LED_GPIO_PIN 21
#define GREEN_LED_GPIO_PIN 26

// ADC1 channel for the MQ-2 sensor (GPIO 36 is ADC1 channel 0)
#define MQ2_ADC_CHANNEL ADC1_CHANNEL_0
#define THRESHOLD 4000  // Set threshold value for the MQ-2 sensor (RAW VALUE)
#define DURATION_THRESHOLD 10 // Duration in seconds
#define SAMPLE_PERIOD_MS 1000 // Sample period in milliseconds


#define VCC 5.0
#define RL 5
#define R0 10.75 
#define THRESHOLD_PPM 2000

const char *message = "Threshold exceeded";

// Timer handle
TimerHandle_t yellow_led_timer;

void configure_led(void)
{
    gpio_reset_pin(LED_GPIO_PIN);
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO_PIN, 0); // Ensure LED is off initially
}

void configure_led_yellow(void)
{
    gpio_reset_pin(YELLOW_LED_GPIO_PIN);
    gpio_set_direction(YELLOW_LED_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(YELLOW_LED_GPIO_PIN, 0); // Ensure LED is off initially
}

void configure_led_green(void)
{
    gpio_reset_pin(GREEN_LED_GPIO_PIN);
    gpio_set_direction(GREEN_LED_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(GREEN_LED_GPIO_PIN, 0); // Ensure LED is off initially
}

void turn_on_led(void)
{
    gpio_set_level(LED_GPIO_PIN, 1);
    printf("LED ON\n");
}

void turn_on_led_yellow(void)
{
    gpio_set_level(YELLOW_LED_GPIO_PIN, 1);
    printf("LED YELLOW ON\n");
}

void turn_on_led_green(void)
{
    gpio_set_level(GREEN_LED_GPIO_PIN, 1);
    printf("LED GREEN ON\n");
}

void turn_off_led_yellow(void)
{
    gpio_set_level(YELLOW_LED_GPIO_PIN, 0);
    printf("LED YELLOW OFF\n");
}

void turn_off_led_green(void)
{
    gpio_set_level(GREEN_LED_GPIO_PIN, 0);
    printf("LED GREEN OFF\n");
}

void turn_off_led(void)
{
    gpio_set_level(LED_GPIO_PIN, 0);
    printf("LED OFF\n");
}

void configure_buzzer(void)
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
        .duty           = 0, // Initially set duty cycle to 0 (off)
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
}

void turn_on_buzzer(void)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL, 4096); // 50% duty cycle
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL);
    printf("Buzzer ON\n");
}

void turn_off_buzzer(void)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL, 0); // 0% duty cycle (off)
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL);
    printf("Buzzer OFF\n");
}

// Timer callback function to turn off the yellow LED
void yellow_led_timer_callback(TimerHandle_t xTimer)
{
    turn_off_led_yellow();
}

// Callback function when message is received
void espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    printf("Received ESP-NOW data from: " MACSTR " \n", MAC2STR(mac_addr));
    printf("Data: %.*s\n", data_len, data);

    // Process the received data
    // If the data is "Threshold exceeded", do some stuff
    if (strncmp((const char *)data, "Threshold exceeded", data_len) == 0) {
        turn_on_led_yellow();
        turn_on_buzzer();

        // Start the timer to turn off the yellow LED after 5 seconds
        xTimerStart(yellow_led_timer, 0);
    }
}

void espnow_init(void)
{
    nvs_flash_init();
    // Initialize WiFi in station mode
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    // Initialize ESP-NOW
    esp_now_init();

    // Callback function upon reception
    esp_now_register_recv_cb(espnow_recv_cb);
}

void espnow_add_broadcast_peer(void)
{
    esp_now_peer_info_t peer_info = {
        .ifidx = ESP_IF_WIFI_STA,
        .channel = 0, // Use current WiFi channel
        .encrypt = false
    };
    memset(peer_info.peer_addr, 0xFF, ESP_NOW_ETH_ALEN); // Set broadcast address
    esp_now_add_peer(&peer_info);
}

void espnow_send_broadcast_data(const char *data)
{
    uint8_t broadcast_addr[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_send(broadcast_addr, (uint8_t *)data, strlen(data));
}

//SENSOR CALIBRATION

float calculateResistance(int analogValue) {
    float Vout = analogValue * (VCC / 4095.0); // Convert ADC reading to voltage
    float Rs = ((VCC - Vout) * RL) / Vout;     // Calculate sensor resistance
    return Rs;
}

// Function to convert Rs to ppm using sensor characteristics
float getPPM(float Rs) {
    float ratio = Rs / R0;
    float logRatio = log10(ratio);
    float ppm = pow(10, (-1.465 * logRatio + 2.675)); 
    return ppm;
}


float adc;

void app_main(void)
{
    // Configure the LED
    configure_led();
    configure_led_yellow();
    configure_led_green();

    turn_on_led_green();

    // Configure the buzzer
    configure_buzzer();

    // Initialize ESP-NOW
    espnow_init();
    espnow_add_broadcast_peer();

    // Create the timer to turn off the yellow LED
    yellow_led_timer = xTimerCreate("YellowLEDTimer", pdMS_TO_TICKS(5000), pdFALSE, (void *)0, yellow_led_timer_callback);

    // Configure ADC width and attenuation
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(MQ2_ADC_CHANNEL, ADC_ATTEN_DB_11);

    // Characterize ADC
    esp_adc_cal_characteristics_t *adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, adc_chars);

    int counter = 0;
    int required_count = DURATION_THRESHOLD * (1000 / SAMPLE_PERIOD_MS); // Number of iterations for the threshold duration

    while (1) {
        // Read ADC value
        int adc_reading = adc1_get_raw(MQ2_ADC_CHANNEL);
        // Convert ADC value to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        printf("ADC Raw: %d\tVoltage: %ldmV\n", adc_reading, voltage);

        float Rs = calculateResistance(adc_reading);
        float ppm = getPPM(Rs);

        // Check if the reading exceeds the threshold
        if (ppm > THRESHOLD_PPM) {
            counter++;
            turn_on_led();
            if (counter >= required_count) {
                turn_on_buzzer();
                printf("Threshold exceeded for %d seconds! LED and Buzzer on\n", DURATION_THRESHOLD);
                espnow_send_broadcast_data(message);
                printf("ESP-NOW broadcast message sent: %s\n", message);
                adc = adc_reading;
            }
        } else {
            counter = 0; // Reset the counter if the reading falls below the threshold
            turn_off_led();
            turn_off_buzzer();
            printf("Below threshold. LED and Buzzer off\n");
        }

        // Delay before next reading
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}
