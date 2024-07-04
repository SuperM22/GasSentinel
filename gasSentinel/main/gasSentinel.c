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
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "ra01s.h"
#include "helper.h"
#include "esp_mac.h"

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
#define MQTT_BROKER_URI "mqtt://mqtt.eclipseprojects.io:1883"

#define VCC 5.0
#define RL 4.7
#define RO_CLEAN_AIR_FACTOR 9.83 //initialization of R0 (datasheet)
#define THRESHOLD_PPM 2000 //5% of LEL LPG
#define CALIBARAION_SAMPLE_TIMES 30 //this should be 30 minutes (sensor warmup period)

float R0 = RO_CLEAN_AIR_FACTOR; 
float LPGCurve[3]  =  {2.3,0.21,-0.47}; //taken from the datasheet to calibrate

const char *message = "Threshold exceeded";

// Implement Wi-Fi initialization and event handling
static const char *TAG = "Gas Sentinel ";

uint8_t mac_addr[6];  // To store the MAC address

// Timer handle
TimerHandle_t yellow_led_timer;

esp_mqtt_client_handle_t mqtt_client;

TaskHandle_t myTaskHandle = NULL;

bool buzz =false;

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            printf("MQTT_EVENT_CONNECTED\n");
            break;
        case MQTT_EVENT_DISCONNECTED:
            printf("MQTT_EVENT_DISCONNECTED\n");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            printf("MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            printf("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            printf("MQTT_EVENT_PUBLISHED, msg_id=%d\n", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            printf("MQTT_EVENT_DATA\n");
            break;
        case MQTT_EVENT_ERROR:
            printf("MQTT_EVENT_ERROR\n");
            break;
        default:
            printf("Other event id:%d\n", event->event_id);
            break;
    }
}

void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_BROKER_URI,
        },
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

// SENSOR CALIBRATION

float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL*(4095-raw_adc)/raw_adc)); //4095 is the max value that adc reading yelds with bit width 12
}

int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}

float MQCalibration()
{
  int i;
  float val=0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //take multiple samples
    val += MQResistanceCalculation(adc1_get_raw(MQ2_ADC_CHANNEL));
    vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculate the average value

  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                        //according to the chart in the datasheet 

  return val; 
}

// LORA
void loraStart()
{
  uint32_t frequencyInHz=915000000;
  LoRaInit();
  int8_t txPowerInDbm = 22;
  float tcxoVoltage = 3.3; // use TCXO
  bool useRegulatorLDO = true; // use DCDC + LDO
  if (LoRaBegin(frequencyInHz, txPowerInDbm, tcxoVoltage, useRegulatorLDO) != 0) {
    ESP_LOGE(TAG, "Does not recognize the module");
    while(1) {
      vTaskDelay(1);
    }
  }

  uint8_t spreadingFactor = 7;
  uint8_t bandwidth = 4;
  uint8_t codingRate = 1;
  uint16_t preambleLength = 8;
  uint8_t payloadLen = 0;
  bool crcOn = true;
  bool invertIrq = false;
  LoRaConfig(spreadingFactor, bandwidth, codingRate, preambleLength, payloadLen, crcOn, invertIrq);
}

void listening_task(void *pvParameter)
{
  ESP_LOGI(pcTaskGetName(NULL), "Start listening");
  uint8_t rxData[256]; // Maximum Payload size of SX1261/62/68 is 255
  while(1) {
    uint8_t rxLen = LoRaReceive(rxData, sizeof(rxData));
    if ( rxLen > 0 ) { 
      printf("Receive rxLen:%d\n", rxLen);
      int txLen;
      char *rec = (char *)rxData;
      if (rec[0] == 'S' ) {
        turn_off_buzzer();
        buzz=false;
        turn_off_led_yellow();
        ESP_LOGI(TAG, "NEIGHBOUR DEVICE EXITED THE ALARM STATE");
      } else if (rec[0] == 'A' ){
        buzz=true;
        turn_on_buzzer();
        turn_on_led_yellow();
        ESP_LOGI(TAG, "NEIGHBOUR DEVICE EXITED THE ALARM STATE");
      } else {
        ESP_LOGI(TAG,"NEIGHBOUR DEVICE WITH NO MQTT CONNECTION SENT THE AGGREGATE");
        #if CONFIG_WIFI
          int l = strlen(rec);
          for (int i = 1 ; i<l-1 ; i++){
            if(rec[ l - i  ] =='}'){
              rec[ l - i ] = '\0';
            }
          }
          // Ensure 'new' is large enough to hold additional information
          char new[64];
          uint8_t* bssid = get_bssid();
          snprintf(new, sizeof(new), "\n    'wifi':'0',\n    'bssid': '" MACSTR "'\n}", MAC2STR(bssid));
          // Calculate the total required length
          printf("%i, %i\n", strlen(rec), strlen(new));
          size_t totalLength = strlen(rec) + strlen(new) + 1; // +1 for null terminator
          if (totalLength <= 256) {
              strcat(rec, new);
              printf("%s", rec);
              esp_mqtt_client_publish(mqtt_client, "/topic/qos0", rec, 0, 1, 0);
              ESP_LOGI(TAG, "MQTT message sent: %s\n", rec);
          } else {
              ESP_LOGE(TAG, "Combined message length exceeds buffer size");
          }
        #endif
      }
    }
    vTaskDelay(1); // Avoid WatchDog alerts
  } // end while
}

