#!/usr/bin/env python3

import argparse
import csv
import math
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import FuncFormatter

from plot_hard_speedups import clean_tick_format, read_dimensions


VARIANTS = (
    ("VRPTW", "soft_vrptw_alpha1_beta1"),
    ("VRPSPD", "soft_vrpspd_alpha1"),
    ("VRPSPDTW", "soft_vrpspdtw_alpha1_beta1"),
)


def arguments():
    here = Path(__file__).resolve().parent
    project = here.parent
    parser = argparse.ArgumentParser(
        description="Plot the three soft Split speedup comparisons."
    )
    parser.add_argument("--input", type=Path, default=here / "timing_results.csv")
    parser.add_argument(
        "--instances-dir",
        type=Path,
        default=project / "Instances-Full-vrpspdtw",
    )
    parser.add_argument(
        "--output-png",
        type=Path,
        default=here / "soft_split_speedups.png",
    )
    parser.add_argument(
        "--output-pdf",
        type=Path,
        default=project / "output" / "pdf" / "soft_split_speedups.pdf",
    )
    return parser.parse_args()


def collect_speedups(rows):
    speedups = []
    all_values = []
    for _label, prefix in VARIANTS:
        bellman = np.array(
            [float(row[f"{prefix}_bellman_ms"]) for row in rows]
        )
        linear = np.array(
            [float(row[f"{prefix}_linear_ms"]) for row in rows]
        )
        if np.any(bellman <= 0) or np.any(linear <= 0):
            raise ValueError(f"Non-positive timing in {prefix}")
        values = bellman / linear
        speedups.append(values)
        all_values.extend(values)
    return speedups, all_values


def make_figure(instance_sizes, speedups, y_ticks):
    x_ticks = [10**2, 10**3, 10**4, 10**5]
    x_tick_labels = ["2", "3", "4", "5"]

    figure, axes = plt.subplots(1, 3, figsize=(12, 5), sharey=True)
    figure.subplots_adjust(wspace=0.12, bottom=0.25, top=0.82)

    for index, (axis, (label, _prefix), values) in enumerate(
        zip(axes, VARIANTS, speedups)
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

        axis.text(
            0.5,
            1.10,
            f"Soft {label}",
            transform=axis.transAxes,
            ha="center",
            va="center",
            clip_on=False,
        )

        if index == 0:
            axis.set_ylabel("Speedup")
            axis.tick_params(labelleft=True)
        else:
            axis.tick_params(labelleft=False)
        axis.set_xlabel("")

    figure.text(0.13, 0.155, "n=10^", ha="right")
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
    options.output_png.parent.mkdir(parents=True, exist_ok=True)
    options.output_pdf.parent.mkdir(parents=True, exist_ok=True)

    figure = make_figure(instance_sizes, speedups, y_ticks)
    figure.savefig(
        options.output_pdf,
        metadata={
            "Title": "Soft Split speedup comparisons",
            "Subject": "Bellman runtime divided by linear Split runtime",
            "Keywords": "VRPTW, VRPSPD, VRPSPDTW, soft Split, speedup",
            "Creator": "Matplotlib",
        },
    )
    figure.savefig(options.output_png, dpi=300)
    plt.close(figure)

    print(f"PNG: {options.output_png}")
    print(f"PDF: {options.output_pdf}")
    print(
        f"Shared speedup range: {y_ticks[0]:g} to {y_ticks[-1]:g}; "
        f"observed range: {min(all_values):.6g} to {max(all_values):.6g}"
    )


if __name__ == "__main__":
    main()
