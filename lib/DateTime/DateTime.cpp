#include "DateTime.h" // Skatech Lab (c) - DateTime v1.0
#include <esp_sntp.h>
// #include <sntp.h>
// #include <time.h>

// extern "C" int settimeofday(const struct timeval* tv, const struct timezone* tz);

String DateTime::toString(const char* format) const {
    struct tm *t = localtime(&_t);
    char v[36];
    strftime(v, sizeof(v), format, t);
    return String(v);
}

DateTime DateTime::fromParts(uint16_t year, uint8_t mon,
        uint8_t mday, uint8_t hour, uint8_t min, uint8_t sec) {
    struct tm t = {0};
    t.tm_year = year - 1900;
    t.tm_mon = mon - 1;
    t.tm_mday = mday;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;
    return mktime(&t);
}

DateTime DateTime::parse(const char* input,
        const char* sscanf_format, int parts_expected, size_t min_length) {
    if (strlen(input) >= min_length) {
        struct tm t = {0};
        if (sscanf(input, sscanf_format,
                &t.tm_year, &t.tm_mon, &t.tm_mday,
                &t.tm_hour, &t.tm_min, &t.tm_sec) == parts_expected) {
            t.tm_year -= 1900;
            t.tm_mon -= 1;
            return mktime(&t);
        }
    }
    return TIME_NONE;
}

bool DateTime::setSystemTime(const DateTime& val) {
    struct timeval tv = { val._t, 0 };
    return settimeofday(&tv, nullptr) == 0;
}

