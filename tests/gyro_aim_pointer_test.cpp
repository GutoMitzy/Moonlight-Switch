#include "GyroAimPointer.hpp"
#include <cassert>
#include <cmath>
#include <limits>

int main() {
    GyroAimPointer aim;
    float x = 0.0f, y = 0.0f;
    aim.setAcceleration(0.0f, 0.0f, -9.80665f);

    for (int i = 0; i < 1800; ++i) {
        assert(aim.update(0.01f, 0.003f, 0.0f, 0.01f, 1.0f, x, y));
        assert(x == 0.5f && y == 0.5f);
    }

    aim.reset();
    for (int i = 0; i < 100; ++i)
        assert(aim.update(0.0f, 0.5f, 0.0f, 0.01f, 1.0f, x, y));
    assert(x < 0.1f);
    const float yawX = x;

    for (int i = 0; i < 100; ++i)
        assert(aim.update(0.0f, -0.5f, 0.0f, 0.01f, 1.0f, x, y));
    assert(std::fabs(x - 0.5f) < 0.03f);

    aim.reset();
    for (int i = 0; i < 100; ++i)
        assert(aim.update(0.5f, 0.0f, 0.0f, 0.01f, 1.0f, x, y));
    assert(y < 0.1f);
    for (int i = 0; i < 100; ++i)
        assert(aim.update(-0.5f, 0.0f, 0.0f, 0.01f, 1.0f, x, y));
    assert(std::fabs(y - 0.5f) < 0.03f);

    GyroAimPointer fastRate, slowRate;
    float xFast = 0.0f, yFast = 0.0f;
    float xSlow = 0.0f, ySlow = 0.0f;
    for (int i = 0; i < 100; ++i)
        assert(fastRate.update(0.0f, 0.35f, 0.0f, 0.01f, 1.0f, xFast, yFast));
    for (int i = 0; i < 50; ++i)
        assert(slowRate.update(0.0f, 0.35f, 0.0f, 0.02f, 1.0f, xSlow, ySlow));
    assert(std::fabs(xFast - xSlow) < 0.01f);
    assert(xFast > yawX);

    aim.reset();
    for (int i = 0; i < 40; ++i)
        assert(aim.update(0.0f, 0.0f, 1.0f, 0.01f, 1.0f, x, y));
    assert(std::fabs(x - 0.5f) < 0.001f);
    assert(std::fabs(y - 0.5f) < 0.001f);

    aim.reset();
    aim.setAcceleration(0.0f, 0.0f, -9.80665f);
    for (int i = 0; i < 50; ++i)
        assert(aim.update(0.0f, 0.0f, 0.0f, 0.01f, 1.0f, x, y));
    aim.setAcceleration(0.0f, -1.8f, -9.64f);
    for (int i = 0; i < 300; ++i) {
        assert(aim.update(0.0f, 0.0f, 0.0f, 0.01f, 1.0f, x, y));
        assert(x == 0.5f && y == 0.5f);
    }
    assert(aim.update(0.0f, 0.09f, 0.0f, 0.01f, 1.0f, x, y));
    assert(x < 0.5f);
    const float firstMotionX = x;
    assert(aim.update(0.0f, 0.09f, 0.0f, 0.01f, 1.0f, x, y));
    assert(x < firstMotionX);

    aim.reset();
    for (int i = 0; i < 5000; ++i) {
        assert(aim.update(0.003f, 0.005f, 0.002f, 0.005f, 1.0f, x, y));
        assert(x == 0.5f && y == 0.5f);
    }

    aim.reset();
    assert(aim.update(0.0f, 0.035f, 0.0f, 0.005f, 1.0f, x, y));
    assert(x < 0.5f);

    aim.reset();
    for (int i = 0; i < 10000; ++i) {
        assert(aim.update(0.021f, -0.014f, 0.007f,
                          0.005f, 1.0f, x, y, true));
        assert(x == 0.5f && y == 0.5f);
    }
    assert(aim.update(0.0f, 0.15f, 0.0f, 0.005f, 1.0f,
                      x, y, true));
    assert(x < 0.5f);

    aim.reset();
    assert(!aim.update(std::numeric_limits<float>::quiet_NaN(),
                       0.0f, 0.0f, 0.01f, 1.0f, x, y));
    assert(!aim.update(0.0f, 1.0f, 0.0f, 0.15f, 1.0f, x, y));
    assert(aim.update(0.0f, 0.0f, 0.0f, 0.01f, 1.0f, x, y));
    assert(x == 0.5f && y == 0.5f);

    aim.reset();
    for (int i = 0; i < 200; ++i)
        assert(aim.update(0.0f, 1.0f, 0.0f, 0.01f, 1.0f, x, y));
    assert(x == 0.0f);
    aim.reset();
    assert(aim.update(0.0f, 0.0f, 0.0f, 0.01f, 1.0f, x, y));
    assert(x == 0.5f && y == 0.5f);
}
