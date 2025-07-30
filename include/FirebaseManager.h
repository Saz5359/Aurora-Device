#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

extern int configParam1;
extern int configParam2;
extern String deviceId;

bool checkFirebaseConnection();
void fetchConfiguration();
void writePendingRegistrationToFirestore();
bool waitForRegistration();
void saveDefaultConfiguration();
void applyConfiguration();
void sendMonitoringData();

#endif
