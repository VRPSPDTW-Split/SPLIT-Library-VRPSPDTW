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
from matplotlib.ticker import MaxNLocator


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
    ("vrptw", "VRPTW", "#D55E00", "^", True),
    ("vrpspd", "VRPSPD", "#0072B2", "o", False),
    ("vrpspdtw", "VRPSPDTW", "#7E57C2", "s", True),
)


def arguments():
    here = Path(__file__).resolve().parent
    project = here.parent
    parser = argparse.ArgumentParser(
        description=(
            "Plot the linear hard and soft Split running times relative to "
            "their corresponding linear CVRP Split."
        )
    )
    parser.add_argument("--input", type=Path, default=here / "timing_results.csv")
    parser.add_argument(
        "--instances-dir",
        type=Path,
        default=project / "Instances-Full-vrpspdtw",
    )
    parser.add_argument("--png-dir", type=Path, default=here)
    parser.add_argument(
        "--hard-output-pdf",
        type=Path,
        default=project / "output" / "pdf" / "hard_linear_relative_times.pdf",
    )
    parser.add_argument(
        "--soft-output-pdf",
        type=Path,
        default=project / "output" / "pdf" / "soft_linear_relative_times.pdf",
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


def moving_average(values, window=9):
    radius = window // 2
    return np.array([
        np.mean(values[max(0, index - radius):min(len(values), index + radius + 1)])
        for index in range(len(values))
    ])


def hard_column(variant, capacity, window_factor, uses_time_windows):
    name = f"hard_{variant}_q{capacity}"
    if uses_time_windows:
        name += f"_wf{window_factor}"
    return name + "_linear_ms"


def collect_hard_series(rows, instance_sizes):
    series = {}
    for variant, label, color, marker, uses_time_windows in VARIANTS:
        average_ratios = []
        for row in rows:
            ratios = []
            for capacity, window_factor in SETTINGS:
                baseline = float(row[f"hard_cvrp_q{capacity}_linear_ms"])
                value = float(row[
                    hard_column(
                        variant,
                        capacity,
                        window_factor,
                        uses_time_windows,
                    )
                ])
                if baseline <= 0 or value <= 0:
                    raise ValueError(
                        f"Non-positive hard timing for {row['instance']}"
                    )
                ratios.append(value / baseline)
            average_ratios.append(np.mean(ratios))
        series[variant] = {
            "label": label,
            "color": color,
            "marker": marker,
            "point_sizes": instance_sizes,
            "point_ratios": np.array(average_ratios),
        }
    return series


def collect_soft_series(rows, instance_sizes):
    columns = {
        "vrptw": "soft_vrptw_alpha1_beta1_linear_ms",
        "vrpspd": "soft_vrpspd_alpha1_linear_ms",
        "vrpspdtw": "soft_vrpspdtw_alpha1_beta1_linear_ms",
    }
    series = {}
    for variant, label, color, marker, _uses_time_windows in VARIANTS:
        ratios = []
        for row in rows:
            baseline = float(row["soft_cvrp_alpha1_linear_ms"])
            value = float(row[columns[variant]])
            if baseline <= 0 or value <= 0:
                raise ValueError(
                    f"Non-positive soft timing for {row['instance']}"
                )
            ratios.append(value / baseline)
        series[variant] = {
            "label": label,
            "color": color,
            "marker": marker,
            "point_sizes": instance_sizes,
            "point_ratios": np.array(ratios),
        }
    return series


def make_figure(instance_sizes, series, constraint_type):
    figure, axis = plt.subplots(figsize=(12.5, 7.0))
    axis.set_facecolor("#F0F0F0")
    axis.set_xscale("log")

    axis.axhline(
        1.0,
        color="#4C4C4C",
        linewidth=1.4,
        linestyle=":",
        label="CVRP (1x baseline)",
        zorder=2,
    )

    all_ratios = [1.0]
    for variant, _label, _color, _marker, _uses_time_windows in VARIANTS:
        values = series[variant]
        all_ratios.extend(values["point_ratios"])
        axis.scatter(
            values["point_sizes"],
            values["point_ratios"],
            s=23,
            marker=values["marker"],
            facecolors=values["color"],
            edgecolors=values["color"],
            linewidths=0.45,
            alpha=1.0,
            zorder=3,
        )
        axis.plot(
            instance_sizes,
            moving_average(values["point_ratios"]),
            color=values["color"],
            linewidth=2.0,
            marker=values["marker"],
            markersize=4.2,
            markevery=12,
            label=values["label"],
            zorder=4,
        )

    maximum_ratio = max(all_ratios)
    axis.set_ylim(0.72, math.ceil(maximum_ratio * 1.06))
    axis.yaxis.set_major_locator(MaxNLocator(nbins=8, integer=True))
    axis.set_xlim(instance_sizes.min() / 1.18, instance_sizes.max() * 1.18)
    axis.set_xticks([100, 1000, 10000, 100000])
    axis.set_xticklabels([r"$10^2$", r"$10^3$", r"$10^4$", r"$10^5$"])

    axis.grid(True, which="major", color="white", linestyle="--", linewidth=0.8)
    axis.grid(True, which="minor", axis="x", color="white", linewidth=0.45)
    axis.set_axisbelow(True)
    axis.set_xlabel(r"Number of customers, $n$")
    axis.set_ylabel(
        f"Times slower than the linear {constraint_type} CVRP Split"
    )
    axis.legend(loc="upper left", ncol=4, frameon=False)

    figure.tight_layout()
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
    hard_series = collect_hard_series(rows, instance_sizes)
    soft_series = collect_soft_series(rows, instance_sizes)

    plt.rcParams.update({
        "font.family": "serif",
        "font.size": 11.5,
        "pdf.fonttype": 42,
        "ps.fonttype": 42,
    })
    options.png_dir.mkdir(parents=True, exist_ok=True)
    options.hard_output_pdf.parent.mkdir(parents=True, exist_ok=True)
    options.soft_output_pdf.parent.mkdir(parents=True, exist_ok=True)

    hard_figure = make_figure(instance_sizes, hard_series, "hard")
    soft_figure = make_figure(instance_sizes, soft_series, "soft")
    hard_png = options.png_dir / "hard_linear_relative_times.png"
    soft_png = options.png_dir / "soft_linear_relative_times.png"

    hard_figure.savefig(
        options.hard_output_pdf,
        metadata={
            "Title": "Hard linear Split running times relative to CVRP",
            "Subject": "Hard linear Split timing comparison",
            "Keywords": "CVRP, VRPTW, VRPSPD, VRPSPDTW, Split, runtime",
            "Creator": "Matplotlib",
        },
    )
    soft_figure.savefig(
        options.soft_output_pdf,
        metadata={
            "Title": "Soft linear Split running times relative to CVRP",
            "Subject": "Soft linear Split timing comparison",
            "Keywords": "CVRP, VRPTW, VRPSPD, VRPSPDTW, Split, runtime",
            "Creator": "Matplotlib",
        },
    )

    hard_figure.savefig(hard_png, dpi=300)
    soft_figure.savefig(soft_png, dpi=300)
    plt.close(hard_figure)
    plt.close(soft_figure)

    print(f"Hard preview: {hard_png}")
    print(f"Soft preview: {soft_png}")
    print(f"Hard vector PDF: {options.hard_output_pdf}")
    print(f"Soft vector PDF: {options.soft_output_pdf}")


if __name__ == "__main__":
    main()
