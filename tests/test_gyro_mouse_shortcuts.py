import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class GyroMousePhysicalShortcutTests(unittest.TestCase):
    def test_switch_hid_reads_are_side_specific(self):
        source = (ROOT / "extern/borealis/library/lib/platforms/switch/switch_input.cpp").read_text()
        start = source.index("bool SwitchInputManager::isJoyConGyroMouseShortcutPressed(")
        end = source.index("std::uint64_t SwitchInputManager::getPhysicalJoyConId", start)
        function = source[start:end]
        self.assertIn("resolveController(controller, &pad, &padIndex, &side)", function)
        self.assertIn("HidNpadButton_LeftSL", function)
        self.assertIn("HidNpadButton_RightSL", function)
        self.assertIn("readJoyConSideButtonFallback(padIndex, side)", function)

    def test_per_side_shortcut_settings_are_persisted(self):
        source = (ROOT / "app/src/utils/Settings.cpp").read_text()
        for side in ("left", "right"):
            self.assertIn('"gyro_mouse_shortcut_' + side + '"', source)
        menu = (ROOT / "app/src/settings_tab.cpp").read_text()
        self.assertIn("gyroMouseShortcutLeft->setSelection", menu)
        self.assertIn("gyroMouseShortcutRight->setSelection", menu)

    def test_source_exclusive_and_no_motion_autopick(self):
        source = (ROOT / "app/src/streaming/InputManager.cpp").read_text()
        self.assertIn("if (gyroMouseController == event.controllerIndex)", source)
        self.assertIn("gyroMouseSource.update(controller, pressed, id)", source)
        self.assertIn("findControllerForPhysicalJoyConId(", source)
        self.assertNotIn("getPreferredGyroMouseController();", source)
        self.assertNotIn("const int preferred = switchInput->getPreferredGyroMouseController();", source)

    def test_overlay_recenter_is_preserved(self):
        source = (ROOT / "app/src/streaming/InputManager.cpp").read_text()
        setter = source.split("void MoonlightInputManager::setGyroMouseActive(bool active)", 1)[1]
        setter = setter.split("std::optional<brls::Point>", 1)[0]
        self.assertIn("gyroAimPointer.reset();", setter)
        self.assertNotIn("if (gyroMouseActive == active)", setter)


if __name__ == "__main__":
    unittest.main()
