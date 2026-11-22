#include "MotionPacketValidation.hpp"
#include <cassert>
#include <limits>

int main()
{
    for (int i = 0; i < 4; ++i)
        assert(validMotionPacket(i, 4, 0.01f, -1.0f, 9.80665f));
    assert(!validMotionPacket(-1, 4, 0.0f, 0.0f, 0.0f));
    assert(!validMotionPacket(4, 4, 0.0f, 0.0f, 0.0f));
    assert(!validMotionPacket(1, 4,
        std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f));
    assert(!validMotionPacket(2, 4, 0.0f,
        std::numeric_limits<float>::infinity(), 0.0f));
    assert(validMotionPacket(3, 4, -5.0f, 2.0f, 0.0f));
}
