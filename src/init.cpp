#include <WiFi.h>
#include <ESPmDNS.h>
#include <SNTPControl.h>
#include <LittleFS.h>
#include "init.h"

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
        Serial.print(F("Addr: "));
        Serial.println(cfg.addr);
        Serial.print(F("Gate: "));
        Serial.println(cfg.gate);
        Serial.print(F("Mask: "));
        Serial.println(cfg.mask);
        Serial.print(F("DNS1: "));
        Serial.println(cfg.dns1);
        Serial.print(F("DNS2: "));
        Serial.println(cfg.dns2);

        Serial.print(F("Changing IP configuration... "));
        if (WiFi.config(cfg.addr, cfg.gate, cfg.mask, cfg.dns1, cfg.dns2)) {
            Serial.println(F("OK"));
        }
        else {
            Serial.print(F("FAILED"));
            while (true) delay(1000);
        }
    }

    Serial.print(F("SSID: "));
    Serial.println(cfg.ssid);

    WiFi.begin(cfg.ssid, cfg.pass);
    while(!watchConnection()) delay(100);

    if (cfg.host.length()) {
        Serial.print(F("Initializing DNS ("));
        Serial.print(cfg.host);
        Serial.print(F(")... "));
        Serial.println(MDNS.begin(cfg.host) ? F("OK") : F("FAILED"));
    }
}

void initTimeSync(const DeviceConfig& cfg) {
    String tsn1(getStringPartCsv(cfg.tsns, 0)),
        tsn2(getStringPartCsv(cfg.tsns, 1)), tsn3(getStringPartCsv(cfg.tsns, 2));

    if (cfg.zone.length() && cfg.tsns.length()) {
        Serial.print(F("Starting SNTP client ("));
        Serial.print(cfg.zone);
        Serial.print(F(", "));
        Serial.print(cfg.tsns);
        Serial.print(F(")... "));
        SNTPControl::setup(cfg.zone, tsn1, tsn2, tsn3);
        Serial.println(SNTPControl::isEnabled() ? F("OK") : F("FAILED!"));
    }
    else Serial.println(F("SNTP client not started, zone or server is not set"));
}

void mountFileSystem() {
    Serial.print(F("Mounting filesystem... "));
    if (LittleFS.begin()) {
        Serial.println(F("OK"));
    }
    else {
        Serial.println(F("FAILED"));
        while(true) delay(1000);
    }
}

void loadDeviceConfig(DeviceConfig& cfg) {
    Serial.print(F("Loading configuration... "));
    if (cfg.load()) {
        Serial.println(F("OK"));
    }
    else {
        Serial.println(F("FAILED"));
        while(true) delay(1000);
    }
}
