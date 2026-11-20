import csv
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from evaluate_gyro_targets import evaluate


class GyroTargetEvaluationTest(unittest.TestCase):
    def test_required_100_traces_and_90_percent(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "trials.csv"
            with path.open("w", newline="", encoding="utf-8") as f:
                writer = csv.writer(f)
                writer.writerow(["target_x", "target_y", "cursor_x", "cursor_y"])
                writer.writerows((0.5, 0.5, 0.5, 0.5) for _ in range(90))
                writer.writerows((0.5, 0.5, 0.8, 0.5) for _ in range(10))
            result = evaluate(path, 0.03)
            self.assertEqual(result["hits"], 90)
            self.assertTrue(result["pass_90_percent"])

    def test_short_run_cannot_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "trials.csv"
            path.write_text(
                "target_x,target_y,cursor_x,cursor_y\n0.5,0.5,0.5,0.5\n",
                encoding="utf-8")
            self.assertFalse(evaluate(path, 0.03)["pass_90_percent"])


if __name__ == "__main__":
    unittest.main()
