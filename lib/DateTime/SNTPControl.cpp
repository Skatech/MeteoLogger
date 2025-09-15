#include "SNTPControl.h" // Skatech Lab (c) - DateTime v1.0
#include <esp_sntp.h>
//#include <sntp.h>

// Seconds between ntp time syncronization CONFIG_LWIP_SNTP_UPDATE_DELAY, default value 3600

const char* SNTPControl::getTimeZone() {
    return getenv("TZ");
}

const char* SNTPControl::getTimeServer(uint8_t index) {
    return sntp_getservername(index % 3);
}

bool SNTPControl::isEnabled() {
    return sntp_enabled();
}

void SNTPControl::start() {
    sntp_init();
}

void SNTPControl::stop() {
    sntp_stop();
}

bool SNTPControl::setup(const String& timezone,
        const String& timeserver1, const String& timeserver2, const String& timeserver3) {
    static String s1, s2, s3;
    configTzTime(timezone.c_str(),
        timeserver1.isEmpty() ? nullptr : (s1 = timeserver1).c_str(),
        timeserver2.isEmpty() ? nullptr : (s2 = timeserver2).c_str(),
        timeserver3.isEmpty() ? nullptr : (s3 = timeserver3).c_str());
    return true;
}

