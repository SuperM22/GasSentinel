# GasSentinel
Group project for IoT Algorithm and Services, ECS Master's degree Sapienza.  
## Group members
<a href="https://www.linkedin.com/in/domenico-lattari-0947b9225/">Domenico Cesare Lattari</a>  
<a href="https://www.linkedin.com/in/shlok-bharuka-890554222/">Shlok Bharuka</a>  

## Brief description
This project aims to develop an IoT-based gas detection and alert system using the ESP32-S3 board and the MQ2 gas sensor. The MQ2 sensor is capable of detecting a variety of gases, including but not limited to, LPG, smoke, propane, methane, alcohol, hydrogen, and carbon monoxide.
Although the sensor senses many gases we are only going to calibrate it to sense LPG.  
The ESP32-S3 board, with its WiFi and LoRA capabilities, serves as the brain of the system. It reads the analog output from the MQ2 sensor, converts it to a digital value, and then processes this data to determine the presence and concentration of gas.
When a gas leak is detected, the system triggers an alert. A remote notification will be sent through an email.
The alert is going to be triggered in all the neighbouring devices, throughout the usage of LoRA.  
You can configure the application according to your needs:  
- Wifi can be switched on or off.
- Neighbour alerts can be switched off (LoRA recieve off).  
  
## Running the project
### 1. Clone the Repository
First, clone the repository to your local machine using the following command:
```sh
git clone https://github.com/SuperM22/GasSentinel.git
```
### 2. Set Up the ESP-IDF
To run the GasSentinel project, you first need to set up the ESP-IDF (Espressif IoT Development Framework) environment on your machine. This involves:
- Ensuring that your development environment is properly configured as per the instructions provided in the [ESP-IDF documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).

### 3. Configure the Project
Once the ESP-IDF is set up:
- Navigate to the `GasSentinel` directory on your local machine.
  - ```sh
    cd GasSentinel
    ```
- Use the IDF menuconfig tool to configure the project:
  - ```sh
    idf.py menuconfig
    ```
  - Set the WiFi credentials under the Application Configuration section to enable the device to connect to the network.
  - Configure LoRA chip under the SX126X settings as specified in the connection table provided in the project documentation (If needed) .

### 4. Build and Flash the Firmware
After configuring the project:
- Build the project to compile the firmware for your ESP32 board.
  - ```sh
    idf.py build
    ```
- Flash the compiled firmware onto the ESP32 board using the appropriate commands provided by the ESP-IDF.
  - ```sh
    idf.py flash
    ```
    
### 5. Start the Python Server
To enable the functionality of the GasSentinel system:
- Execute `pip install -r requirements.txt` to install all the requirements once you have navigated into the server directory.
- Execute the `mqtt_to_database.py` script. This Python script handles MQTT message processing and data storage in the database.

Following these steps will effectively set up and run the GasSentinel system on your ESP32 board, allowing it to detect gas leaks, send alerts, and manage data visualization.

## Connection table

| Component             | Pin / Channel                  | Configuration / Purpose                  |
|-----------------------|--------------------------------|------------------------------------------|
| **LED (Red)**         | GPIO 20                        | Indicates local alarm |
| **LED (Yellow)**      | GPIO 21                        | Indicates neighbour alarm |
| **LED (Green)**       | GPIO 26                        | Indicates if the system is on and working fine |
| **Buzzer**            | GPIO 19 (LEDC_CHANNEL_0)       | Configured for PWM using LEDC to create sound |
| **MQ-2 Sensor**       | GPIO 36 (ADC1_CHANNEL_0)       | Configured as an ADC input to read gas concentration |
| **Resistor (4.7 kOhm)** | Between A0 of MQ-2 and Ground | Forms a voltage divider with the MQ-2 sensor to measure the gas concentration |  

The resistor (4.7 kOhm) to make our circuit a voltage divider circuit (suggested by the datasheet) as the MQ2 sensor changes it's resistance based on the concentration of gasses and we need to measure this change. The circuit divides the voltage between the resistor and the sensor in proportion to their resistances.

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
The datasheet reports the value of Rs/R0, where Rs is the value of the sensor's resistence in presence of the gas and R0 is the value of the sensor's resistance in clean air.
At the startup the sensor should be exposed to cleanair, so that R0 can be correctly determined.  
The data from the datasheet lets us approximate the curve:  
```LPGCurve[3]  =  {2.3,0.21,-0.47};   //data format:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59) ```  
Then we can get the exact ppm by using the formula:  
```(pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])))```  
This approach is quick, easy and free, but be aware that your readings are calculated approximately, so they won’t be as accurate. To make the values more consistent, you should
preheat your sensor, which means leave it on connected to the circuit for at least 24
hours. For future usage only 30 minutes are sufficient.  

