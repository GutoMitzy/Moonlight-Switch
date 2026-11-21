import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class TraceSchemaTests(unittest.TestCase):
    def test_hid_schema(self):
        path = ROOT / "extern/borealis/library/lib/platforms/switch/switch_input.cpp"
        source = path.read_text(encoding="utf-8")
        header = re.search(r'"[^"]*motion_hid\.csv",\s*"([^"]+)"', source)
        self.assertIsNotNone(header)
        self.assertEqual(len(header.group(1).split(",")), 36)
        fmt = re.search(r'motionTrace\.write\("([^"]+)"', source)
        self.assertIsNotNone(fmt)
        self.assertEqual(len(re.findall(r"%[-+0-9.]*[a-zA-Z]", fmt.group(1))), 36)
        # The docked motion path reads bounded HID history, rather than one
        # latest-only sample. This regression check must reflect the v18 API.
        self.assertRegex(
            source,
            r"hidGetSixAxisSensorStates\(padSixAxis\.handles\[sensor\],"
            r"\s*sixaxis,\s*maxSamplesPerPoll\)",
        )
        self.assertIn("constexpr std::size_t maxSamplesPerPoll = 8;", source)
        self.assertIn("forEachOldestMotionSample(sixaxis, samples,", source)
        self.assertIn("motionSampleIntervalSeconds(sixaxis.delta_time)", source)
        self.assertIn("if (!accepted)\n            return;", source)

    def test_network_schema_and_unchanged_values(self):
        source = (ROOT / "app/src/streaming/InputManager.cpp").read_text(encoding="utf-8")
        header = re.search(r'"[^"]*motion_send\.csv",\s*"([^"]+)"', source)
        self.assertIsNotNone(header)
        self.assertEqual(len(header.group(1).split(",")), 12)
        fmt = re.search(r'networkTrace\.write\("([^"]+)"', source)
        self.assertEqual(len(re.findall(r"%[-+0-9.]*[a-zA-Z]", fmt.group(1))), 12)
        for axis in "012":
            self.assertIn(f"event.data[{axis}] * 57.2957795f * gyroScale", source)

    def test_pointer_schema_and_timestamp(self):
        source = (ROOT / "app/src/streaming/InputManager.cpp").read_text(encoding="utf-8")
        header = re.search(r'"[^"]*motion_pointer\.csv",\s*"([^"]+)"', source)
        self.assertIsNotNone(header)
        self.assertEqual(len(header.group(1).split(",")), 11)
        fmt = re.search(r'pointerTrace\.write\("([^"]+)"', source)
        self.assertEqual(len(re.findall(r"%[-+0-9.]*[a-zA-Z]", fmt.group(1))), 11)
        self.assertIn("now - gyroMouseLastSample", source)


if __name__ == "__main__":
    unittest.main()
