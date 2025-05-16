#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

extern WebServer server;
extern String wifiSSID;
extern String wifiPassword;
extern String deviceId;
extern String deviceName;
extern String userId;
extern String growName;
extern bool deviceIdSent;
extern bool isRegistered;
extern unsigned long lastCheckTime;
extern const int checkInterval;
extern int monitorInterval;

void saveWiFiCredentials(String ssid, String password);
void loadWiFiCredentials();
void deleteWiFiCredentials();
bool checkDeviceRegistration();
void sendDeviceIdToBackend();
void startAPMode();
void stopAPMode();
void handleSave();
void factoryResetDevice();
bool connectToWiFi();

#endif
