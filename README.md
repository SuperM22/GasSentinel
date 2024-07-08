# GasSentinel
Group project for IoT, ECS Master's degree Sapienza
## Group members
<a href="https://www.linkedin.com/in/domenico-lattari-0947b9225/">Domenico Cesare Lattari</a>  
<a href="https://www.linkedin.com/in/shlok-bharuka-890554222/">Shlok Bharuka</a>  

## Brief description
This project aims to develop an IoT-based gas detection and alert system using the ESP32-S3 board and the MQ2 gas sensor. The MQ2 sensor is capable of detecting a variety of gases, including but not limited to, LPG, smoke, propane, methane, alcohol, hydrogen, and carbon monoxide, making it an excellent choice for a gas leakage detector.
The ESP32-S3 board, with its WiFi and Bluetooth capabilities, serves as the brain of the system. It reads the analog output from the MQ2 sensor, converts it to a digital value, and then processes this data to determine the presence and concentration of gas.
When a gas leak is detected, the system triggers an alert. This alert will be in the form of a local alarm, such as a LED, and a remote notification, that will be sent through an email.
The alert is going to be triggered in all the neighbours devices, throughout the usage of LoRA.

##Running the project
### 1. Clone the Repository
First, clone the repository to your local machine using the following command:
```sh
git clone https://github.com/SuperM22/GasSentinel.git
```
### 2. Set Up the ESP-IDF
To run the GasSentinel project, you first need to set up the ESP-IDF (Espressif IoT Development Framework) environment on your machine. This involves:
- Downloading and installing the ESP-IDF toolchain and dependencies.
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
### MQTT and Database Integration
The backend of our IoT-based gas detection and alert system leverages the MQTT protocol for efficient, lightweight messaging between the ESP32-S3 devices and the server. MQTT (Message Queuing Telemetry Transport) is chosen due to its suitability for low-bandwidth, high-latency networks, making it ideal for IoT applications.

**MQTT Broker:**
- We use a public MQTT broker, "mqtt.eclipseprojects.io", to handle the message exchange.
- The devices publish gas concentration data to a specific topic, which the server subscribes to.

**SQLite Database:**
- The backend employs SQLite for storing gas concentration data, device information, and timestamps.
- This database is lightweight, easy to manage, and suitable for handling the data requirements of this project.

**Database Schema:**
- The database contains a table named `gas_data` with fields for device ID, gas level aggregate, alarm time, latitude, longitude, and timestamp.
- This structure ensures we can log detailed records of each gas detection event, including location data.

### Email Alerts and Notifications
**Email Notifications:**
- To provide timely alerts for gas leaks, the system sends email notifications when the gas concentration exceeds a predefined threshold.
- The email includes essential details such as the device ID, gas level, and time of detection.
- A 15-minute interval between alerts prevents spam and ensures only significant alerts are sent.

### Heatmap Visualization
**Heatmap Implementation:**
- A heatmap visualization is generated to provide a geographic overview of gas leak incidents.
- This map displays the locations of all detected gas leaks, with markers and circles indicating the severity and frequency of incidents.

**Data Processing:**
- The backend processes incoming MQTT messages, extracts location data using Google Geolocation API, and stores this data in the SQLite database.
- A function counts the number of leaks within a specified radius to provide context to the heatmap markers.

**Heatmap Benefits:**
- The heatmap offers a visual representation of gas leak trends, helping to identify high-risk areas.
- This visualization aids in proactive measures and strategic planning for gas leak prevention and management.

### Advantages of Our Approach (Support by data!!!!!!!)
- **Scalability:** Using MQTT allows the system to scale efficiently, handling multiple devices without significant performance degradation.
- **Efficiency:** MQTT's lightweight nature ensures low power consumption, making it ideal for battery-operated IoT devices.
- **Reliability:** The SQLite database provides a robust mechanism for data storage, ensuring data integrity and easy retrieval.
- **Proactive Alerts:** The email alert system ensures that users are immediately notified of potential dangers, enhancing safety.
- **Insightful Visualization:** The heatmap offers valuable insights into gas leak patterns, enabling better decision-making and resource allocation.

This backend structure, combining MQTT, SQLite, and geolocation services, creates a reliable, efficient, and scalable solution for gas leak detection and alerting.






