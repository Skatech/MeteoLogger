#pragma once
#include <Arduino.h>

class Delay {
public:
    Delay(unsigned long begin = 0UL) {
        _begin = begin;
    }

    // inline unsigned long value() {
    //     return _begin;
    // }

    // inline void reset() {
    //     return _begin = 0UL;
    // }

    inline void sync() {
        _begin = millis();
    }

    void revert(unsigned long ms) {
        _begin -= ms;
    }

    bool await(unsigned long ms) {
        if (millis() - _begin >= ms) {
            _begin += ms;
            return true;
        }
        return false;
    }

    bool zero_or_await(unsigned long ms) {
        if (_begin == 0UL) {
            _begin = millis();
            return true;
        }
        return await(ms);
    }

private:
    unsigned long _begin;
};