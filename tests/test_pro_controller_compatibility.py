import os
import subprocess
import tempfile
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

    def test_fullkey_arrival_type_and_existing_slots(self):
        input_manager = (ROOT / "app/src/streaming/InputManager.cpp").read_text()
        self.assertIn("switchInput->isFullKeyController(i)", input_manager)
        self.assertIn("type = LI_CTYPE_XBOX;", input_manager)
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

    def test_switch_topology_with_simulated_hid_and_serial_failure(self):
        include = ROOT / "extern/borealis/library/include"
        source = (ROOT / "extern/borealis/library/lib/platforms/switch/switch_input.cpp").read_text()
        reset = source[source.index("void SwitchInputManager::resetStreamingControllerOrder()"):
                       source.index("int SwitchInputManager::getConnectedFullControllerCount()")]
        topology = source[source.index("std::vector<StableJoyConSlot> SwitchInputManager::collectStableJoyConSlots()"):
                          source.index("void SwitchInputManager::updateControllerState(")]
        model = r"""
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <utility>
#include <borealis/platforms/switch/hybrid_controller_slots.hpp>
#include <borealis/platforms/switch/stable_joycon_identity.hpp>

using namespace brls;
using u32 = std::uint32_t;
using s32 = std::int32_t;
using Result = int;
using HidNpadIdType = int;
using HidsysUniquePadId = int;
enum HidsysUniquePadType {
    HidsysUniquePadType_Embedded, HidsysUniquePadType_LeftController,
    HidsysUniquePadType_RightController
};
enum class HalfJoyConSide { NONE, LEFT, RIGHT };
enum class JoyConInputMode { SEPARATE, PAIRED, NATIVE };
constexpr int GAMEPADS_MAX = 5;
constexpr u32 HidNpadStyleTag_NpadFullKey = 1;
constexpr u32 HidNpadStyleTag_NpadJoyDual = 2;
struct PadState { u32 style_set = 0; bool active_handheld = false; };
void padUpdate(PadState*) {}
u32 padGetStyleSet(const PadState* pad) { return pad->style_set; }
int padGetAttributes(const PadState*) { return 0; }
u32 joyDualConnectedSideMask(int, int) { return 3; }
HalfJoyConSide halfJoyConSide(u32) { return HalfJoyConSide::NONE; }
bool serialLookupFails = false;
Result hidsysGetUniquePadsFromNpad(int id, int* physical, int, s32* count) {
    assert(id == 0); // The full-size pad at index 1 must use the native path.
    if (serialLookupFails) return 1;
    physical[0] = 1;
    physical[1] = 2;
    *count = 2;
    return 0;
}
Result hidsysGetUniquePadType(int id, HidsysUniquePadType* type) {
    *type = static_cast<HidsysUniquePadType>(id);
    return 0;
}
std::uint64_t stableJoyConHardwareId(int id) { return (1ULL << 63) | id; }
bool R_FAILED(Result result) { return result != 0; }
struct Logger {
    template<class... Args> static void info(Args...) {}
    template<class... Args> static void warning(Args...) {}
};
struct SensorQuality { void reset() {} void beginCalibration() {} };
struct Sensors { SensorQuality quality[2]; };
struct SwitchInputManager {
    PadState padsState[GAMEPADS_MAX], padStateHandheld;
    Sensors m_pad_six_axis[GAMEPADS_MAX];
    StableJoyConIdentitySlots stablePhysicalJoyConSeats;
    std::vector<StableJoyConSlot> cachedLogicalSlots, cachedJoyConSeats;
    std::array<std::uint64_t, GAMEPADS_MAX> cachedNpadTopology{};
    std::array<std::array<std::uint64_t, 2>, GAMEPADS_MAX> sensorPhysicalIds{};
    std::chrono::steady_clock::time_point lastPhysicalIdentityRefresh{};
    bool logicalSlotsCached = false, fullControllerOrderInitialized = false;
    bool fullControllerFirst = false, hadJoyConInput = false;
    int cachedFullControllerCount = 0;
    std::size_t reservedFullControllerSeats = 0;
    JoyConInputMode joyConInputMode = JoyConInputMode::SEPARATE;
    void persistStableJoyConSeats() {}
    void resetStreamingControllerOrder();
    std::vector<StableJoyConSlot> collectStableJoyConSlots();
    int handheldControllerIndex(const std::vector<StableJoyConSlot>&) const;
    short getControllersConnectedCount();
    bool resolveController(int, PadState**, int*, HalfJoyConSide*);
};

bool resolvesTo(SwitchInputManager& input, int controller, int hidPad, HalfJoyConSide expectedSide) {
    PadState* pad = nullptr;
    int index = -2;
    HalfJoyConSide side = HalfJoyConSide::NONE;
    return input.resolveController(controller, &pad, &index, &side) &&
        index == hidPad && side == expectedSide;
}
"""
        scenarios = r"""
int main() {
    SwitchInputManager input;
    assert(input.getControllersConnectedCount() == 0);
    input.padsState[1].style_set = HidNpadStyleTag_NpadFullKey;
    assert(input.getControllersConnectedCount() == 1);
    assert(resolvesTo(input, 0, 1, HalfJoyConSide::NONE));
    input.padsState[1].style_set = 0;
    assert(input.getControllersConnectedCount() == 0);
    input.padsState[1].style_set = HidNpadStyleTag_NpadFullKey;
    input.padStateHandheld.active_handheld = true;
    assert(input.getControllersConnectedCount() == 2);
    assert(resolvesTo(input, 0, 1, HalfJoyConSide::NONE));
    assert(resolvesTo(input, 1, -1, HalfJoyConSide::NONE));

    input.padStateHandheld.active_handheld = false;
    input.padsState[1].style_set = 0;
    input.resetStreamingControllerOrder();
    input.padsState[0].style_set = HidNpadStyleTag_NpadJoyDual;
    assert(input.getControllersConnectedCount() == 4);
    assert(resolvesTo(input, 1, 0, HalfJoyConSide::RIGHT));
    serialLookupFails = true;
    input.padsState[1].style_set = HidNpadStyleTag_NpadFullKey;
    assert(input.getControllersConnectedCount() == 5);
    assert(resolvesTo(input, 1, 0, HalfJoyConSide::RIGHT));
    assert(resolvesTo(input, 4, 1, HalfJoyConSide::NONE));
    input.padsState[1].style_set = 0;
    assert(input.getControllersConnectedCount() == 5);
    assert(!resolvesTo(input, 4, 1, HalfJoyConSide::NONE));
    input.padsState[0].style_set = 0;
    assert(input.getControllersConnectedCount() == 5); // Held until the next stream.
    input.resetStreamingControllerOrder();
    assert(input.getControllersConnectedCount() == 0);
    input.padsState[1].style_set = HidNpadStyleTag_NpadFullKey;
    assert(input.getControllersConnectedCount() == 1);
    input.padsState[0].style_set = HidNpadStyleTag_NpadJoyDual;
    assert(input.getControllersConnectedCount() == 5);
    assert(resolvesTo(input, 0, 1, HalfJoyConSide::NONE));
    assert(resolvesTo(input, 2, 0, HalfJoyConSide::RIGHT));

    SwitchInputManager handheldFirst;
    handheldFirst.padStateHandheld.active_handheld = true;
    assert(handheldFirst.getControllersConnectedCount() == 1);
    handheldFirst.padsState[1].style_set = HidNpadStyleTag_NpadFullKey;
    assert(handheldFirst.getControllersConnectedCount() == 2);
    assert(resolvesTo(handheldFirst, 0, -1, HalfJoyConSide::NONE));
    assert(resolvesTo(handheldFirst, 1, 1, HalfJoyConSide::NONE));
}
"""
        with tempfile.TemporaryDirectory() as work:
            fixture = Path(work) / "topology.cpp"
            binary = Path(work) / "topology"
            fixture.write_text(model + reset + topology + scenarios)
            subprocess.run(
                [os.environ.get("CXX", "c++"), "-std=c++20", "-Wall", "-Wextra", "-Werror",
                 "-I", str(include), str(fixture), "-o", str(binary)],
                check=True,
            )
            subprocess.run([str(binary)], check=True)

    def test_settings_persist_native_pro_layout(self):
        settings = (ROOT / "app/src/utils/Settings.cpp").read_text()
        self.assertIn('"pro_controller_native_mode"', settings)
        self.assertIn("brls::JoyConInputMode::NATIVE", settings)
        menu = (ROOT / "app/src/settings_tab.cpp").read_text()
        self.assertIn("Pro Controller (compatibilidade original)", menu)


if __name__ == "__main__":
    unittest.main()
