#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Preferences.h>

enum DeviceState
{
    STATE_NONE = 0,
    STATE_WIFI_CONNECTED,
    STATE_REGISTERED,
    STATE_CONFIG_FETCHED,
    STATE_MONITORING_READY
};

extern DeviceState currentState;

void loadDeviceState();
void saveDeviceState(DeviceState newState);
void resetDeviceState();
String getStateLabel(DeviceState state);

#endif
