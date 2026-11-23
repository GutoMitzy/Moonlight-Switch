#pragma once

#include <algorithm>
#include <cstdint>

constexpr std::uint16_t gamepadMaskForCount(int count)
{
    return static_cast<std::uint16_t>((1U << std::clamp(count, 0, 16)) - 1U);
}
