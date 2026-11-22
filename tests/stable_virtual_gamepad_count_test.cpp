#include "StableVirtualGamepadCount.hpp"
#include <cassert>

int main()
{
    // Four independent Joy-Con seats exist even before any physical pad
    // appears, so emulators opened afterwards see a fixed SDL device set.
    assert(stableVirtualGamepadCount(0, -1, true, true, false) == 4);
    assert(stableVirtualGamepadCount(0, 4, false, true, false) == 4);
    assert(stableVirtualGamepadCount(1, -1, true, true, false) == 4);
    assert(stableVirtualGamepadCount(2, 4, false, true, false) == 4);
    assert(stableVirtualGamepadCount(4, 4, false, true, false) == 4);
    assert(stableVirtualGamepadCount(0, 4, true, true, false) == 4);
    // Preserve original behavior in handheld and normal paired modes.
    assert(stableVirtualGamepadCount(1, -1, true, true, true) == 1);
    assert(stableVirtualGamepadCount(1, 4, false, true, true) == 1);
    assert(stableVirtualGamepadCount(2, 4, false, false, false) == 2);
    assert(stableVirtualGamepadCount(0, 4, true, false, false) == 0);
}
