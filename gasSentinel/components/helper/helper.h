

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void wifi_init_sta(void);
void configure_led(void);
void configure_led_yellow(void);
void configure_led_green(void);
void turn_on_led(void);
void turn_on_led_yellow(void);
void turn_on_led_green(void);
void turn_off_led_yellow(void);
void turn_off_led_green(void);
void turn_off_led(void);
void configure_buzzer(void);
void turn_on_buzzer(void);
void turn_off_buzzer(void);
void yellow_led_timer_callback(TimerHandle_t xTimer);


//public functions