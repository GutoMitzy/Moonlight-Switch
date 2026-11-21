#pragma once

#include <cmath>

inline bool validMotionPacket(int controller, int controllerLimit,
                              float x, float y, float z)
{
    return controller >= 0 && controller < controllerLimit &&
           std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
}
