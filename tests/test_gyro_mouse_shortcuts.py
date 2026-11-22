import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class FixedRightJoyConGyroMouseTests(unittest.TestCase):
    def test_only_one_original_mouse_mode_combo(self):
        xml = (ROOT / "resources/xml/tabs/settings.xml").read_text()
        tab = (ROOT / "app/src/settings_tab.cpp").read_text()
        settings = (ROOT / "app/src/utils/Settings.cpp").read_text()
        self.assertIn('id="mouse_input_buttons"', xml)
        self.assertNotIn('id="mouse_input_buttons_left"', xml)
        self.assertNotIn('id="mouse_input_buttons_right"', xml)
        self.assertIn('mouseInputButtons->registerClickAction', tab)
        self.assertIn('saveMouseButtons("mouse_input_buttons",', settings)
        self.assertNotIn('saveMouseButtons("mouse_input_buttons_left"', settings)

    def test_overlay_restored_to_single_original_combo(self):
        view = (ROOT / "app/src/streaming_view.cpp").read_text()
        overlay = (ROOT / "app/src/streaming_input_overlay.cpp").read_text()
        self.assertIn('Settings::instance().mouse_input_options()', view)
        self.assertIn('new StreamingInputOverlay(this, true)', view)
        self.assertNotIn('new StreamingInputOverlay(this, true, controller)', view)
        self.assertIn('Settings::instance().mouse_input_options()', overlay)
        self.assertIn('setGyroMouseActive(true)', overlay)
        self.assertNotIn('gyroSourceController', overlay)

    def test_mouse_gyro_is_restricted_to_first_right_half(self):
        input_source = (ROOT / "app/src/streaming/InputManager.cpp").read_text()
        self.assertIn('constexpr int preferredRight = 1;', input_source)
        self.assertIn('getLogicalJoyConSide(preferredRight)', input_source)
        self.assertIn('brls::HalfJoyConSide::RIGHT', input_source)
        self.assertIn('gyroMouseController = preferredRight;', input_source)
        self.assertIn('if (gyroMouseController == event.controllerIndex)', input_source)
        self.assertNotIn('gyroMouseSource', input_source)
        self.assertNotIn('handleGyroMouseShortcuts(', input_source)

    def test_existing_joycon_assignment_not_modified(self):
        source = (ROOT / "extern/borealis/library/lib/platforms/switch/switch_input.cpp").read_text()
        self.assertIn('stablePhysicalJoyConSeats.resolve(observations)', source)
        self.assertIn('getLogicalJoyConSide(int controller)', source)

if __name__ == '__main__':
    unittest.main()
