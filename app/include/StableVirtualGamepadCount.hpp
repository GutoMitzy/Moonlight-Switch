#pragma once

#include <algorithm>

// Switch handheld and physically complete controllers (Pro Controller,
// other FullKey-compatible pads) keep the native Moonlight behavior.
// In detached Joy-Con mode, announce four fixed virtual seats before the
// emulator enumerates SDL devices; additionally reserve physical full-size
// seats only when they are connected.
inline int stableVirtualGamepadCount(int detected, int previous,
                                     bool newStream, bool separateJoyCons,
                                     bool handheld = false,
                                     int fullControllerCount = 0)
{
    detected = std::max(0, detected);
    fullControllerCount = std::max(0, fullControllerCount);
    if (!separateJoyCons || handheld)
        return detected;

    // Pro Controller alone: exactly the number of real controllers, using
    // the upstream FullKey button/stick/rumble pipeline. If a stream started
    // empty and already announced neutral Joy-Con ports, do not hot-unplug
    // those ports as soon as a Pro Controller is connected.
    if (fullControllerCount > 0 && detected == fullControllerCount)
    {
        if (newStream || previous <= 0)
            return detected;
        return std::max(detected, previous);
    }

    // Joy-Cons alone: 4 stable virtual seats, even if none is attached.
    // Mixed mode: 4 Joy-Con seats + one seat per connected full-size pad.
    // The Borealis physical topology determines each logical pad index.
    return std::max(4, detected);
}
