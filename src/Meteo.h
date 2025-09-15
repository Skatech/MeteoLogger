#pragma once
#include <Arduino.h>

// Return http error code, normally 200 (HTTP_CODE_OK)
// Use t_http_codes and HTTPClient.errorToString(httpCode) from HTTPClient.h
int sendMessage(const String& request);

class Meteo {
public:
    void initSensors() const;
    bool updateSensors();
    bool writeLogger() const;
    // Replacement blocks {TIMEX} {MEASX} {HUMID} {TEMP1} {PRESS} {TEMP2}
    void formatString(String& format) const;
    String formatTime(const char* timeCustomFormat) const;

private:
    float humid = 0.0f, temp1 = 0.0f, press = 0.0f, temp2 = 0.0f;
    time_t timex = 0;
    uint32_t measx = 0;    
};
