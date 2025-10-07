#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetBIOS.h>
#include <SNTPControl.h>
#include <LittleFS.h>
#include "init.h"

#define WPS_BUTTON_PIN  0
#define WPS_CHANNEL     1
#define WPS_HIDDEN      0
#define WPS_MAXCONN     1

// On initial call check WPS button state and select device mode;
// On next calls restart device if WPS/boot button pressed;
// Return false if normal mode, true if WPS
bool monitorModeWPS() {
    static bool modeWPS = false, initialCall = true;

    auto getButtonPressed = []() {
        pinMode(WPS_BUTTON_PIN, INPUT_PULLUP);
        bool pressed = LOW == digitalRead(WPS_BUTTON_PIN);
        pinMode(WPS_BUTTON_PIN, INPUT);
        return pressed;
    };

    if (initialCall) {
        initialCall = false;
        modeWPS = getButtonPressed();
        while(getButtonPressed())
            delay(100);
    }
    else if (getButtonPressed()) {
        Serial.println(F("Restarting device by WPS button\n\n"));
        Serial.flush();
        ESP.restart();
    }

    return modeWPS;
}

template <typename T> void annotateValue(const __FlashStringHelper* title, const T& value) {
    Serial.print(title);
    Serial.print(": ");
    Serial.println(value);
}

bool annotateOperation(const __FlashStringHelper* title, bool result, const char* param = nullptr) {
    Serial.print(title);
    if (param) {
        Serial.print(' ');
        Serial.print('(');
        Serial.print(param);
        Serial.print(')');
    }
    Serial.print(F("... "));
    Serial.println(result ? F("OK") : F("FAILED"));
    return result;
}

void printInfoWPS() {
    String info(F(R""""(
To configure device restart it in WPS mode (keep pressed
boot button during start), connect to device and open it configuration
web page in browser (ssid, password and address can be marked on device case)
    [AP: {SSID}, password: {PASS}, page: http://{ADDR} ({HOST}.local]))""""));
    info.replace(F("{SSID}"), F(WPS_SSID));
    info.replace(F("{PASS}"), F(WPS_PASS));
    info.replace(F("{ADDR}"), F(WPS_ADDR));
    info.replace(F("{HOST}"), F(WPS_SSID));
    Serial.println(info);
}

void haltOnCriticalError(const __FlashStringHelper* message) {
    Serial.println(message);
    printInfoWPS();
    while(true)
        delay(1000);
}

// Logs to serial connection status changes, return true when connected
// WL_NO_SHIELD -> WiFi.begin() -> WL_DISCONNECTED -> WL_CONNECTED
// Reconnect: @5 @6 @0 OK => WL_CONNECTION_LOST > WL_DISCONNECTED > WL_IDLE_STATUS > WL_CONNECTED
bool watchConnection() {
    static auto old = WL_CONNECTED;
    auto now = WiFi.status();
    if (old != now) {
        if (old == WL_CONNECTED) {
            digitalWrite(LED_BUILTIN, HIGH);
            Serial.print("Connecting...");
        }
        if (now == WL_CONNECTED) {
            digitalWrite(LED_BUILTIN, LOW);
            Serial.print(" OK, IP ");
            Serial.println(WiFi.localIP());
        }
        else {
            Serial.print(" @");
            Serial.print(now);
        }
        old = now;
    }
    return now == WL_CONNECTED;
}

void initNameServices(const String& host) {
    if (host.length()) {
        annotateOperation(F("Initializing NetBIOS"), NBNS.begin(host.c_str()), host.c_str());
        annotateOperation(F("Initializing MDNS"), MDNS.begin(host), host.c_str());
        annotateOperation(F("Registering MDNS http service"), MDNS.addService(F("http"), F("tcp"), 80));
    }
}

void initConnection(const DeviceConfig& cfg) {
    pinMode(LED_BUILTIN, OUTPUT);

    if (cfg.addr != INADDR_NONE) {
        annotateValue(F("Addr"), cfg.addr);
        annotateValue(F("Gate"), cfg.gate);
        annotateValue(F("Mask"), cfg.mask);
        annotateValue(F("DNS1"), cfg.dns1);
        annotateValue(F("DNS2"), cfg.dns2);

        if (!annotateOperation(F("Changing IP configuration"),
                WiFi.config(cfg.addr, cfg.gate, cfg.mask, cfg.dns1, cfg.dns2))) {

            if (!annotateOperation(F("Trying restore IP configuration to default DHCP"), WiFi.config(0U, 0U, 0U)))
                haltOnCriticalError(F("Unable to configure IP, device HALTED"));
        }
    }

    if (!annotateOperation(F("Starting connection"), WiFi.setHostname(cfg.host.c_str())
            && WiFi.mode(WIFI_STA) && WiFi.begin(cfg.ssid, cfg.pass), cfg.ssid.c_str())) {
        haltOnCriticalError(F("Unable to start connection, device HALTED"));
    }

    for(int attempt = 0; !watchConnection(); ++attempt) {
        if (attempt == 60) {
            Serial.println();
            printInfoWPS();
        }
        delay(1000);
    }

    initNameServices(cfg.host);
}

void initAP() {
    WiFi.setTxPower(WIFI_POWER_MINUS_1dBm);
    Serial.println(F("Device in WPS mode, minimal TX power"));

    String ssid(F(WPS_SSID)), pass(F(WPS_PASS));
    IPAddress addr, mask(255,255,255,0);
        
    if (annotateOperation(F("Configuring AP"), WiFi.mode(WIFI_AP)
            && WiFi.softAPsetHostname(ssid.c_str())
            && addr.fromString(F(WPS_ADDR)) && WiFi.softAPConfig(addr, addr, mask))) {

        if (annotateOperation(F("Starting AP"),
                WiFi.softAP(ssid, pass, WPS_CHANNEL, WPS_HIDDEN, WPS_MAXCONN), ssid.c_str())) {

            initNameServices(ssid);

            annotateValue(F("SSID"), ssid);
            annotateValue(F("Pass"), pass);
            annotateValue(F("Addr"), addr);
            // annotateValue(F("Host"), WiFi.getHostname());
        }
        else haltOnCriticalError(F("Unable to start AP, in WPS mode, device HALTED"));
    }
    else haltOnCriticalError(F("Unable to configure device in WPS mode, device HALTED"));
}

void initTimeSync(const DeviceConfig& cfg) {
    if (cfg.zone.length() && cfg.tsns.length()) {
        Serial.print(F("Starting SNTP client ("));
        Serial.print(cfg.zone);
        Serial.print(F(", "));
        Serial.print(cfg.tsns);
        annotateOperation(F(")"), SNTPControl::setup(cfg.zone, getStringPartCsv(cfg.tsns, 0),
            getStringPartCsv(cfg.tsns, 2), getStringPartCsv(cfg.tsns, 2)));
    }
    else Serial.println(F("SNTP client not started, zone or server is not properly set"));
}

void mountFileSystem() {
    if (!annotateOperation(F("Mounting filesystem"), LittleFS.begin()))
        haltOnCriticalError(F("Mounting filesystem failed, device HALTED"));
}

void loadDeviceConfig(DeviceConfig& cfg, bool tryLoadDefaultsWhenFailed) {
    if (annotateOperation(F("Loading configuration"), cfg.load(false)))
        return;

    if (tryLoadDefaultsWhenFailed &&
            annotateOperation(F("Loading default configuration"), cfg.load(false)))
        return;

    haltOnCriticalError(F("Unable to load configuration, device HALTED"));
}
