#include "GyroPointerFilter.hpp"
#include <cassert>
#include <cmath>
#include <limits>

int main()
{
    GyroPointerFilter f;
    float x = 0.0f, y = 0.0f;
    for (int i = 0; i < 300; ++i)
    {
        f.process(0.025f, -0.01f, 0.01f, 420.0f, x, y);
        assert(x == 0.0f && y == 0.0f);
    }
    for (int i = 0; i < 30; ++i)
        f.process(1.0f, 0.0f, 0.01f, 420.0f, x, y);
    assert(f.moving && x > 2.0f && y == 0.0f);

    float a = 0.0f, b = 0.0f;
    GyroPointerFilter g, h;
    float totalA = 0, totalB = 0;
    for (int i = 0; i < 100; ++i)
    {
        g.process(0.7f, 0.3f, 0.01f, 420.0f, a, b);
        totalA += a;
    }
    for (int i = 0; i < 50; ++i)
    {
        h.process(0.7f, 0.3f, 0.02f, 420.0f, a, b);
        totalB += a;
    }
    assert(std::fabs(totalA - totalB) / totalA < 0.02f);

    f.reset();
    assert(!f.moving);
    f.process(std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.01f, 420.0f, x, y);
    assert(x == 0 && y == 0 && !f.moving);

    for (int i = 0; i < 30; ++i)
        f.process(0.9f, 0.0f, 0.01f, 420.0f, x, y);
    for (int i = 0; i < 15; ++i)
    {
        f.process(0.0f, 0.0f, 0.01f, 420.0f, x, y);
        assert(x == 0 && y == 0);
    }
    assert(!f.moving);
}
