#include "StableVirtualGamepadCount.hpp"

#include <cassert>

int main()
{
    assert(stableVirtualGamepadCount(2, -1, true, true) == 2);
    assert(stableVirtualGamepadCount(2, 2, false, true) == 2);
    assert(stableVirtualGamepadCount(1, 2, false, true) == 2);
    assert(stableVirtualGamepadCount(0, 2, false, true) == 2);
    assert(stableVirtualGamepadCount(4, 2, false, true) == 4);
    assert(stableVirtualGamepadCount(2, 4, false, true) == 4);
    assert(stableVirtualGamepadCount(0, 4, true, true) == 0);
    assert(stableVirtualGamepadCount(2, 4, false, false) == 2);
    assert(stableVirtualGamepadCount(0, -1, false, true) == 0);
}
