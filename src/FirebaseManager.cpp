// FirebaseManager.cpp
#include "FirebaseManager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Preferences.h>
#include "Weather.h"
#include "SoilSensors.h"

// There is already a Preferences object defined in WiFiManager.cpp so we use prefers for this file
Preferences prefers;

// Global configuration parameters
int configParam1 = 0;
int configParam2 = 0;
extern String deviceId;

// Bring in global variables from WiFiManager
extern String wifiSSID;
extern String wifiPassword;
extern String deviceName;
extern String userId;
extern String growName;
extern int monitorInterval;

const String FIREBASE_URL = "https://aurora-demo-d9027-default-rtdb.firebaseio.com/";
const String FIRESTORE_URL = "https://firestore.googleapis.com/v1/projects/aurora-demo-d9027/databases/(default)/documents";
const String FIREBASE_AUTH = "?auth=qdvTsHBSsNwQXYzUgaU3mwFvkNvasKgej8Mk6jQ5";

// NTP Client for real-time timestamps
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // Sync every 60 seconds

// Function to get formatted timestamp
String getTimestamp()
{
    timeClient.update();
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime((time_t *)&epochTime);
    char buffer[20];
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
            ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    return String(buffer);
}

void writePendingRegistrationToFirestore()
{
    Serial.println("📡 Checking Firebase for existing registration...");

    String url = FIREBASE_URL + "devices/Aurora-" + deviceId + ".json" + FIREBASE_AUTH;

    HTTPClient http;
    http.begin(url);
    int getCode = http.GET();

    if (getCode == 200)
    {
        DynamicJsonDocument doc(512);
        DeserializationError err = deserializeJson(doc, http.getString());

        if (!err)
        {
            String status = doc["status"] | "";

            if (status == "pending")
            {
                Serial.println("⏳ Device is already marked as pending. Skipping write.");
                http.end();
                return;
            }
            else if (status == "registered")
            {
                Serial.println("✅ Device is already registered. Skipping write.");
                http.end();
                return;
            }
        }
        else
        {
            Serial.println("⚠️ JSON parsing failed: " + String(err.c_str()));
        }
    }
    else if (getCode != 404)
    {
        Serial.println("⚠️ Failed to check existing registration. HTTP code: " + String(getCode));
        http.end();
        return;
    }

    http.end(); // Done with GET

    // If device is not found (404) or has no status, attempt to write it
    Serial.println("📡 Writing pending registration to Firebase...");

    const int maxRetries = 3;
    int attempt = 0;
    bool success = false;

    String body = "{\"status\":\"pending\",\"ts\": {\".sv\":\"timestamp\"}}";

    while (attempt < maxRetries && !success)
    {
        http.begin(url);
        http.addHeader("Content-Type", "application/json");

        int httpCode = http.PUT(body); // Or use POST if that’s your DB structure
        http.end();

        if (httpCode == 200 || httpCode == 201)
        {
            Serial.println("✅ Registration request sent. Waiting for approval...");
            success = true;
        }
        else
        {
            Serial.println("❌ Failed to write registration. Attempt " + String(attempt + 1) +
                           " of " + String(maxRetries) + ". HTTP Code: " + String(httpCode));
            delay(2000); // Retry delay
            attempt++;
        }
    }

    if (!success)
    {
        Serial.println("🚨 All registration attempts failed. Please check your connection.");
    }
}

bool waitForRegistration()
{
    Serial.println("📡 Checking if device is registered...");

    unsigned long startTime = millis();
    const int retryDelay = 3000; // Check every 3 seconds

    String url = FIREBASE_URL + "devices/Aurora-" + deviceId + ".json" + FIREBASE_AUTH;

    while (millis())
    {
        HTTPClient http;
        http.begin(url);
        int code = http.GET();

        if (code == 200)
        {
            DynamicJsonDocument doc(512);
            DeserializationError err = deserializeJson(doc, http.getString());

            if (!err)
            {
                String status = doc["status"] | "";
                if (status == "registered")
                {
                    deviceName = doc["deviceName"] | "";
                    userId = doc["userId"] | "";
                    growName = doc["growName"] | "";

                    // Save locally
                    prefers.begin("deviceMeta", false);
                    prefers.putString("userId", userId);
                    prefers.putString("deviceName", deviceName);
                    prefers.putString("growName", growName);
                    prefers.end();

                    Serial.println("✅ Device registered successfully!");
                    Serial.println("🔌 Device ID: " + deviceId);
                    Serial.println("📛 Device Name: " + deviceName);
                    Serial.println("👤 User ID: " + userId);
                    Serial.println("🌱 Grow Name: " + growName);

                    http.end();
                    return true;
                }
                else
                {
                    Serial.println("⌛ Status: " + status + " — Waiting for registration...  Http Code: " + String(code));
                }
            }
            else
            {
                Serial.println("❌ JSON parsing error: " + String(err.c_str()));
            }
        }
        else
        {
            Serial.println("❌ HTTP error while checking registration. Code: " + String(code));
        }

        http.end();
        delay(retryDelay);
    }

    Serial.println("⏰ Timeout reached — Device was not registered.");
    return false;
}

