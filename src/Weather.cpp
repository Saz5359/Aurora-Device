#include "Weather.h"
#include <DHT.h>

#define DHT_PIN 4
#define DHT_TYPE DHT22
#define UV_SENSOR_PIN 34

DHT dht(DHT_PIN, DHT_TYPE);

float currentTemperature = 0.0;
float currentHumidity = 0.0;
float currentUVIndex = 0.0;
float currentUVVoltage = 0.0;

void initWeatherSensor()
{
    dht.begin();
    pinMode(UV_SENSOR_PIN, INPUT);
    Serial.println("✅ Weather Sensor Initialized");
}
// map between 0 and 6500
// take map value - 0.045v max = 1880V
//
// change UV to visible light
void readWeatherData()
{
    int attempts = 0;
    while (attempts < 5)
    { // Try up to 5 times
        currentTemperature = dht.readTemperature();
        currentHumidity = dht.readHumidity();

        // If sensor data is invalid, retry up to 5 times
        if (isnan(currentTemperature) || isnan(currentHumidity))
        {
            Serial.println("❌ Failed to read from DHT sensor, retrying...");
            attempts++;
            delay(1000); // Wait a second before retrying
        }
        else
        {
            break; // Exit if valid data is read
        }
    }

    if (isnan(currentTemperature) || isnan(currentHumidity))
    {
        Serial.println("❌ Unable to read from DHT sensor after 5 attempts");
        return;
    }

    int lightRaw = analogRead(UV_SENSOR_PIN);
    float lightVoltage = lightRaw * (3.3 / 4095.0); // Convert ADC to voltage
    int visibleLight = map(lightVoltage * 1000, 0, 1880, 0, 6500);

    Serial.println("🌦️ Weather Data:");
    Serial.print("🌡️ Temperature: ");
    Serial.print(currentTemperature);
    Serial.println(" °C");

    Serial.print("💧 Humidity: ");
    Serial.print(currentHumidity);
    Serial.println(" %");

    Serial.print("🔆 UV Voltage: ");
    Serial.print(currentUVVoltage, 2);
    Serial.println(" V");

    Serial.print("☀️ Estimated UV Index: ");
    Serial.println(currentUVIndex, 1);
    Serial.println(" ");
}
