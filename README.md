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
| **LED (Red)**         | GPIO 20                        | Indicates local alarm |
| **LED (Yellow)**      | GPIO 21                        | Indicates neighbour alarm |
| **LED (Green)**       | GPIO 26                        | Indicates if the system is on and working fine |
| **Buzzer**            | GPIO 19 (LEDC_CHANNEL_0)       | Configured for PWM using LEDC to create sound |
| **MQ-2 Sensor**       | GPIO 36 (ADC1_CHANNEL_0)       | Configured as an ADC input to read gas concentration |
| **Resistor (4.7 kOhm)** | Between A0 of MQ-2 and Ground | Forms a voltage divider with the MQ-2 sensor to measure the gas concentration |

The project is based on an ESP32s3 v3 LoRa 1262. The following configuration is for the LoRa chip. It can be changed by running `idf.py menuconfig`, in the SX126X Configuration voice.

| LoRa Module Pin | ESP32 Pin | Configuration / Purpose                 |
|-----------------|-----------|-----------------------------------------|
| NSS             | GPIO 8    | Chip Select (CS) for SPI communication  |
| SCK             | GPIO 9    | Serial Clock for SPI communication      |
| MOSI            | GPIO 10   | Master Out Slave In for SPI communication |
| MISO            | GPIO 11   | Master In Slave Out for SPI communication |
| RST             | GPIO 12   | Reset pin for the LoRa module           |
| BUSY            | GPIO 13   | Indicates the status of the LoRa module |

## Sensor calibration process
In order to have the correct ppm of the LPG gas, the sensor has to go through a calibration process. Since calibration kits are very costly, the calibration process is done by exploiting all the available data found in the [sensor's datasheet](https://www.pololu.com/file/0J309/MQ2.pdf). Using the sensitivity characteristic we can map the digital outputs of the sensor (ranging from 0 to 4095) to the ppm value of the LPG.  
This approach is quick, easy and free, but be aware that your readings are calculated approximately, so they won’t be as accurate. To make the values more consistent, you should
preheat your sensor, which means leave it on connected to the circuit for at least 24
hours. For future usage only 30 minutes are sufficient.  
The sensor is correctly calibrated after 30 seconds. It should be exposed to clean air during this period.

## Application configuration
`idf.py menuconfig`  
Menuconfig screenshot
In order to retrieve trends of the gas leaks we invite you to configure the internet access point credentials.   
The application is thought for differents point of installation.
If you intend to use the application for the monitoring of a building complex, then the wifi configuration should be chosen. If the wifi connection is not available, then the nowifi configuration will do fine.  
If you intend to use the application in order to monitor in remote places, and you do not care about receiving neighbour alerts, you should opt for no LoRa configuration.  
REPORT BATTERY GRAPHS AND COMPARISON CONSUMPTION FOR REMOTE MONITORING.

## LoRa over ESP-NOW

During the initial state of the project we opted to use ESP-NOW for communication between devices. Not much later we discovered that the wifi antenna we have on our devices does not work properly when there are thick walls between the devices.  
So we ought for LoRa communication.
Esp now communication graphs (failing when walls are between devices)
LoRa graphs in crowded spaces.
LoRa graphs for transmission time.

## Backend
mqtt + mail latency 
screens od the heatmap 




