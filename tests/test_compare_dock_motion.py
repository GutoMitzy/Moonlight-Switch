import csv
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from compare_dock_motion import compare


class CompareDockMotionTest(unittest.TestCase):
    def test_detects_docked_hid_sample_gaps(self):
        with tempfile.TemporaryDirectory() as base:
            portable = Path(base) / "portable"
            docked = Path(base) / "docked"
            portable.mkdir()
            docked.mkdir()
            for folder, sequence in (
                (portable, [1, 2, 3, 4]),
                (docked, [1, 2, 5, 6]),
            ):
                with (folder / "motion_hid.csv").open("w", newline="") as f:
                    writer = csv.writer(f)
                    writer.writerow([
                        "controller", "pad", "side", "handle", "sample",
                        "accepted", "rest", "dt_ns", "t_us",
                    ])
                    for idx, sample in enumerate(sequence):
                        writer.writerow([0, 0, 2, 1, sample, 1, 0, 5000000,
                                         1000000 + 5000 * idx])

            report = compare(portable, docked)
            self.assertEqual(report["portable"]["missing_sample_sequence"], 0)
            self.assertEqual(report["docked"]["missing_sample_sequence"], 2)
            self.assertEqual(report["portable"]["hid_samples"], 4)
            self.assertEqual(report["docked"]["hid_samples"], 4)


if __name__ == "__main__":
    unittest.main()
