#ifndef SOILSENSORS_H
#define SOILSENSORS_H

// Include necessary libraries
#include <Arduino.h>
#include <DHT.h> // Library for DHT sensors

// Define pin numbers
#define SOIL_SENSOR_PIN A0 // Analog pin for soil moisture sensor
#define DHT_PIN 2          // Digital pin for DHT sensor (change if needed)
#define PH_SENSOR_PIN A3   // Analog pin for pH sensor
#define EC_SENSOR_PIN A6   // Analog pin for EC sensor

#define NPK_SERIAL_RX 16  // ESP32 hardware serial RX (choose suitable pins)
#define NPK_SERIAL_TX 17  // ESP32 hardware serial TX
#define NPK_BAUDRATE 9600 // NPK sensor baud rate

// DHT sensor initialization
#define DHT_TYPE DHT22 // DHT22 is used for temperature and humidity
extern DHT dht;        // Create an instance of the DHT sensor
int currentSoilMoisture = 0;
float currentSoilHumidity = 0.0;

// Hardware serial for NPK sensor
HardwareSerial &npkSerial = Serial2;

// Function to initialize the soil sensors
void initSoilSensor()
{
  Serial.println("🌱 Initializing Soil Sensors...");

  // Initialize DHT sensor
  dht.begin();

  pinMode(SOIL_SENSOR_PIN, INPUT); // Set soil sensor pin to input
  pinMode(PH_SENSOR_PIN, INPUT);   // Set pH sensor pin to input
  pinMode(EC_SENSOR_PIN, INPUT);   // Set EC sensor pin to input

  npkSerial.begin(NPK_BAUDRATE, SERIAL_8N1, NPK_SERIAL_RX, NPK_SERIAL_TX);

  Serial.println("✅ Soil Sensor Initialized");
}

// Function to read soil data (moisture, temperature, pH, EC, NPK)
void readSoilData()
{
  // Read soil moisture from analog sensor (updated range)
  int soilMoisture = analogRead(SOIL_SENSOR_PIN);              // Read moisture value
  int moisturePercentage = map(soilMoisture, 0, 4095, 100, 0); // Updated to ESP32 ADC range

  // Read temperature and humidity from DHT sensor
  // float temperature = dht.readTemperature(); // Temperature in Celsius
  float humidity = dht.readHumidity(); // Humidity in percentage

  currentSoilMoisture = moisturePercentage; // Store the latest moisture value
  currentSoilHumidity = humidity;
  // Read soil pH from pH sensor (ensure proper calculation)
  /* int rawPh = analogRead(PH_SENSOR_PIN);
  float voltagePh = (rawPh / 4095.0) * 3.3;
  float pH = 7 + ((2.5 - voltagePh) / 0.18); // pH calculation

  // Read soil EC from EC sensor (updated calculation)
  int rawEc = analogRead(EC_SENSOR_PIN);
  float ecVoltage = (rawEc / 4095.0) * 3.3; // Voltage reading
  float ec = ecVoltage * 1000;              // EC in millisiemens (check if this conversion needs tweaking) */

  // Check if any reading failed (e.g., if the sensor is disconnected)
  if (isnan(currentSoilMoisture) || isnan(currentSoilHumidity))
  {
    Serial.println("❌ Failed to read from DHT sensor!");
    return; // Exit if the sensor failed
  }

  Serial.println("📊 Soil Data:");
  Serial.println("💧 Soil Moisture: " + String(currentSoilMoisture) + " %");
  Serial.println("💨 Humidity: " + String(currentSoilHumidity) + " %");

  // Output the data with icons

  /* Serial.print("🌡️ Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, "); */

  /*   Serial.print("🌱 Soil pH: ");
    Serial.print(pH, 2);
    Serial.print(", ");

    Serial.print("⚡ Soil EC: ");
    Serial.print(ec);
    Serial.println(" mS/cm"); */
}

#endif // SOILSENSORS_H
