# 🚀Smart Greenhouse Application

This project demonstrates how to create a **smart greenhouse system** using a NodeMCU (ESP8266) microcontroller. It uses sensors to monitor environmental conditions and automates the water supply based on temperature readings. The system also uploads real-time data to a Firebase database for further analysis and monitoring.

## 📂Features
1. **Temperature and Humidity Monitoring**: Uses a DHT11 sensor to measure temperature and humidity.
2. **Light Level Detection**: Reads light intensity using an LDR sensor.
3. **Automated Water Control**: Turns on the water supply if the temperature exceeds 28°C using a relay module.
4. **Cloud Integration**: Sends sensor data to Firebase every 30 minutes for remote monitoring.
5. **Real-time Clock**: Fetches the current time using an NTP client.

## 📂Components Used
- **NodeMCU (ESP8266)**: Microcontroller board.
- **DHT11 Sensor**: For measuring temperature and humidity.
- **LDR Sensor**: For detecting light levels.
- **Relay Module**: For controlling the water pump.
- **Firebase**: For data storage and monitoring.

## 📂Installation and Setup
1. Clone this repository:  
   ```
   git clone https://github.com/your-username/smart-greenhouse.git
   ```
2. Install PlatformIO IDE or use any Arduino-compatible IDE.
3. Configure your WiFi credentials and Firebase details in main.cpp:
```
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"
#define DATABASE_URL "Your_Firebase_Database_URL"
```
4. Connect the components as per the pin definitions in the code:
DHT11: Data pin to D2
LDR: Output to A0
Relay: Control pin to D1
5. Upload the code to the NodeMCU board.
6. Monitor the serial output at a baud rate of 115200.
   
## 📂Libraries Required
Ensure these libraries are installed in your PlatformIO or Arduino IDE:
Firebase Arduino Client Library for ESP8266 and ESP32
NTPClient
DHT sensor library
Adafruit Unified Sensor
LittleFS

## 📂Usage
After powering on the system, it will connect to WiFi and Firebase.
The relay module will control the water pump automatically based on the temperature.
Sensor data is uploaded to Firebase every 30 minutes for monitoring.

## 📂Notes
Ensure your Firebase database rules are configured for read/write access.
The system uses a time zone offset for Sri Lanka (+5:30 GMT). Update if required for your region.
This project is perfect for beginners looking to explore IoT and smart farming applications!
