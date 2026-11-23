import os
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class WirelessInputRegressionTests(unittest.TestCase):
    def test_system_event_is_cleared_before_early_returns(self):
        source = (ROOT / "extern/borealis/library/lib/platforms/switch/switch_input.cpp").read_text()
        function = source.split("void SwitchInputManager::processSystemButtonEvent(", 1)[1]
        before_first_return = function.split("return;", 1)[0]
        self.assertIn("eventClear(event);", before_first_return)
        self.assertIn("resolveWirelessHomeOwner(owner, candidates, GAMEPADS_MAX)", function)

    def test_recovered_gyro_samples_do_not_build_network_backlog(self):
        source = (ROOT / "app/src/streaming/InputManager.cpp").read_text()
        sensor_callback = source.split("->getControllerSensorStateChanged()", 1)[1]
        self.assertIn("if (!event.latestInBatch) return;", sensor_callback)
        self.assertLess(sensor_callback.index("gyroAimPointer.update("),
                        sensor_callback.index("if (!event.latestInBatch) return;"))
        self.assertLess(sensor_callback.index("if (!event.latestInBatch) return;"),
                        sensor_callback.index("LiSendControllerMotionEvent("))

    def test_controller_state_is_resent_after_queue_failure_and_focus_return(self):
        source = (ROOT / "app/src/streaming/InputManager.cpp").read_text()
        drop = source.split("void MoonlightInputManager::dropInput()", 1)[1]
        drop = drop.split("// Drop touchscreen mouse state", 1)[0]
        drop_loop = drop[drop.index("    for (short i = 0;"):]

        handler = source.split("void MoonlightInputManager::handleControllers(", 1)[1]
        handler = handler.split("void MoonlightInputManager::handleInput(", 1)[0]
        send_loop = handler[handler.rindex("    for (int i = 0;"):]
        header = (ROOT / "app/src/streaming/InputManager.hpp").read_text()
        gamepad = header.split("struct GamepadState", 1)[1].split("struct MouseStateS", 1)[0]
        fixture = """
#include <array>
#include <cassert>
#include <utility>
#include <vector>

struct GamepadState
"""
        fixture += gamepad
        fixture += """
namespace brls {
struct Logger {
    static void debug(const char*, int) {}
};
}

std::vector<std::pair<int, int>> sent;
bool failNext = false;
int LiSendMultiControllerEvent(int index, short mask, int flags, int, int, int, int, int, int) {
    assert(mask == 3);
    sent.emplace_back(index, flags);
    return std::exchange(failNext, false) ? 1 : 0;
}

std::array<bool, 2> pendingGamepadStates{};
GamepadState lastGamepadStates[2];
std::array<GamepadState, 2> states{};
constexpr int controllersCount = 2;
constexpr short mappedControllersCount = 3;

void send(bool controllerSetChanged) {
"""
        fixture += send_loop
        fixture += """
void drop() {
    GamepadState gamepadState{};
    const short activeMask = mappedControllersCount;
    bool res = true;
"""
        fixture += drop_loop
        fixture += """
    assert(res);
}

int main() {
    states[0].buttonFlags = 1;
    states[1].buttonFlags = 2;
    send(true);
    sent.clear();

    states[0].buttonFlags = 0;
    failNext = true;
    send(false);
    assert(lastGamepadStates[0].buttonFlags == 1);
    send(false);
    const std::vector<std::pair<int, int>> retry = {{0, 0}, {0, 0}};
    assert(sent == retry);

    send(false);
    assert(sent == retry);
    sent.clear();

    drop();
    send(false);
    const std::vector<std::pair<int, int>> resumed = {
        {0, 0}, {1, 0}, {0, 0}, {1, 2},
    };
    assert(sent == resumed);
}
"""
        with tempfile.TemporaryDirectory() as work:
            path = Path(work) / "gamepad.cpp"
            binary = Path(work) / "gamepad"
            path.write_text(fixture)
            subprocess.run(
                [os.environ.get("CXX", "c++"), "-std=c++20", "-Wall", "-Wextra", "-Werror",
                 str(path), "-o", str(binary)],
                check=True,
            )
            subprocess.run([str(binary)], check=True)

    def test_four_stable_host_seats(self):
        source = (ROOT / "app/include/StableVirtualGamepadCount.hpp").read_text()
        # v20 reserves four detached Joy-Con seats from stream startup,
        # while preserving the native FullKey/Pro Controller count.
        self.assertIn("if (!separateJoyCons || handheld)", source)
        self.assertIn("if (fullControllerCount > 0 && detected == fullControllerCount)", source)
        self.assertIn("return std::max(4, detected);", source)
        self.assertIn("return std::max(detected, previous);", source)


if __name__ == "__main__":
    unittest.main()
