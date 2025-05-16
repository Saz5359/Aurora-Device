#include "StateManager.h"

Preferences statePrefs;
DeviceState currentState = STATE_NONE;

void loadDeviceState()
{
    statePrefs.begin("deviceState", true);
    currentState = (DeviceState)statePrefs.getUInt("state", STATE_NONE);
    statePrefs.end();
    Serial.println("📦 [StateManager] Current saved state: " + getStateLabel(currentState));
}

void saveDeviceState(DeviceState newState)
{
    statePrefs.begin("deviceState", false);
    statePrefs.putUInt("state", newState);
    statePrefs.end();
    currentState = newState;
    Serial.println("✅ [StateManager] Updated to: " + getStateLabel(currentState));
}

void resetDeviceState()
{
    statePrefs.begin("deviceState", false);
    statePrefs.putUInt("state", STATE_NONE);
    statePrefs.end();
    currentState = STATE_NONE;
    Serial.println("🔄 Device state reset to STATE_NONE.");
}

String getStateLabel(DeviceState state)
{
    switch (state)
    {
    case STATE_NONE:
        return "None";
    case STATE_WIFI_CONNECTED:
        return "WiFi Connected";
    case STATE_REGISTERED:
        return "Registered";
    case STATE_CONFIG_FETCHED:
        return "Config Fetched";
    case STATE_MONITORING_READY:
        return "Monitoring Ready";
    default:
        return "Unknown";
    }
}
