#ifndef SENSOR_POWER_H
#define SENSOR_POWER_H

#define sensorPowerControlPin 32 // GPIO used to control power to sensors

void turnOnSensors()
{
    pinMode(sensorPowerControlPin, OUTPUT);
    digitalWrite(sensorPowerControlPin, HIGH);
    Serial.println("✅ Sensor power enabled");
}

void turnOffSensors()
{
    digitalWrite(sensorPowerControlPin, LOW);
    Serial.println("🛑 Sensor power disabled");
}

#endif
