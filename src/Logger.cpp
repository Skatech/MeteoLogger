#include <WiFi.h>
#include <HTTPClient.h>
#include "Logger.h"
#include "Delay.h"

int sendMessage(const String& request) {
    WiFiClient wifi;
    HTTPClient http;
    int result = 0;
    if (http.begin(wifi, request)) {
        result = http.GET();
    }
    http.end();
    wifi.stop();
    return result;
}

bool Logger::pollWrite(const Meteo& meteo) {
    static Delay delay;
    if (WiFi.isConnected()) {
        if (delay.zero_or_await(loggingPeriod)) {
            String request(loggingRequest);
            request.replace(F("{TIMEX}"), String(meteo.timex));
            request.replace(F("{MEASX}"), String(meteo.measx));
            request.replace(F("{HUMID}"), String(meteo.humid));
            request.replace(F("{TEMP1}"), String(meteo.temp1));
            request.replace(F("{PRESS}"), String(meteo.press));
            request.replace(F("{TEMP2}"), String(meteo.temp2));
            if (sendMessage(request) == 200) {
                return true;
            }
            else delay.revert(loggingPeriod);
        }
    }
    return false;
}
