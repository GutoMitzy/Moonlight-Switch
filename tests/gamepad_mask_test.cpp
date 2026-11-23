#include "GamepadMask.hpp"

static_assert(gamepadMaskForCount(-1) == 0); // Before arrival
static_assert(gamepadMaskForCount(0) == 0);  // Nothing connected
static_assert(gamepadMaskForCount(1) == 1);  // Pro Controller or handheld
static_assert(gamepadMaskForCount(4) == 0xf); // Joy-Con seats
static_assert(gamepadMaskForCount(5) == 0x1f); // Mixed session
static_assert(gamepadMaskForCount(16) == 0xffff);
static_assert(gamepadMaskForCount(32) == 0xffff);

int main() {}
