#pragma once

#include <algorithm>
#include <cmath>

struct GyroPointerFilter
{
    float filteredX = 0.0f;
    float filteredY = 0.0f;
    bool moving = false;

    void reset()
    {
        filteredX = 0.0f;
        filteredY = 0.0f;
        moving = false;
    }

    void process(float yaw, float pitch, float dt, float gain, float& dx, float& dy)
    {
        dx = 0.0f;
        dy = 0.0f;
        if (!std::isfinite(yaw) || !std::isfinite(pitch) ||
            !std::isfinite(dt) || !std::isfinite(gain) ||
            dt <= 0.0f || dt >= 0.1f)
        {
            reset();
            return;
        }

        const float speed = std::hypot(yaw, pitch);
        if (speed > 15.0f)
        {
            reset();
            return;
        }

        constexpr float startThreshold = 0.075f;
        constexpr float stopThreshold = 0.032f;
        if (moving ? speed <= stopThreshold : speed >= startThreshold)
            moving = !moving;

        const float alpha = -std::expm1(-dt * 18.0f);
        filteredX += ((moving ? yaw : 0.0f) - filteredX) * alpha;
        filteredY += ((moving ? pitch : 0.0f) - filteredY) * alpha;

        if (!moving)
        {
            if (std::fabs(filteredX) < 0.004f) filteredX = 0.0f;
            if (std::fabs(filteredY) < 0.004f) filteredY = 0.0f;
            return;
        }

        dx = filteredX * dt * gain;
        dy = filteredY * dt * gain;
    }
};
