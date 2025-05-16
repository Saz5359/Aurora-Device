#ifndef SOILSENSORS_H
#define SOILSENSORS_H

#include <Arduino.h>

extern int currentSoilMoisture;
// extern float currentSoilTemperature;
extern float currentSoilHumidity;
/* extern float currentSoilPH;
extern float currentSoilEC; */

void initSoilSensor();
void readSoilData();

#endif