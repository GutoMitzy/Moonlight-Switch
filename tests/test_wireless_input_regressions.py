import re
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

    def test_four_stable_host_seats(self):
        source = (ROOT / "app/include/StableVirtualGamepadCount.hpp").read_text()
        self.assertRegex(source, r"if \(detected > 0 .*previous > 0\)\)")
        self.assertIn("return 4;", source)


if __name__ == "__main__":
    unittest.main()
