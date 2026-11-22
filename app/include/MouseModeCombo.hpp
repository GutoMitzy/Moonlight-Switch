#pragma once
#include <vector>

// Use the same button matching rules when opening and closing mouse mode.
// Evaluate buttons belonging to ONE logical controller, never the unified
// state merged from several Joy-Cons.
template<typename Button, typename ButtonState>
bool mouseModeComboMatches(const std::vector<Button>& combo,
                           const ButtonState& pressed)
{
    if (combo.empty()) return false;
    for (const auto button : combo)
        if (!pressed[button])
            return false;
    return true;
}
