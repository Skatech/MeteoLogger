#pragma once
#include <Arduino.h>

class WatchMS {
    unsigned long _begin;

    public:
    WatchMS() {
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


class Delay {
public:
    Delay(unsigned long begin = 0UL) {
        _begin = begin;
    }

    // inline unsigned long value() {
    //     return _begin;
    // }

    inline bool is_zero() {
        return _begin == 0UL;
    }

    inline void reset() {
        _begin = 0UL;
    }

    inline void syncronize() {
        _begin = millis();
    }

    inline void revert(unsigned long ms) {
        _begin -= ms;
    }

    inline void advance(unsigned long ms) {
        _begin += ms;
    }

    inline bool check(unsigned long ms) {
        return millis() - _begin >= ms;
    }

    bool check_with_advance(unsigned long ms) {
        if (check(ms)) {
            _begin += ms;
            return true;
        }
        return false;
    }

    // bool zero_or_await_and_advance(unsigned long ms) {
    //     if (is_zero()) {
    //         syncronize();
    //         return true;
    //     }
    //     return await_and_advance(ms);
    // }

private:
    unsigned long _begin;
};