#pragma once
#include <Arduino.h>

// Return http error code, normally 200 (HTTP_CODE_OK)
// Use t_http_codes and HTTPClient.errorToString(httpCode) from HTTPClient.h
int sendMessage(const String& request);

class Meteo {
public:
    Meteo(Meteo const&) = delete;
    Meteo& operator=(Meteo const&) = delete;
    Meteo(unsigned long sensingPeriodMs = 5000UL) : sensingPeriod(sensingPeriodMs) {}

    void initSensors() const;
    bool updateSensors();
    bool pollUpdate();

    unsigned long sensingPeriod;
    float humid = 0.0f, temp1 = 0.0f, press = 0.0f, temp2 = 0.0f;
    time_t timex = 0;
    uint32_t measx = 0;
};