void app_main(void)
{
  uint8_t txData[8];
  int txLen;
  
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Initialize Wi-Fi
  #if CONFIG_WIFI
  wifi_init_sta();
  
  // Retrieve MAC address
  ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, mac_addr));
  
  // Retrieve BSSID
  uint8_t* bssid = get_bssid();
  
  // Initialize MQTT
  mqtt_app_start();
  #endif

  #if CONFIG_NOWIFI
  if(esp_efuse_mac_get_default(mac_addr)== ESP_OK){
    ESP_LOGI(TAG,"Mac address acquired correctly");
  } else {
    ESP_LOGE(TAG,"MAC ADDRESS WAS NOT ACQUIRED");
  }
  #endif

  // Configure the LED
  configure_led();
  configure_led_yellow();
  configure_led_green();

  turn_on_led_green();

  // Configure the buzzer
  configure_buzzer();

  // Create the timer to turn off the yellow LED
  yellow_led_timer = xTimerCreate("YellowLEDTimer", pdMS_TO_TICKS(5000), pdFALSE, (void *)0, yellow_led_timer_callback);

  // Configure ADC width and attenuation
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(MQ2_ADC_CHANNEL, ADC_ATTEN_DB_11);

  // Characterize ADC
  esp_adc_cal_characteristics_t *adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, adc_chars);

  int counter = 0;
  int required_count = DURATION_THRESHOLD * (1000 / SAMPLE_PERIOD_MS); // Number of iterations for the threshold duration

  printf("Starting calibration ...\n");
  R0 = MQCalibration();
  printf("Sensor calibrated r0: %f\n",R0);
  float Rs;
  int ppm;
  bool triggered = false;
  bool loraSent = false;
  long long int avgPPM = 0;

  loraStart();
  xTaskCreatePinnedToCore(&listening_task, "LISTENING", 4096, NULL, 5, &myTaskHandle,0);
  while (1) {
    // Read ADC value
    int adc_reading = adc1_get_raw(MQ2_ADC_CHANNEL);
    // Convert ADC value to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    printf("ADC Raw: %d\tVoltage: %ldmV\n", adc_reading, voltage);

    Rs = MQResistanceCalculation(adc_reading);
    ppm = MQGetPercentage(Rs/R0,LPGCurve);
    printf("PPM calculated : %i\n",ppm);
    // Check if the reading exceeds the threshold
    printf("%lld",avgPPM);
    if (ppm > THRESHOLD_PPM) {
      counter++;
      turn_on_led();
      if (counter >= required_count) {
        triggered = true;
        turn_on_buzzer();
        printf("Threshold exceeded for %d seconds! LED and Buzzer on\n", DURATION_THRESHOLD);
        if(!loraSent){
          vTaskSuspend(myTaskHandle);
          txLen = sprintf((char *)txData, "A");
          if(LoRaSend(txData,txLen,SX126x_TXMODE_SYNC)){
            loraSent=true;
            ESP_LOGI(TAG,"Alert sent through LoRa");
            memset(txData,0,8);
          }else{
            ESP_LOGE(TAG,"ERROR SENDING THE ALERT");
          }
        }
        // Take max ppm , avg ppm and counter
      }
      avgPPM += (long long)ppm;
    } else {
      if(loraSent){
        vTaskSuspend(myTaskHandle);
        txLen = sprintf((char *)txData, "S");
        if(!LoRaSend(txData,txLen,SX126x_TXMODE_SYNC)){
          ESP_LOGE(TAG,"Error exiting alert state through LoRa");
        } else {
          loraSent=false;
          vTaskResume(myTaskHandle);
          memset(txData,0,8);
          ESP_LOGI(TAG,"STOPALERT sent through LoRa");
        }
      }
      if(triggered){
        triggered = false;
        avgPPM = avgPPM / (long long)counter;
                
        #if CONFIG_WIFI
        char mqtt_message[256];
        snprintf(mqtt_message, sizeof(mqtt_message), "{\n   'device_id': '" MACSTR "',\n    'gas_level_agg': '%lld',\n    'alarm_time' : '%i',\n    'wifi':'1',\n    'bssid': '" MACSTR "'\n}", MAC2STR(mac_addr), avgPPM, counter, MAC2STR(bssid));
        esp_mqtt_client_publish(mqtt_client, "/topic/qos0", mqtt_message, 0, 1, 0);
        printf("MQTT message sent: %s\n", mqtt_message);
        #endif

        #if CONFIG_NOWIFI
        uint8_t mqtt_message[256];
        txLen = snprintf((char *)mqtt_message, sizeof(mqtt_message), "{\n   'device_id': '" MACSTR "',\n    'gas_level_agg': '%lld',\n    'alarm_time' : '%i',}", MAC2STR(mac_addr), avgPPM, counter);
        vTaskSuspend(myTaskHandle);
        if(!LoRaSend(mqtt_message,txLen,SX126x_TXMODE_SYNC)){
          ESP_LOGE(TAG,"Error sending aggregate data through lora");
        } else {
          vTaskResume(myTaskHandle);
          ESP_LOGI(TAG,"Aggregate data sent through LoRa");
        }
        #endif
      }
      avgPPM = 0;
      counter = 0; // Reset the counter if the reading falls below the threshold
      turn_off_led();
      if(!buzz){
        turn_off_buzzer();
      }
      printf("Below threshold. LED and Buzzer off\n");
    }

    // Delay before next reading
    vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
  }
}
