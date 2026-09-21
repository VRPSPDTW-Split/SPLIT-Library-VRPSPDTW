#!/usr/bin/env python3

import argparse
import csv
import re
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import FuncFormatter


SETTINGS = (
    (100, 10),
    (200, 20),
    (500, 50),
    (1000, 100),
    (2000, 200),
    (5000, 500),
    (10000, 1000),
    (20000, 2000),
    (50000, 5000),
    (100000, 10000),
)


def arguments():
    here = Path(__file__).resolve().parent
    project = here.parent
    parser = argparse.ArgumentParser(
        description="Plot hard VRPSPDTW speedups for the ten capacity/window settings."
    )
    parser.add_argument("--input", type=Path, default=here / "timing_results.csv")
    parser.add_argument(
        "--instances-dir",
        type=Path,
        default=project / "Instances-Full-vrpspdtw",
    )
    parser.add_argument(
        "--output", type=Path, default=here / "hard_vrpspdtw_speedup.png"
    )
    return parser.parse_args()


def read_dimensions(directory):
    dimensions = {}
    suffix = "_05_vrpspdtw.gt"
    for instance_path in directory.glob(f"*{suffix}"):
        instance = instance_path.name[:-len(suffix)]
        with instance_path.open(encoding="utf-8") as source:
            for line in source:
                match = re.fullmatch(r"DIMENSION\s*:\s*(\d+)\s*", line)
                if match:
                    dimensions[instance] = int(match.group(1))
                    break
                if line.startswith("GIANT_TOUR_SECTION"):
                    break
        if instance not in dimensions:
            raise ValueError(f"No DIMENSION found in {instance_path}")
    return dimensions


def clean_tick_format(value, _position):
    if value >= 1:
        return f"{int(value)}"
    return f"{value:.1f}"


def main():
    options = arguments()
    dimensions = read_dimensions(options.instances_dir)
    with options.input.open(newline="", encoding="utf-8") as source:
        rows = list(csv.DictReader(source))
    if len(rows) != 105:
        raise ValueError(f"Expected 105 instances, found {len(rows)}")

    for row in rows:
        if row["instance"] not in dimensions:
            raise ValueError(f"No instance file found for {row['instance']}")
    rows.sort(key=lambda row: (dimensions[row["instance"]], row["instance"]))
    instance_sizes = np.array([dimensions[row["instance"]] for row in rows])

    y_ticks = [2**power for power in range(-2, 12)]
    x_ticks = [10**2, 10**3, 10**4, 10**5]
    x_tick_labels = ["2", "3", "4", "5"]

    plt.rcParams.update({"font.size": 12})
    figure, axes = plt.subplots(1, 10, figsize=(12, 5), sharey=True)
    figure.subplots_adjust(wspace=0.15, bottom=0.25)

    for index, (axis, (capacity, window_factor)) in enumerate(zip(axes, SETTINGS)):
        prefix = f"hard_vrpspdtw_q{capacity}_wf{window_factor}"
        bellman = np.array([float(row[f"{prefix}_bellman_ms"]) for row in rows])
        linear = np.array([float(row[f"{prefix}_linear_ms"]) for row in rows])
        if np.any(linear <= 0):
            raise ValueError(f"Non-positive linear timing for Q={capacity}")
        speedup = bellman / linear

        axis.set_xscale("log")
        axis.set_yscale("log")
        axis.scatter(
            instance_sizes,
            speedup,
            color="black",
            s=10,
            marker="o",
            zorder=3,
        )

        axis.set_xticks(x_ticks)
        axis.set_xticklabels(x_tick_labels)
        axis.tick_params(axis="x", pad=10)
        axis.set_yticks(y_ticks)
        axis.yaxis.set_major_formatter(FuncFormatter(clean_tick_format))
        axis.set_ylim([y_ticks[0], y_ticks[-1]])
        axis.set_facecolor("#f0f0f0")

        for value in y_ticks:
            axis.axhline(
                y=value,
                color="white",
                linestyle="--",
                linewidth=0.7,
                zorder=1,
            )
        for value in x_ticks:
            axis.axvline(
                x=value,
                color="white",
                linestyle="--",
                linewidth=0.7,
                zorder=1,
            )

        axis.set_title(f"{capacity}\n{window_factor}", pad=16)
        if index == 0:
            axis.set_ylabel("Speedup")
            axis.tick_params(labelleft=True)
        else:
            axis.tick_params(labelleft=False)
        axis.set_xlabel("")

    figure.text(0.13, 0.155, "n=10^", ha="right")
    figure.text(0.12, 0.93, "Q=\nb_i*=", ha="right")

    options.output.parent.mkdir(parents=True, exist_ok=True)
    figure.savefig(
        options.output,
        dpi=300,
        bbox_inches="tight",
        pad_inches=0.0,
    )
    plt.close(figure)
    print(options.output)


if __name__ == "__main__":
    main()
