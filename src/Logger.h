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

    uint32_t countSuccess = 0U, countFails = 0U, countLastFails = 0U;

    bool pollWrite(const Meteo& meteo);
};
