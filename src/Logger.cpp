#include <WiFi.h>
#include <HTTPClient.h>
#include <DateTime.h>
#include "Logger.h"
#include "Delay.h"

int sendMessage(const String& request) {
    WiFiClient wifi;
    HTTPClient http;
    http.setConnectTimeout(1000);
    http.setTimeout(1000);

    int result = 0;
    if (http.begin(wifi, request)) {
        result = http.GET();
    }
    http.end();
    wifi.stop();
    return result;
}
bool Logger::pollWrite(const Meteo& meteo) {
    static unsigned long delay = 0UL;
    static WatchMS watch;

    if (loggingPeriod > 0UL && WiFi.isConnected() && DateTime::isSyncronized()) {
        if (watch.is_passed(delay)) {
            watch.try_advance_or_reset(delay);

            String request(loggingRequest);
            request.replace(F("{TIMEX}"), String(meteo.timex));
            request.replace(F("{MEASX}"), String(meteo.measx));
            request.replace(F("{HUMID}"), String(meteo.humid));
            request.replace(F("{TEMP1}"), String(meteo.temp1));
            request.replace(F("{PRESS}"), String(meteo.press));
            request.replace(F("{TEMP2}"), String(meteo.temp2));

            if (sendMessage(request) == HTTP_CODE_OK) {
                countSuccess += countSuccess < UINT32_MAX ? 1U : 0U;
                countLastFails = 0U;
                delay = loggingPeriod;
                return true;
            }

            countLastFails += countLastFails < UINT32_MAX ? 1U : 0U;
            countFails += countFails < UINT32_MAX ? 1U : 0U;
            delay = min(countLastFails, 30U) * 10000;
        }
    }
    return false;
}
