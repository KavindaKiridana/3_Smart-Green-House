#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "DHT.h" // Include the DHT sensor library

#include <NTPClient.h> // For getting the time data
#include <WiFiUdp.h>

// Provide the token generation process info
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions
#include "addons/RTDBHelper.h"

// Replace with your network credentials
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"

// Firebase credentials
#define API_KEY "Your API Key"
#define USER_EMAIL "Your email address"
#define USER_PASSWORD "email password"
#define DATABASE_URL "Your_Firebase_Database_URL"

#define DHTPIN D2     // DHT 11 sensor pin
#define DHTTYPE DHT11 // DHT sensor type
#define LDR_PIN A0    // A0 for LDR module
#define RELAY_PIN D1  // GPIO5 for relay module

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Variables
String uid;
String databasePath;
String tempPath = "/temperature";
String humPath = "/humidity";
String timePath = "/timestamp";
String parentPath;
FirebaseJson json;

unsigned long firebasePrevMillis = 0;
unsigned long controlPrevMillis = 0;
unsigned long firebaseDelay = 1800000; // 30 minutes in milliseconds
unsigned long controlDelay = 1000;     // 1 second in milliseconds

// DHT sensor object
DHT dht(DHTPIN, DHTTYPE);

// Initialize WiFi
void initWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    retries++;
    Serial.print(".");
    delay(1000);
    if (retries > 20)
    { // Retry for ~20 seconds
      Serial.println("\nFailed to connect to WiFi. Restarting...");
      ESP.restart();
    }
  }
  Serial.println("\nConnected to WiFi. IP Address: " + WiFi.localIP().toString());
}

void setup()
{
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);   // Set the relay pin as an output
  digitalWrite(RELAY_PIN, LOW); // Ensure the relay is off at startup

  // Initialize WiFi
  initWiFi();

  // Initialize NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(19800); // +5:30 timezone in seconds

  // Initialize DHT sensor
  dht.begin();

  // Firebase configuration
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // Token callback

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  if (config.database_url == "")
  {
    Serial.println("Firebase Error: Missing Database URL!");
    while (true)
    {
      delay(1000); // Halt the program
    }
  }

  // Authenticate with Firebase
  Serial.println("Getting User UID...");
  unsigned long authTimeout = millis();
  while ((auth.token.uid) == "")
  {
    Serial.print(".");
    delay(1000);
    if (millis() - authTimeout > 10000) // Timeout after 10 seconds
    {
      Serial.println("\nFailed to authenticate with Firebase. Check credentials.");
      while (true)
      {
        delay(1000); // Halt the program
      }
    }
  }

  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  databasePath = "/UsersData/" + uid + "/readings";
  parentPath = databasePath; // Initialize parentPath

  Serial.println("System Initialized.");
}

void loop()
{
  unsigned long currentMillis = millis();

  // **Control Water Supply every 1 second**
  if (currentMillis - controlPrevMillis >= controlDelay)
  {
    controlPrevMillis = currentMillis;

    // Read DHT sensor data
    float temperature = dht.readTemperature();

    // Control relay based on temperature
    if (!isnan(temperature))
    {
      if (temperature > 28.0)
      {
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println(temperature);
        Serial.println("Water supply ON");
      }
      else
      {
        digitalWrite(RELAY_PIN, LOW);
        Serial.println(temperature);
        Serial.println("Water supply OFF");
      }
    }
    else
    {
      Serial.println("Failed to read temperature for water control.");
    }
  }

  // **Send data to Firebase every 30 minutes**
  if (currentMillis - firebasePrevMillis >= firebaseDelay)
  {
    firebasePrevMillis = currentMillis;

    timeClient.update();
    // Get current time components
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
    // Calculate date and month from epoch time
    time_t rawTime = timeClient.getEpochTime();
    struct tm *timeInfo = gmtime(&rawTime);
    int currentDay = timeInfo->tm_mday;         // Extract day
    int currentMonth = timeInfo->tm_mon + 1;    // Extract month (tm_mon is 0-based)
    int currentYear = timeInfo->tm_year + 1900; // Extract year (tm_year is years since 1900)
    String dateTime = String(currentHour) + ":" +
                      String(currentMinute) + " " +
                      String(currentDay) + "-" +
                      String(currentMonth) + "-" +
                      String(currentYear);
    Serial.println(dateTime);

    // Read DHT sensor data
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int lightLevel = analogRead(LDR_PIN);

    // Check if the readings are valid
    if (isnan(temperature))
    {
      Serial.println("Failed to read DHT sensor or missing sensor");
      temperature = NAN; // Assign NAN to indicate an error
    }
    if (isnan(humidity))
    {
      Serial.println("Failed to read DHT sensor or missing sensor");
      humidity = NAN;
    }
    if (isnan(analogRead(LDR_PIN)))
    {
      Serial.println("Failed to read LDR sensor or missing sensor");
      lightLevel = -1; // Use -1 to indicate an error
    }

    // Update Firebase paths
    parentPath = databasePath + "/" + String(dateTime);
    json.set(tempPath.c_str(), String(temperature));
    json.set(humPath.c_str(), String(humidity));
    json.set("/lightLevel", String(lightLevel));
    json.set("/time", String(dateTime));

    // Push data to Firebase
    if (Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json))
    {
      Serial.println("Data sent successfully to Firebase!");
    }
    else
    {
      Serial.println("Failed to send data to Firebase: " + fbdo.errorReason());
    }
  }
}
