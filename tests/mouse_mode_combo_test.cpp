#include "MouseModeCombo.hpp"
#include <array>
#include <cassert>
#include <vector>

int main()
{
    using Buttons = std::array<bool, 8>;
    const std::vector<int> leftShortcut = {1, 3, 5};
    const std::vector<int> rightShortcut = {0, 2};
    Buttons left = {};
    Buttons right = {};

    // Nothing triggers with an unconfigured combination.
    assert(!mouseModeComboMatches(std::vector<int>{}, left));
    left[1] = left[3] = true;
    assert(!mouseModeComboMatches(leftShortcut, left));
    left[5] = true;
    assert(mouseModeComboMatches(leftShortcut, left));
    assert(!mouseModeComboMatches(rightShortcut, left));

    // Independent source cannot borrow one key from another Joy-Con.
    right[0] = true;
    assert(!mouseModeComboMatches(rightShortcut, right));
    right[2] = true;
    assert(mouseModeComboMatches(rightShortcut, right));
    right[2] = false;
    assert(!mouseModeComboMatches(rightShortcut, right));

    // Fully custom combinations, including a single arbitrary key.
    assert(mouseModeComboMatches(std::vector<int>{3}, left));
    assert(mouseModeComboMatches(std::vector<int>{0}, right));
    left[3] = false;
    assert(!mouseModeComboMatches(std::vector<int>{3}, left));
}
