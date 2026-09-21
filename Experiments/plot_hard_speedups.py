#!/usr/bin/env python3

import argparse
import csv
import math
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

VARIANTS = (
    ("vrpspd", "VRPSPD", False),
    ("vrptw", "VRPTW", True),
    ("vrpspdtw", "VRPSPDTW", True),
)


def arguments():
    here = Path(__file__).resolve().parent
    project = here.parent
    parser = argparse.ArgumentParser(
        description="Plot manuscript-style speedup figures for the hard Split variants."
    )
    parser.add_argument("--input", type=Path, default=here / "timing_results.csv")
    parser.add_argument(
        "--instances-dir",
        type=Path,
        default=project / "Instances-Full-vrpspdtw",
    )
    parser.add_argument("--png-dir", type=Path, default=here)
    parser.add_argument(
        "--pdf-dir",
        type=Path,
        default=project / "output" / "pdf",
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


def timing_prefix(variant, capacity, window_factor, uses_time_windows):
    prefix = f"hard_{variant}_q{capacity}"
    if uses_time_windows:
        prefix += f"_wf{window_factor}"
    return prefix


def clean_tick_format(value, _position):
    if value >= 1:
        return f"{int(value)}"
    return f"{value:.1f}"


def collect_speedups(rows):
    speedups = {}
    all_values = []
    for variant, _label, uses_time_windows in VARIANTS:
        variant_values = []
        for capacity, window_factor in SETTINGS:
            prefix = timing_prefix(
                variant, capacity, window_factor, uses_time_windows
            )
            bellman = np.array(
                [float(row[f"{prefix}_bellman_ms"]) for row in rows]
            )
            linear = np.array(
                [float(row[f"{prefix}_linear_ms"]) for row in rows]
            )
            if np.any(bellman <= 0) or np.any(linear <= 0):
                raise ValueError(f"Non-positive timing in {prefix}")
            values = bellman / linear
            variant_values.append(values)
            all_values.extend(values)
        speedups[variant] = variant_values
    return speedups, all_values


def make_figure(instance_sizes, variant_values, uses_time_windows, y_ticks):
    x_ticks = [10**2, 10**3, 10**4, 10**5]
    x_tick_labels = ["2", "3", "4", "5"]

    figure, axes = plt.subplots(1, 10, figsize=(12, 5), sharey=True)
    figure.subplots_adjust(wspace=0.15, bottom=0.25, top=0.80)

    for index, (axis, (capacity, window_factor), values) in enumerate(
        zip(axes, SETTINGS, variant_values)
    ):
        axis.set_xscale("log")
        axis.set_yscale("log")
        axis.scatter(
            instance_sizes,
            values,
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

        if index == 0:
            axis.set_ylabel("Speedup")
            axis.tick_params(labelleft=True)
        else:
            axis.tick_params(labelleft=False)
        axis.set_xlabel("")
        axis.text(
            0.5,
            1.20,
            f"{capacity}",
            transform=axis.transAxes,
            ha="center",
            va="center",
            clip_on=False,
        )
        if uses_time_windows:
            axis.text(
                0.5,
                1.09,
                f"{window_factor}",
                transform=axis.transAxes,
                ha="center",
                va="center",
                clip_on=False,
            )

    figure.text(0.13, 0.155, "n=10^", ha="right")
    axes[0].text(
        -0.08,
        1.20,
        "Q=",
        transform=axes[0].transAxes,
        ha="right",
        va="center",
        clip_on=False,
    )
    if uses_time_windows:
        axes[0].text(
            -0.08,
            1.09,
            "b_i*=",
            transform=axes[0].transAxes,
            ha="right",
            va="center",
            clip_on=False,
        )
    return figure


def main():
    options = arguments()
    dimensions = read_dimensions(options.instances_dir)
    with options.input.open(newline="", encoding="utf-8") as source:
        rows = list(csv.DictReader(source))
    if len(rows) != 105:
        raise ValueError(f"Expected 105 instances, found {len(rows)}")
    if len({row["instance"] for row in rows}) != 105:
        raise ValueError("Instance names are not unique")

    for row in rows:
        if row["instance"] not in dimensions:
            raise ValueError(f"No instance file found for {row['instance']}")
    rows.sort(key=lambda row: (dimensions[row["instance"]], row["instance"]))
    instance_sizes = np.array([dimensions[row["instance"]] for row in rows])

    speedups, all_values = collect_speedups(rows)
    y_min_power = math.floor(math.log2(min(all_values) / 1.05))
    y_max_power = math.ceil(math.log2(max(all_values) * 1.05))
    y_ticks = [2**power for power in range(y_min_power, y_max_power + 1)]

    plt.rcParams.update({
        "font.size": 12,
        "pdf.fonttype": 42,
        "ps.fonttype": 42,
    })
    options.png_dir.mkdir(parents=True, exist_ok=True)
    options.pdf_dir.mkdir(parents=True, exist_ok=True)

    for variant, label, uses_time_windows in VARIANTS:
        figure = make_figure(
            instance_sizes,
            speedups[variant],
            uses_time_windows,
            y_ticks,
        )
        png_path = options.png_dir / f"hard_{variant}_speedup.png"
        pdf_path = options.pdf_dir / f"hard_{variant}_speedup.pdf"
        figure.savefig(png_path, dpi=300)
        figure.savefig(
            pdf_path,
            metadata={
                "Title": f"Hard {label} Split speedup",
                "Subject": "Bellman runtime divided by linear Split runtime",
                "Keywords": f"{label}, Split, Bellman, linear, speedup",
                "Creator": "Matplotlib",
            },
        )
        plt.close(figure)
        print(f"{label} PNG: {png_path}")
        print(f"{label} PDF: {pdf_path}")

    print(
        f"Shared speedup range: {y_ticks[0]:g} to {y_ticks[-1]:g}; "
        f"observed range: {min(all_values):.6g} to {max(all_values):.6g}"
    )


if __name__ == "__main__":
    main()
