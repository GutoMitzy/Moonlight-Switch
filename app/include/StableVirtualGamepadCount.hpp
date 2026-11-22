#pragma once

#include <algorithm>

// Preserve stable Sunshine gamepad identities from the *first* packet of a
// streaming session. Detached Joy-Cons in separate mode always expose four
// seats, including when no Joy-Con is connected at stream startup.
// Missing controllers are represented by neutral state, not by hot-unplugging
// and shifting subsequent gamepad IDs. Handheld and paired layouts keep their
// original controller counts.
inline int stableVirtualGamepadCount(int detected, int previous,
                                     bool newStream, bool separateJoyCons,
                                     bool handheld = false)
{
    (void)previous;
    (void)newStream;
    if (!separateJoyCons || handheld)
        return std::max(0, detected);
    return 4;
}
