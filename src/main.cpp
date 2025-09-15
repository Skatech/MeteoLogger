#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include <DateTime.h>
#include <SNTPControl.h>

#include "Meteo.h"
#include "delay.h"

unsigned long sensingPeriod = 5000UL, loggingPeriod = 60000UL;
Meteo meteo;

WebServer webServer(80);

bool checkConnection() {
    static auto old = WL_CONNECTED;
    auto now = WiFi.status();
    // WL_NO_SHIELD -> (begin) -> WL_DISCONNECTED -> WL_CONNECTED
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
    while(!checkConnection()) delay(100);

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

void initWebServer() {
    webServer.on("/", HTTP_GET, []() {
        // Serial.print(F("Handling client: "));
        // Serial.print(webServer.client().remoteIP());
        // Serial.print(F("  uri: "));
        // Serial.println(webServer.uri());
        String fmt(F(R""""(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="icon" href="data:,">
    <title>Meteo Logger</title>
    <style>
        body {
            padding: 0;
            margin: 0;
            font-family: Consolas;
            letter-spacing: -0.04em;
            font-size: 1.02em;
        }
        h1 {
            font-size: 3.5rem;
            font-weight: lighter;
            text-align: center;
        }
        div h4 {
            max-width: calc(var(--width) + 2.5rem);
            border-bottom: 1px solid #888;
            font-size: large;
        }
        div.container {
            width: 400px;
            height: 100vh;
            margin: 0 auto;
            padding: 10px;
        }
        div.parameter {
            text-align: center;
            font-size: 1.2rem;
            font-weight: bold;
            margin-bottom: 0.75rem;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Meteo Logger</h1>
        <h4>Time</h4>
        <div class="parameter">{TIMEX}</div>
        <div class="parameter">Measurement: {MEASX}</div>
        <h4>AHT20</h4>
        <div class="parameter">Humidity: {HUMID}%</div>
        <div class="parameter">Temperature: {TEMP1}°C</div>
        <h4>BMP240</h4>
        <div class="parameter">Pressure: {PRESS}mm</div>
        <div class="parameter">Temperature: {TEMP2}°C</div>
    </div>
</body>
</html>
)""""));
        fmt.replace(F("{TIMEX}"), meteo.formatTime("%H:%M:%S %a, %b %d, %Y")); //19:02:43 Sat, Sep 13, 2025
        meteo.formatString(fmt);
        webServer.send(200, F("text/html"), fmt);
    });

    webServer.onNotFound([]() {
        Serial.print(F("Handling client (not found): "));
        Serial.print(webServer.client().remoteIP());
        Serial.print(F("  uri: "));
        Serial.println(webServer.uri());
        webServer.send(404, F("text/plain"), F("Not found"));
    });

    webServer.begin();
}

void setup(void) {
    Serial.begin(SERIAL_SPEED);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.print(F("\n\nESP32-MeteoLogger v"));
    Serial.println(FIRMWARE_VERSION);
    Serial.println(F("======================"));
    initConnection();
    initWebServer();
    initTimeSync();
    meteo.initSensors();
}

void loop(void) {
    static Delay sense, wrlog;
    bool connected = checkConnection();

    if (sense.await(sensingPeriod)) {
        if (meteo.updateSensors()) { // 79ms
            if (wrlog.zero_or_await(loggingPeriod)) {
                if (connected) {
                    if (!meteo.writeLogger())
                        wrlog.revert(loggingPeriod);
                }
            }
        }
    }
    
    if (connected) {
        webServer.handleClient();
    }
}