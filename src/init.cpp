#include <WiFi.h>
#include <ESPmDNS.h>
#include <SNTPControl.h>
#include <LittleFS.h>
#include "init.h"

template <typename T> void annotateValue(const __FlashStringHelper* title, const T& value) {
    Serial.print(title);
    Serial.print(": ");
    Serial.println(value);
}

bool annotateOperation(const __FlashStringHelper* title, bool result) {
    Serial.print(title);
    Serial.print(F("... "));
    Serial.println(result ? F("OK") : F("FAILED"));
    return result;
}

void haltOnCriticalError(const __FlashStringHelper* message) {
    Serial.println(message);
    Serial.println(F(R""""(
        To configure device restart it in AP mode (keep pressed
        boot button during start), connect to device and open it configuration
        web page in browser (ssid, password and address marked ae device case)
            [AP: MeteoLogger, password: initialize, page: http://192.168.4.1]
        )""""));
    while(true)
        delay(1000);
}

// Logs to serial connection status changes, return true when connected
// WL_NO_SHIELD -> WiFi.begin() -> WL_DISCONNECTED -> WL_CONNECTED
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

void initConnection(const DeviceConfig& cfg) {
    pinMode(LED_BUILTIN, OUTPUT);
    
    if (cfg.addr != IPAddress()) {
        annotateValue(F("Addr"), cfg.addr);
        annotateValue(F("Gate"), cfg.gate);
        annotateValue(F("Mask"), cfg.mask);
        annotateValue(F("DNS1"), cfg.dns1);
        annotateValue(F("DNS2"), cfg.dns2);

        if (!annotateOperation(F("Changing IP configuration"),
                WiFi.config(cfg.addr, cfg.gate, cfg.mask, cfg.dns1, cfg.dns2)))
            haltOnCriticalError(F("Unable to configure IP, device HALTED"));
    }

    annotateValue(F("SSID"), cfg.ssid);
    WiFi.begin(cfg.ssid, cfg.pass);
    for(int attempt = 0; watchConnection() == false; ++attempt) {
        if (attempt > 60)
            haltOnCriticalError(F("Unable to connect, device HALTED"));
        delay(1000);
    }

    if (cfg.host.length()) {
        annotateValue(F("Hostname"), cfg.host);
        annotateOperation(F("Initializing DNS"), MDNS.begin(cfg.host));
    }
}

void initTimeSync(const DeviceConfig& cfg) {
    String tsn1(getStringPartCsv(cfg.tsns, 0)),
        tsn2(getStringPartCsv(cfg.tsns, 1)), tsn3(getStringPartCsv(cfg.tsns, 2));

    if (cfg.zone.length() && cfg.tsns.length()) {
        annotateValue(F("Time zone"), cfg.zone);
        annotateValue(F("Time servers"), cfg.tsns);
        annotateOperation(F("Starting SNTP client"), SNTPControl::setup(cfg.zone, tsn1, tsn2, tsn3));
    }
    else Serial.println(F("SNTP client not started, zone or server is not set properly"));
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