bool checkFirebaseConnection()
{
    Serial.println("📡 Checking Firebase connection...");

    HTTPClient http;
    String testUrl = FIREBASE_URL + "test.json" + FIREBASE_AUTH;

    http.begin(testUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200)
    {
        Serial.println("✅ Successfully connected to Firebase.");
        http.end();
        return true;
    }
    else
    {
        Serial.print("❌ Firebase connection failed. HTTP Code: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }
}

void fetchConfiguration()
{
    Serial.println("...");
    Serial.println("📡 Fetching device configuration from Firebase...");
    String url = FIREBASE_URL + "Aurora-" + deviceId + "/configuration.json" + FIREBASE_AUTH;

    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200)
    {
        String response = http.getString();
        Serial.println("✅ Response received from Firebase.");
        Serial.println("...");

        if (response == "null" || response.isEmpty())
        {
            Serial.println("⚠️ No configuration found. Saving default settings...");
            saveDefaultConfiguration();
            http.end();
            return;
        }

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, response);

        if (error)
        {
            Serial.print("❌ Failed to parse configuration: ");
            Serial.println(error.c_str());
            http.end();
            return;
        }

        prefers.begin("deviceConfig", false);
        long remoteLastUpdated = doc["wifi"]["lastUpdated"].as<long>();
        long localLastUpdated = prefers.getLong("lastUpdated", 0);

        /* if (remoteLastUpdated == localLastUpdated)
        {
            Serial.println("⚠️ Config already up-to-date. Skipping fetch.");
            prefers.end();
            http.end();
            return;
        } */

        bool configChanged = false;

        if (doc.containsKey("wifi"))
        {
            Serial.println("🔍 Checking Wi-Fi settings...");
            String fetchedSSID = doc["wifi"]["ssid"].as<String>();
            String fetchedPassword = doc["wifi"]["password"].as<String>();

            // Wifi is not stored in plain text, so we don't compare passwords
            if (fetchedSSID != wifiSSID /*||  fetchedPassword != wifiPassword */)
            {
                Serial.println("✅ Wi-Fi config changed.");
                wifiSSID = fetchedSSID;
                // wifiPassword = fetchedPassword;
                prefers.putString("wifiSSID", wifiSSID);
                prefers.putString("wifiPassword", wifiPassword);
                configChanged = true;
            }
        }

        if (doc.containsKey("monitoring"))
        {
            Serial.println("🔍 Checking monitoring settings...");
            int fetchedInterval = doc["monitoring"]["interval"].as<int>();
            bool sendData = doc["monitoring"]["sendData"].as<bool>();

            if (fetchedInterval != monitorInterval)
            {
                Serial.println("✅ Monitoring interval changed. " + String(fetchedInterval) + " ms" + " vs " + String(monitorInterval) + " ms");
                monitorInterval = fetchedInterval;
                prefers.putInt("monitorInterval", monitorInterval);
                configChanged = true;
            }
        }

        if (doc.containsKey("device"))
        {
            Serial.println("🔍 Checking device info...");
            String fetchedDeviceName = doc["device"]["deviceName"].as<String>();
            String fetchedUserId = doc["device"]["userId"].as<String>();
            String fetchedGrow = doc["device"]["growName"].as<String>();

            if (fetchedDeviceName != deviceName || fetchedUserId != userId || fetchedGrow != growName)
            {
                Serial.println("✅ Device settings changed.");
                deviceName = fetchedDeviceName;
                userId = fetchedUserId;
                growName = fetchedGrow;
                prefers.putString("deviceName", deviceName);
                prefers.putString("userId", userId);
                prefers.putString("growName", growName);
                configChanged = true;
            }
        }

        if (configChanged)
        {
            Serial.println("✅ Applying updated configuration...");
            applyConfiguration();
        }
        else
        {
            Serial.println("ℹ️ No changes detected in configuration.");
        }

        // Always save lastUpdated even if nothing else changed
        prefers.putLong("lastUpdated", remoteLastUpdated);
        prefers.end();
        Serial.println("...");
    }
    else
    {
        Serial.println("⚠️ Failed to fetch configuration.");
        saveDefaultConfiguration();
    }

    http.end();
}

