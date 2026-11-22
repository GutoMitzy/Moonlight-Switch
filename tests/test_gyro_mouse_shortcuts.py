import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class GyroMouseShortcutTests(unittest.TestCase):
    def test_obsolete_ui_is_absent(self):
        xml = (ROOT / "resources/xml/tabs/settings.xml").read_text()
        tab = (ROOT / "app/src/settings_tab.cpp").read_text()
        for old in ("halfcon_mapping", "joycon_players",
                    "gyro_mouse_shortcut_left", "gyro_mouse_shortcut_right"):
            self.assertNotIn('id="' + old + '"', xml)
        self.assertNotIn("new HalfConMappingView()", tab)
        self.assertNotIn("new JoyConPlayersView()", tab)
        self.assertNotIn("gyroShortcutChoices", tab)

    def test_free_form_combinations_in_mouse_mode_settings(self):
        xml = (ROOT / "resources/xml/tabs/settings.xml").read_text()
        tab = (ROOT / "app/src/settings_tab.cpp").read_text()
        for side in ("left", "right"):
            self.assertIn('id="mouse_input_buttons_' + side + '"', xml)
            self.assertIn('mouseInputButtons' + side.title(), tab)
            self.assertIn("mouse_input_options_" + side + "()", tab)
            self.assertIn("set_mouse_input_options_" + side + "(options)", tab)
        self.assertIn("ButtonSelectingDialog::create(", tab)

    def test_settings_migrate_and_save_each_side(self):
        code = (ROOT / "app/src/utils/Settings.cpp").read_text()
        self.assertIn('m_mouse_input_options_left.buttons = m_mouse_input_options.buttons;', code)
        self.assertIn('m_mouse_input_options_right.buttons = m_mouse_input_options.buttons;', code)
        for side in ("left", "right"):
            self.assertIn('"mouse_input_buttons_' + side + '"', code)
        self.assertNotIn('"gyro_mouse_shortcut_left"', code)

    def test_shortcut_opens_with_physical_joycon(self):
        view = (ROOT / "app/src/streaming_view.cpp").read_text()
        self.assertIn("switchInput->updateControllerState(&state, controller)", view)
        self.assertIn("getLogicalJoyConSide(controller)", view)
        self.assertIn("mouseModeComboMatches(options.buttons, state.buttons)", view)
        self.assertIn("new StreamingInputOverlay(this, true, controller)", view)
        self.assertNotIn("gyroMouseSource.update(controller, pressed, id)", view)

    def test_overlay_uses_opening_source_for_recenter_and_exit(self):
        overlay = (ROOT / "app/src/streaming_input_overlay.cpp").read_text()
        self.assertIn("gyroSourceController(sourceController)", overlay)
        self.assertIn("setGyroMouseActive(true, gyroSourceController)", overlay)
        self.assertIn("updateControllerState(&shortcutState, gyroSourceController)", overlay)
        self.assertIn("mouseModeComboMatches(options.buttons, shortcutState.buttons)", overlay)
        self.assertIn("setGyroMouseActive(gyroMouseEnabled, gyroSourceController)", overlay)

    def test_gyro_uses_selected_sensor_only(self):
        source = (ROOT / "app/src/streaming/InputManager.cpp").read_text()
        self.assertIn("if (gyroMouseController == event.controllerIndex)", source)
        self.assertIn("gyroMouseSource.select(sourceController,", source)
        self.assertIn("findControllerForPhysicalJoyConId(", source)
        self.assertIn("getPreferredGyroMouseController()", source)
        self.assertNotIn("gyroMouseSource.update(controller, pressed, id)", source)


if __name__ == "__main__":
    unittest.main()
