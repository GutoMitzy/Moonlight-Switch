#include "GyroCalibrationHold.hpp"

#include <cassert>
#include <chrono>

int main()
{
    using Clock = GyroCalibrationHold::Clock;
    const auto start = Clock::time_point{} + std::chrono::seconds(10);
    GyroCalibrationHold player1;
    GyroCalibrationHold player2;

    assert(!player1.update(false, start));
    assert(!player1.update(true, start));
    assert(!player1.update(true, start + std::chrono::milliseconds(1999)));
    assert(player1.update(true, start + std::chrono::seconds(2)));
    assert(!player1.update(true, start + std::chrono::seconds(5)));
    assert(!player2.update(true, start + std::chrono::seconds(5)));
    assert(!player1.update(false, start + std::chrono::seconds(6)));
    assert(!player1.update(true, start + std::chrono::seconds(7)));
    assert(!player1.update(false, start + std::chrono::seconds(8)));
    assert(!player1.update(true, start + std::chrono::seconds(9)));
    assert(player1.update(true, start + std::chrono::seconds(11)));
    assert(player2.update(true, start + std::chrono::seconds(7)));
    assert(!player2.update(true, start + std::chrono::seconds(8)));
}
