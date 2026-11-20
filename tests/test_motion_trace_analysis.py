import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from analyze_motion_trace import percentile, summarize


class MotionTraceAnalysisTest(unittest.TestCase):
    def test_percentiles(self):
        self.assertEqual(percentile([], 0.95), None)
        self.assertEqual(percentile([4, 1, 2, 3], 0.5), 3)

    def test_csv_summary(self):
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            (directory / "motion_hid.csv").write_text(
                "t_us,controller,pad,side,handle,sample,dt_ns,rest,accepted,gyro_x_rads,gyro_y_rads,gyro_z_rads\n"
                "1000,0,0,1,0,10,5000000,1,1,0.01,0.0,0.0\n"
                "2000,0,0,1,0,11,5000000,1,1,0.01,0.0,0.0\n"
                "3000,0,0,1,0,13,5000000,0,1,0.6,0.0,0.0\n")
            (directory / "motion_send.csv").write_text(
                "t_us,controller,sample,type,return_code\n"
                "1100,0,10,2,0\n2100,0,11,2,0\n3100,0,13,2,0\n")
            (directory / "motion_pointer.csv").write_text(
                "controller,dt_s,accepted,cursor_x,cursor_y\n"
                "0,0.005,1,0.5,0.5\n0,0.005,1,0.6,0.5\n")
            result = summarize(directory)
            hid = result["hid"]["0/0/1/0"]
            self.assertEqual(hid["missing_sample_sequence"], 1)
            self.assertEqual(hid["stationary_samples"], 2)
            self.assertAlmostEqual(hid["stationary_gyro_rads_rms"], 0.01)
            self.assertEqual(result["send"]["0/2"]["local_hid_to_enqueue_ms_p50"], 0.1)
            self.assertEqual(result["pointer"]["0"]["rows"], 2)


if __name__ == "__main__":
    unittest.main()
