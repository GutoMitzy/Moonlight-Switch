import argparse
import csv
import json
import math
import statistics
from collections import defaultdict
from pathlib import Path


def percentile(values, fraction):
    if not values:
        return None
    sorted_values = sorted(values)
    return round(sorted_values[round((len(sorted_values) - 1) * fraction)], 3)


def summarize(folder):
    folder = Path(folder)
    hid_path = folder / "motion_hid.csv"
    send_path = folder / "motion_send.csv"
    pointer_path = folder / "motion_pointer.csv"

    summary = {
        "notes": [
            "Os tempos HID->Moonlight medem apenas etapas no Switch.",
            "O atraso completo ate o jogo exige instrumentacao no PC ou video sincronizado.",
            "Aceleração inclui gravidade e nao mede posicao espacial diretamente."
        ],
        "hid": {},
        "send": {},
        "pointer": {}
    }
    hid_samples = {}
    hid_data = defaultdict(lambda: {
        "rows": 0, "rejected": 0, "duplicates": 0, "gaps": 0,
        "rest_gyro_squared": [], "rest_gyro_peak": 0.0,
        "dt_ms": [], "last_sample": None
    })
    if hid_path.exists():
        with hid_path.open(newline="", encoding="utf-8") as f:
            for row in csv.DictReader(f):
                group = (row["controller"], row["pad"], row["side"], row["handle"])
                g = hid_data[group]
                g["rows"] += 1
                seq = int(row["sample"])
                previous = g["last_sample"]
                if previous is not None:
                    if seq <= previous:
                        g["duplicates"] += 1
                    elif seq > previous + 1:
                        g["gaps"] += seq - previous - 1
                g["last_sample"] = seq
                if row["accepted"] == "0":
                    g["rejected"] += 1
                if row["rest"] == "1" and row["accepted"] == "1":
                    omega = [float(row[k]) for k in ("gyro_x_rads", "gyro_y_rads", "gyro_z_rads")]
                    mag_sq = sum(v * v for v in omega)
                    g["rest_gyro_squared"].append(mag_sq)
                    g["rest_gyro_peak"] = max(g["rest_gyro_peak"], math.sqrt(mag_sq))
                dt = int(row["dt_ns"]) / 1000000
                if 0 < dt < 250:
                    g["dt_ms"].append(dt)
                if row["accepted"] == "1":
                    hid_samples[(row["controller"], seq)] = int(row["t_us"])
        for key, g in hid_data.items():
            m = g["rest_gyro_squared"]
            summary["hid"]["/".join(key)] = {
                "rows": g["rows"],
                "rejected": g["rejected"],
                "out_of_order_or_duplicates": g["duplicates"],
                "missing_sample_sequence": g["gaps"],
                "sensor_dt_ms_p50": percentile(g["dt_ms"], 0.50),
                "sensor_dt_ms_p95": percentile(g["dt_ms"], 0.95),
                "stationary_samples": len(m),
                "stationary_gyro_rads_rms": round(math.sqrt(statistics.mean(m)), 6) if m else None,
                "stationary_gyro_rads_peak": round(g["rest_gyro_peak"], 6) if m else None
            }

    send_data = defaultdict(lambda: {"rows": 0, "errors": 0, "interval_us": [],
                                    "last_us": None, "local_delay_us": []})
    if send_path.exists():
        with send_path.open(newline="", encoding="utf-8") as f:
            for row in csv.DictReader(f):
                key = (row["controller"], row["type"])
                g = send_data[key]
                g["rows"] += 1
                if int(row["return_code"]) < 0:
                    g["errors"] += 1
                t = int(row["t_us"])
                if g["last_us"] is not None and 0 < t - g["last_us"] < 1000000:
                    g["interval_us"].append(t - g["last_us"])
                g["last_us"] = t
                sensor_time = hid_samples.get((row["controller"], int(row["sample"])))
                if sensor_time is not None and 0 <= t - sensor_time < 1000000:
                    g["local_delay_us"].append(t - sensor_time)
        for key, g in send_data.items():
            summary["send"]["/".join(key)] = {
                "rows": g["rows"],
                "negative_return_codes": g["errors"],
                "enqueue_interval_ms_p50": percentile([x / 1000 for x in g["interval_us"]], 0.5),
                "enqueue_interval_ms_p95": percentile([x / 1000 for x in g["interval_us"]], 0.95),
                "local_hid_to_enqueue_ms_p50": percentile([x / 1000 for x in g["local_delay_us"]], 0.5),
                "local_hid_to_enqueue_ms_p95": percentile([x / 1000 for x in g["local_delay_us"]], 0.95)
            }

    pointer_data = defaultdict(lambda: {"rows": 0, "rejected": 0,
                                        "dt_ms": [], "motion_px_1280": [],
                                        "previous": None})
    if pointer_path.exists():
        with pointer_path.open(newline="", encoding="utf-8") as f:
            for row in csv.DictReader(f):
                g = pointer_data[row["controller"]]
                g["rows"] += 1
                g["rejected"] += int(row["accepted"] == "0")
                g["dt_ms"].append(float(row["dt_s"]) * 1000)
                xy = (float(row["cursor_x"]), float(row["cursor_y"]))
                if g["previous"] is not None:
                    dx = (xy[0] - g["previous"][0]) * 1280
                    dy = (xy[1] - g["previous"][1]) * 720
                    g["motion_px_1280"].append(math.hypot(dx, dy))
                g["previous"] = xy
        for key, g in pointer_data.items():
            summary["pointer"][key] = {
                "rows": g["rows"],
                "rejected": g["rejected"],
                "sample_dt_ms_p50": percentile(g["dt_ms"], 0.5),
                "sample_dt_ms_p95": percentile(g["dt_ms"], 0.95),
                "cursor_step_px_p50_at_1280x720": percentile(g["motion_px_1280"], 0.5),
                "cursor_step_px_p95_at_1280x720": percentile(g["motion_px_1280"], 0.95)
            }

    return summary


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("folder", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    report = json.dumps(summarize(args.folder), indent=2, ensure_ascii=False)
    if args.output:
        args.output.write_text(report + "\n", encoding="utf-8")
    else:
        print(report)


if __name__ == "__main__":
    main()
