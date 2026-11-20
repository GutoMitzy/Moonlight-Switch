#pragma once

#include <algorithm>

inline int stableVirtualGamepadCount(int detected, int previous,
                                     bool newStream, bool separateJoyCons)
{
    if (newStream || !separateJoyCons)
        return detected;
    return std::max(detected, std::max(previous, 0));
}
