import argparse
import json
from pathlib import Path

from analyze_motion_trace import summarize


def mode_metrics(folder):
    report = summarize(folder)
    hid = list(report["hid"].values())
    send = list(report["send"].values())
    pointer = list(report["pointer"].values())

    def maximum(items, name):
        values = [x[name] for x in items if x.get(name) is not None]
        return max(values) if values else None

    return {
        "hid_samples": sum(x["rows"] for x in hid),
        "missing_sample_sequence": sum(x["missing_sample_sequence"] for x in hid),
        "rejected_samples": sum(x["rejected"] for x in hid),
        "worst_sensor_dt_p95_ms": maximum(hid, "sensor_dt_ms_p95"),
        "worst_stationary_gyro_rms": maximum(hid, "stationary_gyro_rads_rms"),
        "motion_enqueue_errors": sum(x["negative_return_codes"] for x in send),
        "worst_enqueue_interval_p95_ms": maximum(send, "enqueue_interval_ms_p95"),
        "worst_cursor_step_p95_px_at_1280": maximum(
            pointer, "cursor_step_px_p95_at_1280x720"),
        "traced_controllers": len(hid),
    }


def compare(portable, docked):
    portable_metrics = mode_metrics(portable)
    docked_metrics = mode_metrics(docked)
    return {
        "portable": portable_metrics,
        "docked": docked_metrics,
        "limitations": [
            "Both capture folders require matched test duration and motions.",
            "Sample sequence gaps count missed HID samples, not network loss.",
            "The source traces do not measure end-to-end motion latency.",
            "Do not compare absolute sample totals between unmatched controller layouts.",
        ],
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("portable", type=Path)
    parser.add_argument("docked", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    report = json.dumps(compare(args.portable, args.docked),
                        indent=2, ensure_ascii=False)
    if args.output:
        args.output.write_text(report + "\n", encoding="utf-8")
    else:
        print(report)


if __name__ == "__main__":
    main()
