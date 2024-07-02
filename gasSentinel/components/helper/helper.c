#include <stdio.h>
#include <string.h>
#include <math.h>
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
#include "esp_event.h"
#include "esp_log.h"
#include "helper.h"



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

#define WIFI_SSID "TIM-42988688"
#define WIFI_PASS "ycrcxyEEyktfnsYaTntpayKr"
#define MQTT_BROKER_URI "mqtt://mqtt.eclipseprojects.io:1883"

static const char *TAG = "Aux";

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retry to connect to the AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        char ip_str[16];
        esp_ip4addr_ntoa(&event->ip_info.ip, ip_str, sizeof(ip_str));
        ESP_LOGI(TAG, "Got IP: %s", ip_str);
    }
}

void wifi_init_sta(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
}


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
