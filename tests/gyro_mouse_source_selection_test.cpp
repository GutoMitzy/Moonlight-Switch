#include "GyroMouseSourceSelection.hpp"
#include <cassert>
#include <cstdint>

int main()
{
    GyroMouseSourceSelection selector;
    assert(selector.controller() == -1);

    // Any side and any logical seat can own the cursor, not the first gyro
    // which happens to report a movement.
    assert(selector.update(0, true, 111));
    assert(selector.controller() == 0 && selector.hardwareId() == 111);
    assert(!selector.update(0, true, 111)); // held edge cannot retrigger
    assert(!selector.update(0, false));
    assert(selector.update(3, true, 444));
    assert(selector.controller() == 3 && selector.hardwareId() == 444);

    // The same physical Joy-Con toggles off, no other sensor is selected.
    assert(!selector.update(3, true, 444));
    assert(!selector.update(3, false));
    assert(selector.update(3, true, 444));
    assert(selector.controller() == -1);
    assert(!selector.update(3, true, 444));

    // A later Joy-Con can select its gyro immediately.
    assert(selector.update(6, true, 666));
    assert(selector.controller() == 6 && selector.hardwareId() == 666);
    assert(selector.rebind(-1)); // disconnected: never use another sensor
    assert(selector.controller() == -1);
    assert(!selector.rebind(-1));
    assert(selector.rebind(2)); // physical identity now in another HID slot
    assert(selector.controller() == 2 && selector.hardwareId() == 666);
    assert(!selector.rebind(2));

    // No serial available: select current controller but never guess another.
    selector.reset();
    assert(selector.update(1, true, 0));
    assert(selector.controller() == 1 && selector.hardwareId() == 0);
    assert(!selector.rebind(4));
    assert(selector.controller() == 1);
    selector.reset();
    assert(selector.controller() == -1 && selector.hardwareId() == 0);

    // Validate all four joycon positions and out-of-range input safety.
    for (int i = 0; i < 4; ++i)
    {
        selector.reset();
        assert(selector.update(i, true, static_cast<std::uint64_t>(100 + i)));
        assert(selector.controller() == i);
        assert(!selector.update(8, true, 999));
        assert(!selector.update(-1, true, 999));
    }
}
