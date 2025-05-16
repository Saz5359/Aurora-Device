#ifndef WEATHER_H
#define WEATHER_H

extern float currentTemperature;
extern float currentHumidity;
extern float currentUVIndex;
extern float currentUVVoltage;

void initWeatherSensor();
void readWeatherData();

#endif