#include "StableVirtualGamepadCount.hpp"

#include <cassert>

int main()
{
    // Fixed host seats prevent first-pair input profiles from migrating when
    // a second pair is connected or temporarily disconnected.
    assert(stableVirtualGamepadCount(2, -1, true, true) == 4);
    assert(stableVirtualGamepadCount(2, 4, false, true) == 4);
    assert(stableVirtualGamepadCount(4, 4, false, true) == 4);
    assert(stableVirtualGamepadCount(1, 4, false, true) == 4);
    assert(stableVirtualGamepadCount(0, 4, false, true) == 4);
    assert(stableVirtualGamepadCount(4, 2, false, true) == 4);
    assert(stableVirtualGamepadCount(0, -1, true, true) == 0);
    assert(stableVirtualGamepadCount(2, 4, false, false) == 2);
    assert(stableVirtualGamepadCount(0, -1, false, true) == 0);
    // Portable with attached Joy-Cons is still one controller.
    assert(stableVirtualGamepadCount(1, -1, true, true, true) == 1);
    assert(stableVirtualGamepadCount(1, 4, false, true, true) == 1);
    // Leaving handheld for docked establishes four persistent host seats.
    assert(stableVirtualGamepadCount(2, 1, false, true, false) == 4);
}
