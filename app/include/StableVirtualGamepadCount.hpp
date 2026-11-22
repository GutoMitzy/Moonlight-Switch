#pragma once

#include <algorithm>

// Do not hot-plug and re-enumerate host devices every time a Joy-Con is attached.
// Four stable virtual seats are reserved for up to two independent Joy-Con pairs.
// An absent physical controller produces neutral state on its reserved seat.
inline int stableVirtualGamepadCount(int detected, int previous,
                                     bool newStream, bool separateJoyCons)
{
    if (!separateJoyCons)
        return detected;
    if (detected > 0 || (!newStream && previous > 0))
        return 4;
    return 0;
}
