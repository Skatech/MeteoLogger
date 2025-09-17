#pragma once
#include <Arduino.h>
#include "Meteo.h"

class Logger {
public:
    Logger(Logger const&) = delete;
    Logger& operator=(Logger const&) = delete;
    Logger(String&& requestTemplate, unsigned long loggingPeriodMs = 300000UL)
        : loggingRequest(requestTemplate), loggingPeriod(loggingPeriodMs) {}

    String loggingRequest;
    unsigned long loggingPeriod;
    bool pollWrite(const Meteo& meteo);
};
