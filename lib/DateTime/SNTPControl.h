#ifndef SNTPControl_h // Skatech Lab (c) - DateTime v1.0
#define SNTPControl_h

#include <Arduino.h>

class SNTPControl {
public:
    static const char* getTimeZone();
    static const char* getTimeServer(uint8_t index);
    static bool isEnabled();
    static void start();
    static void stop();
    static bool setup(const String& timezone,
        const String& timeserver1, const String& timeserver2, const String& timeserver3);
};

#endif