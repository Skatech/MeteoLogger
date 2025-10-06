#pragma once
#include <Arduino.h>

class TimerMS {
    unsigned long _begin;

public:
    TimerMS() {
        reset();
    }

    inline void reset() {
        _begin = millis();
    }

    inline void try_advance_or_reset(unsigned long ms) {
        _begin += ms;
        if (is_passed(ms))
            reset();
    }

    inline unsigned long passed() {
        return millis() - _begin;
    }

    inline bool is_passed(unsigned long ms) {
        return passed() >= ms;
    }
};
