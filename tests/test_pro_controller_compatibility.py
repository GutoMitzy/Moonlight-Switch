import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class ProControllerCompatibilityTests(unittest.TestCase):
    def test_fullkey_native_hid_and_input_remains_active(self):
        source = (ROOT / "extern/borealis/library/lib/platforms/switch/switch_input.cpp").read_text()
        self.assertIn("HidNpadStyleTag_NpadFullKey", source)
        self.assertIn("switchKey = full ? SWITCH_BUTTONS_FULL_MAPPING", source)
        self.assertIn("if (!halfMappingEnabled || side == HalfJoyConSide::NONE)", source)
        self.assertIn("sendRumbleInternal(m_vibration_device_handles[padIndex]", source)

    def test_native_mode_does_not_reassign_joycon_hid(self):
        source = (ROOT / "extern/borealis/library/lib/platforms/switch/switch_input.cpp").read_text()
        self.assertIn("if (mode != JoyConInputMode::NATIVE)", source)
        self.assertIn("if (joyConInputMode == JoyConInputMode::PAIRED || nativeInput", source)
        self.assertIn("getConnectedFullControllerCount()", source)
        self.assertIn("resetStreamingControllerOrder()", source)

    def test_pro_count_avoids_joycon_phantom_devices(self):
        count = (ROOT / "app/include/StableVirtualGamepadCount.hpp").read_text()
        self.assertIn("fullControllerCount", count)
        self.assertIn("detected == fullControllerCount", count)
        input_manager = (ROOT / "app/src/streaming/InputManager.cpp").read_text()
        self.assertIn("pro_controller_native_mode()", input_manager)
        self.assertIn("getConnectedFullControllerCount()", input_manager)
        self.assertIn("fullControllerSession", input_manager)
        self.assertIn("resetStreamingControllerOrder()", input_manager)
        self.assertIn("const int firstNewController = newStream ? 0 : previousControllerCount;", input_manager)
        self.assertIn("for (int i = firstNewController; i < controllersCount; i++)", input_manager)
        self.assertIn("LI_CCAP_RUMBLE | LI_CCAP_ACCEL | LI_CCAP_GYRO", input_manager)

    def test_mixed_pro_and_joycon_seat_order_and_motion(self):
        borealis = (ROOT / "extern/borealis/library/lib/platforms/switch/switch_input.cpp").read_text()
        self.assertIn("arrangeHybridControllerSlots(", borealis)
        self.assertIn("fullControllerOrderInitialized", borealis)
        self.assertIn("hadJoyConInput", borealis)
        self.assertIn("HidNpadStyleTag_NpadFullKey", borealis)
        self.assertIn("handleControllerSensors()", borealis)

    def test_settings_persist_native_pro_layout(self):
        settings = (ROOT / "app/src/utils/Settings.cpp").read_text()
        self.assertIn('"pro_controller_native_mode"', settings)
        self.assertIn("brls::JoyConInputMode::NATIVE", settings)
        menu = (ROOT / "app/src/settings_tab.cpp").read_text()
        self.assertIn("Pro Controller (compatibilidade original)", menu)


if __name__ == "__main__":
    unittest.main()