## Application configuration
In order to retrieve trends of the gas leaks we invite you to configure the internet access point credentials.   
The application is thought for differents point of installation.
If you intend to use the application for the monitoring of a building complex, then the wifi configuration should be chosen. If the wifi connection is not available, then the nowifi configuration will do fine.  
If you intend to use the application in order to monitor in remote places, and you do not care about receiving neighbour alerts, you should opt for no LoRa configuration.  
The configuration of the application can be accessible by running `idf.py menuconfig`.  
  
![alt text](https://github.com/SuperM22/GasSentinel/blob/main/gasSentinel/Photos/menuconfig.png)  


## LoRa over ESP-NOW

During the initial state of the project we opted to use ESP-NOW for communication between devices. Not much later we discovered that the wifi antenna we have on our devices does not work properly when there are thick walls between the devices.  
When we were testing with ESP-NOW we came to the conclusion that it was only working fine when the devices were in their line of sight and as soon as there was a wall(as thick as 15-20cm) between them the devices stopped communicating.  
So we ought for LoRa communication.
  
![alt text](https://github.com/SuperM22/GasSentinel/blob/main/gasSentinel/Photos/LoRa%20Performance%20Packet%20Loss%20Rate%20vs.%20Distance%20with%20Obstacles.png)  

We configured LoRA with the maximum possible power which is 22db and we noticed that the packet loss wasn't increasing by a significant amount anytime the devices communicated between the distances 0-120m after that the losses were significant and they stopped communicating.
  
![alt text](https://github.com/SuperM22/GasSentinel/blob/main/gasSentinel/Photos/LoRA%20latency.png)  
  
We were able to achieve a latency as low as 40ms for alerting neighbouring devices for the WIFI config(1 byte).  
While we were able to get the lowest latency as 65ms for alerting neightbouring devices for the NOWIFI config(256 bytes).  

## Energy consumption  
The idea behind the no-wifi and no-lora receive configuration was to monitor remote places, without electric or internet supply. Tests were conducted in order to see what kind of battery we would need and how expensive is to run our system.  

![alt text](https://github.com/SuperM22/GasSentinel/blob/main/gasSentinel/Photos/Current%20Usage%20everything%20on.png)  
  
![alt text](https://github.com/SuperM22/GasSentinel/blob/main/gasSentinel/Photos/Power%20Usage%20everything%20on.png)  

The configuration wifi on and lora receive on, gave the average of current to be 217.8411 mA and power to be 1018.8 when the system is idle.

  
![alt text](https://github.com/SuperM22/GasSentinel/blob/main/gasSentinel/Photos/Current%20Usage%20Everything%20off.png)  

![alt text](https://github.com/SuperM22/GasSentinel/blob/main/gasSentinel/Photos/Power%20usage%20everything%20off.png)  
  
The configuration wifi off and lora receive off, gave the average of current to be 206.486 mA and power to be 971.3255 when the system is just sensing gas and not listening to LoRA.  

Altough powering this kind of device with a battery is not feasible as per the calculations a 10,000 mAh battery would last for approximately 46 hours. This is due to the constant need of monitoring for gas leaks. 


## Server side
### MQTT and Database Integration
The backend of our IoT-based gas detection and alert system leverages the MQTT protocol for efficient, lightweight messaging between the ESP32-S3 devices and the server. MQTT (Message Queuing Telemetry Transport) is chosen due to its suitability for low-bandwidth, high-latency networks, making it ideal for IoT applications.

**MQTT Broker:**
- We use a public MQTT broker, "mqtt.eclipseprojects.io", to handle the message exchange.
- The devices publish gas concentration data to a specific topic, which the server subscribes to.

**SQLite Database:**
- The database contains a table named `gas_data` with fields for device ID, gas level aggregate, alarm time, latitude, longitude, and timestamp.

### Email Alerts and Notifications
**Email Notifications:**
- To provide alerts for gas leaks, the system sends email notifications when the gas concentration reaches the alarm threshold.
- The email is sent in an average of 1 second after the gas leak is detected and transmitted through MQTT.

### Heatmap Visualization
**Heatmap Implementation:**
- A heatmap visualization is generated to provide a geographic overview of gas leak incidents.
- This map displays the locations of all detected gas leaks, with markers and circles indicating the frequency of incidents.

**Data Processing:**
- The backend processes incoming MQTT messages, extracts location data using Google Geolocation API, and stores this data in the SQLite database.
- A function counts the number of leaks within a specified radius to provide context to the heatmap markers.

![alt text](https://github.com/SuperM22/GasSentinel/blob/main/gasSentinel/Photos/map.png)






