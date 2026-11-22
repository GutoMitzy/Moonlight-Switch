import argparse
import csv
import json
import math
from pathlib import Path


def evaluate(path, tolerance):
    successes = 0
    count = 0
    errors = []
    with Path(path).open(newline="", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            values = [float(row[k]) for k in
                      ("target_x", "target_y", "cursor_x", "cursor_y")]
            if not all(math.isfinite(value) for value in values):
                raise ValueError("Invalid non-finite coordinate")
            if not all(0.0 <= value <= 1.0 for value in values):
                raise ValueError("Expected normalized coordinates in [0, 1]")
            error = math.hypot(values[0] - values[2], values[1] - values[3])
            errors.append(error)
            successes += error <= tolerance
            count += 1
    if count == 0:
        raise ValueError("No recorded trials")
    return {
        "trials": count,
        "hits": successes,
        "hit_rate_percent": round(100.0 * successes / count, 2),
        "target_tolerance_normalized": tolerance,
        "p95_error_normalized": sorted(errors)[math.ceil(0.95 * count) - 1],
        "pass_90_percent": count >= 100 and successes / count >= 0.90,
        "notes": "Only independently recorded target trials represent real device precision. No synthetic trial is proof of hardware performance."
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("csv", type=Path)
    parser.add_argument("--tolerance", type=float, default=0.01)
    args = parser.parse_args()
    if not (0 < args.tolerance < 1):
        parser.error("--tolerance must be between 0 and 1")
    print(json.dumps(evaluate(args.csv, args.tolerance), indent=2))


if __name__ == "__main__":
    main()