void saveDefaultConfiguration()
{
    Serial.println("...");
    Serial.println("💾 Saving default configuration to Firebase...");
    String url = FIREBASE_URL + "Aurora-" + deviceId + "/configuration.json" + FIREBASE_AUTH;

    // Get current timestamp
    timeClient.update();
    long currentTimestamp = timeClient.getEpochTime();

    prefers.begin("deviceMeta", true);
    userId = prefers.getString("userId", "");
    deviceName = prefers.getString("deviceName", "");
    growName = prefers.getString("growName", "");
    prefers.end();

    DynamicJsonDocument doc(1024);

    // Firebase config
    doc["wifi"]["ssid"] = wifiSSID;
    doc["wifi"]["password"] = "HIDDEN"; // Do not store password in plain text
    doc["wifi"]["lastUpdated"] = currentTimestamp;

    doc["monitoring"]["interval"] = monitorInterval;
    doc["monitoring"]["sendData"] = true;

    doc["device"]["power"] = 85;
    doc["device"]["reset"] = false;
    doc["device"]["deviceName"] = deviceName;
    doc["device"]["userId"] = userId;
    doc["device"]["growName"] = growName;

    // Convert JSON to string
    String jsonString;
    serializeJson(doc, jsonString);

    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.PUT(jsonString);

    if (httpResponseCode == 200)
    {
        Serial.println("✅ Default configuration saved to Firebase.");

        // Save locally to preferences
        prefers.begin("config", false);
        prefers.putString("ssid", wifiSSID);
        prefers.putString("password", wifiPassword);
        prefers.putInt("interval", monitorInterval);
        prefers.putString("deviceName", deviceName);
        prefers.putString("userId", userId);
        prefers.putString("growName", growName);
        prefers.putULong("lastUpdated", currentTimestamp);
        prefers.end();

        Serial.println("📦 Local config saved (initial default).");
        Serial.println("");
    }
    else
    {
        Serial.println("❌ Failed to save default configuration.");
    }

    http.end();
}

void applyConfiguration()
{
    Serial.println("...");
    Serial.println("⚙️ Applying configuration...");
    Serial.println("...");
    Serial.println("🔹 WiFi SSID: " + wifiSSID);
    Serial.println("🔹 Device Name: " + deviceName);
    Serial.println("🔹 User ID: " + userId);
    Serial.println("🔹 Grow Name: " + growName);
    Serial.println("🔹 Monitoring Interval: " + String(monitorInterval) + " ms");
    Serial.println("...");
    // Additional actions to apply these settings can be added here.
}

// Function to send monitoring data to Firebase
void sendMonitoringData()
{
    Serial.println("...");
    Serial.println("📡 Sending monitoring data to Firebase...");

    // Get current timestamp
    String timestamp = getTimestamp();
    String currentDate = timestamp.substring(0, 10);  // Get YYYY-MM-DD
    String currentTime = timestamp.substring(11, 19); // Get HH:MM:SS

    // Generate random data for monitoring (you can replace this with real sensor values)
    float temperature = currentTemperature;
    float humidity = currentHumidity;
    float uvVoltage = currentUVVoltage;
    float uvIndex = currentUVIndex;
    float soilMoisture = currentSoilMoisture;
    float soilHumidity = currentSoilHumidity;

    // Create the JSON object to send
    DynamicJsonDocument json(1024); // Increase size if needed

    json["Temperature"] = temperature;
    json["Humidity"] = humidity;
    json["UV_Voltage"] = uvVoltage;
    json["UV_Index"] = uvIndex;
    json["Soil_Moisture"] = soilMoisture;
    json["Soil_Humidity"] = soilHumidity;
    /*     json["WiFi_Name"] = WiFi.SSID();
        json["WiFi_Signal_Strength"] = WiFi.RSSI();                                   */

    // Convert JSON to string
    String jsonString;
    serializeJson(json, jsonString);

    // Construct the Firebase URL for monitoring data
    String url = FIREBASE_URL + "Aurora-" + deviceId + "/monitoring/" + currentDate + "/" + currentTime + ".json" + FIREBASE_AUTH;

    // Send data to Firebase using HTTP PUT request
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.PUT(jsonString);

    if (httpResponseCode == 200)
    {
        Serial.println("✅ Monitoring data successfully sent to Firebase.");
    }
    else
    {
        Serial.print("❌ Failed to send monitoring data. HTTP Error: ");
        Serial.println(httpResponseCode);
    }

    http.end();
}
