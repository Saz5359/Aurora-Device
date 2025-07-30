#include "WiFiManager.h"
#include "StateManager.h"

WebServer server(80);
Preferences preferences;

String wifiSSID = "";
String wifiPassword = "";
String deviceId = "";
bool deviceIdSent = false;
bool isRegistered = false;
String deviceName = "";
String userId = "";
String growName = "";
unsigned long lastCheckTime = 0;
const int checkInterval = 10000;
int monitorInterval = 10000;

void saveWiFiCredentials(String ssid, String password)
{
    preferences.begin("wifiCreds", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();
    Serial.println("✅ WiFi credentials saved.");
}

void loadWiFiCredentials()
{
    preferences.begin("wifiCreds", true);
    wifiSSID = preferences.getString("ssid", "");
    wifiPassword = preferences.getString("password", "");
    preferences.end();
    Serial.println("📥 Loaded SSID: " + wifiSSID);
}

void factoryResetDevice()
{
    Serial.println("🧹 Performing full factory reset...");

    Preferences prefs;

    prefs.begin("deviceState", false);
    prefs.clear();
    prefs.end();

    prefs.begin("wifiCreds", false);
    prefs.clear();
    prefs.end();

    prefs.begin("deviceMeta", false);
    prefs.clear();
    prefs.end();

    Serial.println("✅ All preferences cleared. Restarting...");

    resetDeviceState();
    delay(2000);
    ESP.restart();
}

void startAPMode()
{
    Serial.println("⚡ Starting Access Point Mode...");
    WiFi.softAP("Aurora-Setup", "12345678");
    Serial.println("🌐 AP Mode: Connect to 'Aurora-Setup' to configure WiFi.");
}

void stopAPMode()
{
    WiFi.softAPdisconnect(true);
    Serial.println("🛑 Stopped Access Point Mode.");
}

void handleSave()
{
    if (server.hasArg("ssid") && server.hasArg("password"))
    {
        String ssid = server.arg("ssid");
        String password = server.arg("password");

        Serial.println("📶 Received WiFi credentials:");
        Serial.println("SSID: " + ssid);
        Serial.println("Password: [Hidden]");

        WiFi.begin(ssid.c_str(), password.c_str());

        Serial.println("➡️ Trying to connect...");
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 10)
        {
            delay(1000);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\n✅ Connected to WiFi!");

            // ✅ Send success response first
            server.send(200, "text/plain", "WiFi connected and saved.");

            // ✅ Give the frontend enough time to receive the response
            delay(1500);

            // ✅ Now stop AP mode and save credentials
            stopAPMode();
            saveWiFiCredentials(ssid, password);
            saveDeviceState(STATE_WIFI_CONNECTED);

            Serial.println("🔁 Restarting device to apply settings...");
            delay(500); // small safety buffer
            ESP.restart();
        }
        else
        {
            Serial.println("\n❌ Failed to connect to WiFi.");
            String errorMessage = "Failed to connect to SSID: " + ssid + ". Check password or network.";
            server.send(400, "application/json", "{\"error\": \"" + errorMessage + "\"}");
        }
    }
    else
    {
        server.send(400, "application/json", "{\"error\": \"Missing SSID or password.\"}");
    }
}

bool connectToWiFi()
{
    Serial.println("➡️ Connecting to saved WiFi...");
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10)
    {
        delay(1000);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\n✅ Connected to WiFi!");
        Serial.println("IP Address: " + WiFi.localIP().toString());
        return true;
    }
    else
    {
        Serial.println("\n❌ Failed to connect to WiFi.");
        return false;
    }
}
