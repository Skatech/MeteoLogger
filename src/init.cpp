#include <WiFi.h>
#include <ESPmDNS.h>
#include <SNTPControl.h>
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

void initConnection() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    String conn_addr(F(CONN_ADDR)), conn_gate(F(CONN_GATE)),
        conn_mask(F(CONN_MASK)), conn_dns1(F(CONN_DNS1)), conn_dns2(F(CONN_DNS2));
    
    if (conn_addr.length()) {  
        Serial.print(F("Addr: "));
        Serial.println(conn_addr);
        Serial.print(F("Gate: "));
        Serial.println(conn_gate);
        Serial.print(F("Mask: "));
        Serial.println(conn_mask);
        Serial.print(F("DNS1: "));
        Serial.println(conn_dns1);
        Serial.print(F("DNS2: "));
        Serial.println(conn_dns2);

        IPAddress addr, gate, mask, dns1, dns2;
        addr.fromString(conn_addr);
        gate.fromString(conn_gate);
        mask.fromString(conn_mask);
        dns1.fromString(conn_dns1);
        dns2.fromString(conn_dns2);
        
        Serial.print(F("Configuring IPs... "));
        if (WiFi.config(addr, gate, mask, dns1, dns2)) {
            Serial.println(F("OK"));
        }
        else {
            Serial.print(F("FAILED"));
            while (true) delay(1000);
        }
    }

    String conn_ssid(F(CONN_SSID)), conn_pass(F(CONN_PASS));
    Serial.print(F("SSID: "));
    Serial.println(conn_ssid);

    WiFi.begin(conn_ssid, conn_pass);
    while(!watchConnection()) delay(100);

    String conn_mdns(F(CONN_MDNS));
    if (conn_mdns.length()) {
        Serial.print(F("mDNS: "));
        Serial.println(conn_mdns);

        Serial.print(F("Initializing mDNS... "));
        Serial.println(MDNS.begin(conn_mdns) ? F("OK") : F("FAILED"));
    }
}

void initTimeSync() {
    String sntp_zone(F(SNTP_ZONE)), sntp_tsn1(F(SNTP_TSN1)), sntp_tsn2(F(SNTP_TSN2)), sntp_tsn3(F(SNTP_TSN3));
    if (sntp_zone.length()) {
        Serial.print(F("SNTP Zone: "));
        Serial.print(sntp_zone);
        Serial.print(F(", "));
        Serial.print(sntp_tsn1);
        Serial.print(F(", "));
        Serial.print(sntp_tsn2);
        Serial.print(F(", "));
        Serial.println(sntp_tsn3);

        Serial.print(F("Starting SNTP... "));
        SNTPControl::setup(sntp_zone, sntp_tsn1, sntp_tsn2, sntp_tsn3);
        Serial.println(SNTPControl::isEnabled() ? F("OK") : F("FAILED!"));
    }
}
