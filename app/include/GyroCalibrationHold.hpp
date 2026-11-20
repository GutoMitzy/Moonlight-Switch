#pragma once

#include <chrono>

class GyroCalibrationHold {
public:
    using Clock = std::chrono::steady_clock;

    bool update(bool bothPressed, Clock::time_point now)
    {
        if (!bothPressed)
        {
            reset();
            return false;
        }
        if (!active)
        {
            started = now;
            active = true;
            return false;
        }
        if (triggered || now < started)
            return false;
        if (now - started < std::chrono::seconds(2))
            return false;
        triggered = true;
        return true;
    }

    void reset()
    {
        active = false;
        triggered = false;
        started = {};
    }

private:
    Clock::time_point started = {};
    bool active = false;
    bool triggered = false;
};
