#include <Arduino.h>
#include "WiFiManager.h"
#include "FirebaseManager.h"
#include "Weather.h"
#include "SoilSensors.h"
#include "SensorPower.h"
#include "StateManager.h"

// Define the DHT object here
#define DHT_PIN 4      // DHT sensor pin (change if necessary)
#define DHT_TYPE DHT22 // DHT sensor type (change if necessary)
// DHT dht(DHT_PIN, DHT_TYPE); // Initialize the DHT sensor object

unsigned long lastSensorReadTime = 0;
unsigned long sensorReadInterval = 20000; // 30 seconds

void setup()
{
  Serial.begin(115200);
  loadDeviceState(); // 🔁 Load last saved lifecycle state
  // factoryResetDevice();
  loadWiFiCredentials(); // 📡 Load saved SSID/password
  deviceId = String((uint32_t)ESP.getEfuseMac());

  switch (currentState)
  {
  case STATE_NONE:
    Serial.println("📶 STATE_NONE: Starting AP mode...");
    startAPMode(); // Let user send WiFi credentials
    server.on("/save", HTTP_POST, handleSave);
    server.on("/reset", HTTP_POST, factoryResetDevice);
    server.begin();
    break;

  case STATE_WIFI_CONNECTED:
    Serial.println("🌐 STATE_WIFI_CONNECTED: Checking device registration...");
    if (connectToWiFi())
    {
      turnOnSensors(); // Only power up if we're WiFi ready

      isRegistered = checkDeviceRegistration();
      if (!isRegistered)
      {
        sendDeviceIdToBackend();
        // stay in this state until registration is confirmed
      }
      else
      {
        saveDeviceState(STATE_REGISTERED); // 👉 Advance
        ESP.restart();                     // Fresh start for next phase
      }
    }
    else
    {
      Serial.println("❌ Could not connect to WiFi. Reverting to AP mode.");
      startAPMode(); // Fallback
      server.on("/save", HTTP_POST, handleSave);
      server.on("/reset", HTTP_POST, factoryResetDevice);
      server.begin();
    }
    break;

  case STATE_REGISTERED:
    Serial.println("🗂️ STATE_REGISTERED: Ready for monitoring...");
    saveDeviceState(STATE_MONITORING_READY); // Advance to monitoring
    ESP.restart();                           // Reboot into next state
    break;

  case STATE_MONITORING_READY:
    Serial.println("📡 STATE_MONITORING_READY: Connecting to Firebase and initializing sensors...");
    turnOnSensors();
    if (connectToWiFi() && checkFirebaseConnection())
    {
      fetchConfiguration(); // Config from cloud
      initWeatherSensor();
      initSoilSensor();
    }
    else
    {
      Serial.println("⚠️ Failed to connect to WiFi or Firebase.");
      saveDeviceState(STATE_WIFI_CONNECTED); // Retry from WiFi phase
      ESP.restart();
    }
    break;

  default:
    Serial.println("❌ Unknown state. Resetting device state.");
    factoryResetDevice();
    ESP.restart();
    break;
  }
}

void loop()
{
  server.handleClient();

  // Periodic registration checks only when connected to WiFi
  if (WiFi.status() == WL_CONNECTED && currentState == STATE_WIFI_CONNECTED && millis() - lastCheckTime > checkInterval)
  {
    lastCheckTime = millis();
    isRegistered = checkDeviceRegistration();
    if (isRegistered)
    {
      saveDeviceState(STATE_REGISTERED); // 👉 Advance
      ESP.restart();
    }
  }

  if (currentState == STATE_MONITORING_READY && WiFi.status() == WL_CONNECTED)
  {
    unsigned long now = millis();
    if (now - lastSensorReadTime >= sensorReadInterval)
    {
      lastSensorReadTime = now;
      readWeatherData();
      readSoilData();
      sendMonitoringData();
    }
  }
}
