#include "GyroAimPointer.hpp"

#include <cassert>
#include <cmath>
#include <limits>

int main()
{
    GyroAimPointer adaptive;
    GyroAimPointer legacy;
    float xa = 0.5f, ya = 0.5f;
    float xl = 0.5f, yl = 0.5f;

    for (int i = 0; i < 4000; ++i)
    {
        const float gyroNoise = (i % 2) ? -0.022f : 0.022f;
        const float accelNoise = (i % 2) ? 0.11f : -0.11f;
        adaptive.setAcceleration(accelNoise, 0.0f, -9.80665f);
        assert(adaptive.update(0.0f, gyroNoise, 0.0f,
                               0.005f, 1.0f, xa, ya, true));
        assert(xa == 0.5f && ya == 0.5f);
    }

    adaptive.reset();
    for (int i = 0; i < 5; ++i)
    {
        assert(adaptive.update(0.0f, 0.8f, 0.0f,
                               0.005f, 1.0f, xa, ya, true));
        assert(legacy.update(0.0f, 0.8f, 0.0f,
                             0.005f, 1.0f, xl, yl, false));
    }
    assert(xa < xl);
    assert(xa < 0.5f);
    assert(ya == 0.5f && yl == 0.5f);

    const float beforeClick = xa;
    adaptive.stabilizeClick();
    for (int i = 0; i < 9; ++i)
    {
        assert(adaptive.update(0.0f, 0.5f, 0.0f,
                               0.005f, 1.0f, xa, ya, true));
        assert(xa == beforeClick);
    }
    for (int i = 0; i < 25; ++i)
        assert(adaptive.update(0.0f, 0.5f, 0.0f,
                               0.005f, 1.0f, xa, ya, true));
    assert(xa < beforeClick);

    GyroAimPointer fast;
    GyroAimPointer slow;
    float xf = 0.5f, yf = 0.5f, xs = 0.5f, ys = 0.5f;
    for (int i = 0; i < 200; ++i)
        assert(fast.update(0.0f, 0.38f, 0.0f,
                           0.005f, 1.0f, xf, yf, true));
    for (int i = 0; i < 100; ++i)
        assert(slow.update(0.0f, 0.38f, 0.0f,
                           0.01f, 1.0f, xs, ys, true));
    assert(std::fabs(xf - xs) < 0.025f);

    adaptive.reset();
    assert(adaptive.update(0.0f, 0.0f, 0.0f,
                           0.005f, 1.0f, xa, ya, true));
    assert(xa == 0.5f && ya == 0.5f);
    assert(!adaptive.update(std::numeric_limits<float>::quiet_NaN(),
                            0.0f, 0.0f, 0.005f, 1.0f, xa, ya, true));
}
