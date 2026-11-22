#pragma once
#include <array>
#include <cstdint>

// A mouse source is explicitly selected by a rising physical shortcut edge,
// never by "first movement wins". A known hardware ID is used to rebind the
// source if Nintendo HID reorders its logical gamepad indices.
class GyroMouseSourceSelection
{
public:
    static constexpr int maxControllers = 8;

    void reset()
    {
        activeController = -1;
        activeHardwareId = 0;
        held.fill(false);
    }

    void select(int controller, std::uint64_t hardwareId = 0)
    {
        reset();
        if (controller < 0 || controller >= maxControllers)
            return;
        activeController = controller;
        activeHardwareId = hardwareId;
    }

    bool update(int controller, bool pressed, std::uint64_t hardwareId = 0)
    {
        if (controller < 0 || controller >= maxControllers)
            return false;

        const bool rising = pressed && !held[controller];
        held[controller] = pressed;
        if (!rising)
            return false;

        if (controller == activeController &&
            (!hardwareId || !activeHardwareId || hardwareId == activeHardwareId))
        {
            activeController = -1;
            activeHardwareId = 0;
        }
        else
        {
            activeController = controller;
            activeHardwareId = hardwareId;
        }
        return true;
    }

    bool rebind(int newController)
    {
        if (!activeHardwareId)
            return false; // Anonymous HID: don't guess another physical pad.
        if (newController < -1 || newController >= maxControllers)
            return false;
        if (newController == activeController)
            return false;
        activeController = newController;
        return true;
    }

    [[nodiscard]] bool isHeld(int controller) const {
        return controller >= 0 && controller < maxControllers &&
               held[controller];
    }
    [[nodiscard]] int controller() const { return activeController; }
    [[nodiscard]] std::uint64_t hardwareId() const { return activeHardwareId; }

private:
    int activeController = -1;
    std::uint64_t activeHardwareId = 0;
    std::array<bool, maxControllers> held{};
};
