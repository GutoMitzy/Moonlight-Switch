#include "StableVirtualGamepadCount.hpp"
#include <cassert>

int main()
{
    // Detached Joy-Con seats exist before hardware connects.
    assert(stableVirtualGamepadCount(0, -1, true, true, false, 0) == 4);
    assert(stableVirtualGamepadCount(0, 4, false, true, false, 0) == 4);
    assert(stableVirtualGamepadCount(1, 4, false, true, false, 0) == 4);
    assert(stableVirtualGamepadCount(4, 4, false, true, false, 0) == 4);

    // Pro Controller only behaves like upstream Moonlight: one real pad,
    // including when the user uses the "Separate Joy-Cons" preference.
    assert(stableVirtualGamepadCount(1, -1, true, true, false, 1) == 1);
    assert(stableVirtualGamepadCount(2, -1, true, true, false, 2) == 2);
    assert(stableVirtualGamepadCount(1, 1, false, true, false, 1) == 1);

    // If stream started with zero controllers, it announced four Joy-Con
    // placeholders; inserting Pro must not remove them mid-game.
    assert(stableVirtualGamepadCount(1, 4, false, true, false, 1) == 4);

    // Pro first then a Joy-Con pair gives Pro seat 0 plus four Joy-Con seats.
    assert(stableVirtualGamepadCount(5, 1, false, true, false, 1) == 5);
    // Joy-Cons first then Pro: Joy-Con seats 0..3, Pro joins at 4.
    assert(stableVirtualGamepadCount(5, 4, false, true, false, 1) == 5);
    assert(stableVirtualGamepadCount(6, 5, false, true, false, 2) == 6);
    assert(stableVirtualGamepadCount(5, 5, false, true, false, 1) == 5);

    // Explicit Pro-native compatibility mode avoids pre-announcing Joy-Con
    // devices even when the stream begins with zero connected controllers.
    assert(stableVirtualGamepadCount(0, -1, true, false, false, 0) == 0);
    assert(stableVirtualGamepadCount(1, 0, false, false, false, 1) == 1);
    assert(stableVirtualGamepadCount(2, 1, false, false, false, 2) == 2);
    assert(stableVirtualGamepadCount(0, 2, false, false, false, 0) == 0);

    // Paired mode and physical handheld mode keep original counts.
    assert(stableVirtualGamepadCount(1, 4, false, true, true, 0) == 1);
    assert(stableVirtualGamepadCount(1, -1, true, false, false, 1) == 1);
    assert(stableVirtualGamepadCount(0, 4, false, false, false, 0) == 0);
}
