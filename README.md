# GasSentinel
Group project for IoT, ECS Master's degree Sapienza
## Group members
<a href="https://www.linkedin.com/in/domenico-lattari-0947b9225/">Domenico Cesare Lattari</a>  
<a href="https://www.linkedin.com/in/shlok-bharuka-890554222/">Shlok Bharuka</a>  

## Brief description
This project aims to develop an IoT-based gas detection and alert system using the ESP32-S3 board and the MQ2 gas sensor. The MQ2 sensor is capable of detecting a variety of gases, including but not limited to, LPG, smoke, propane, methane, alcohol, hydrogen, and carbon monoxide, making it an excellent choice for a gas leakage detector.
The ESP32-S3 board, with its WiFi and Bluetooth capabilities, serves as the brain of the system. It reads the analog output from the MQ2 sensor, converts it to a digital value, and then processes this data to determine the presence and concentration of gas.
When a gas leak is detected, the system triggers an alert. This alert will be in the form of a local alarm, such as a LED, and a remote notification, that will be sent through a Telegram bot. 
