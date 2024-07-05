# GasSentinel
Group project for IoT, ECS Master's degree Sapienza
## Group members
<a href="https://www.linkedin.com/in/domenico-lattari-0947b9225/">Domenico Cesare Lattari</a>  
<a href="https://www.linkedin.com/in/shlok-bharuka-890554222/">Shlok Bharuka</a>  

## Brief description
This project aims to develop an IoT-based gas detection and alert system using the ESP32-S3 board and the MQ2 gas sensor. The MQ2 sensor is capable of detecting a variety of gases, including but not limited to, LPG, smoke, propane, methane, alcohol, hydrogen, and carbon monoxide, making it an excellent choice for a gas leakage detector.
The ESP32-S3 board, with its WiFi and Bluetooth capabilities, serves as the brain of the system. It reads the analog output from the MQ2 sensor, converts it to a digital value, and then processes this data to determine the presence and concentration of gas.
When a gas leak is detected, the system triggers an alert. This alert will be in the form of a local alarm, such as a LED, and a remote notification, that will be sent through an email.
The alert is going to be triggered in all the neighbours devices, throughout the usage of the ESP-NOW protocol.

## Connection table

| Component             | Pin / Channel                  | Configuration / Purpose                  |
|-----------------------|--------------------------------|------------------------------------------|
| **LED (Red)**         | GPIO 20                        | Configured as an output to indicate various states |
| **LED (Yellow)**      | GPIO 21                        | Configured as an output to indicate the threshold being exceeded |
| **LED (Green)**       | GPIO 26                        | Configured as an output to indicate system readiness |
| **Buzzer**            | GPIO 19 (LEDC_CHANNEL_0)       | Configured for PWM using LEDC to create sound |
| **MQ-2 Sensor**       | GPIO 36 (ADC1_CHANNEL_0)       | Configured as an ADC input to read gas concentration |
| **Resistor (4.7 kOhm)** | Between A0 of MQ-2 and Ground | Forms a voltage divider with the MQ-2 sensor to measure the gas concentration |

LoRa 1262 pins

| LoRa Module Pin | ESP32 Pin | Configuration / Purpose                 |
|-----------------|-----------|-----------------------------------------|
| NSS             | GPIO 8    | Chip Select (CS) for SPI communication  |
| SCK             | GPIO 9    | Serial Clock for SPI communication      |
| MOSI            | GPIO 10   | Master Out Slave In for SPI communication |
| MISO            | GPIO 11   | Master In Slave Out for SPI communication |
| RST             | GPIO 12   | Reset pin for the LoRa module           |
| BUSY            | GPIO 13   | Indicates the status of the LoRa module |

## Sensor calibration process
Data taken from the sensor datasheet.
First warmup 24 hrs , then 30 minutes

## Application configuration
Menuconfig screenshot

## LoRa over ESP-NOW
Esp now communication graphs (failing when walls are between devices)
LoRa graphs in crowded spaces.
LoRa graphs for transmission time.

## Remote places application
Energy consumption graph wifi vs no wifi.
graphs for both device consumption.

## Backend
mqtt + mail latency 
screens od the heatmap 




