#!/usr/bin/env python3

import argparse
import csv
import math
import re
from pathlib import Path


SAMPLED_INSTANCES = (
    "wi29",
    "eil51",
    "rd100",
    "d198",
    "fl417",
    "pr1002",
    "mu1979",
    "fnl4461",
    "kz9976",
    "d18512",
    "bm33708",
    "ch71009",
)

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
    ("vrptw", "VRPTW", True),
    ("vrpspd", "VRPSPD", False),
    ("vrpspdtw", "VRPSPDTW", True),
)


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


def arguments():
    here = Path(__file__).resolve().parent
    project = here.parent
    parser = argparse.ArgumentParser(
        description="Generate arXiv-style LaTeX timing tables for the hard Splits."
    )
    parser.add_argument("--input", type=Path, default=here / "timing_results.csv")
    parser.add_argument(
        "--instances-dir",
        type=Path,
        default=project / "Instances-Full-vrpspdtw",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=here / "hard_timing_tables.tex",
    )
    return parser.parse_args()


def timing_prefix(variant, capacity, window_factor, uses_time_windows):
    prefix = f"hard_{variant}_q{capacity}"
    if uses_time_windows:
        prefix += f"_wf{window_factor}"
    return prefix


def format_number(value):
    if value < 0.1 or value >= 1000:
        exponent = math.floor(math.log10(value))
        mantissa = value / (10**exponent)
        mantissa_text = f"{mantissa:.2f}"
        return rf"{mantissa_text} \times 10^{{{exponent}}}"
    if value >= 100:
        return f"{value:.0f}"
    if value >= 10:
        return f"{value:.1f}"
    if value >= 1:
        return f"{value:.2f}"
    return f"{value:.3f}"


def timing_cell(value, winner):
    number = format_number(value)
    if winner:
        return rf"$\mathbf{{{number}}}$"
    return rf"${number}$"


def setting_header(capacity, window_factor, uses_time_windows):
    if uses_time_windows:
        return rf"$Q={capacity}$, $b_i^*={window_factor}$"
    return rf"$Q={capacity}$"


def table_block(rows, dimensions, variant, uses_time_windows, setting_slice):
    selected_settings = SETTINGS[setting_slice]
    start_column = 3
    cmidrules = []
    for setting_index in range(len(selected_settings)):
        left = start_column + 2 * setting_index
        cmidrules.append(rf"\cmidrule(lr){{{left}-{left + 1}}}")

    lines = [
        r"\resizebox{\textwidth}{!}{%",
        r"\begin{tabular}{@{}lr*{5}{rr}@{}}",
        r"\toprule",
    ]

    headers = ["Inst.", "$n$"]
    for capacity, window_factor in selected_settings:
        label = setting_header(capacity, window_factor, uses_time_windows)
        headers.append(rf"\multicolumn{{2}}{{c}}{{{label}}}")
    lines.append(" & ".join(headers) + r" \\")
    lines.append(" ".join(cmidrules))

    timing_headers = ["", ""]
    for _setting in selected_settings:
        timing_headers.extend(
            (r"$T_{\mathrm{Bellman}}$", r"$T_{\mathrm{Linear}}$")
        )
    lines.append(" & ".join(timing_headers) + r" \\")
    lines.append(r"\midrule")

    for instance in rows:
        row = [instance, str(dimensions[instance])]
        for capacity, window_factor in selected_settings:
            prefix = timing_prefix(
                variant, capacity, window_factor, uses_time_windows
            )
            bellman = float(rows[instance][f"{prefix}_bellman_ms"])
            linear = float(rows[instance][f"{prefix}_linear_ms"])
            row.append(timing_cell(bellman, bellman <= linear))
            row.append(timing_cell(linear, linear <= bellman))
        lines.append(" & ".join(row) + r" \\")

    lines.extend((r"\bottomrule", r"\end{tabular}%", r"}"))
    return lines


def make_table(rows, dimensions, variant, label, uses_time_windows):
    if uses_time_windows:
        setting_description = "vehicle capacity and closing-window multiplier"
    else:
        setting_description = "vehicle capacity"

    lines = [
        r"\begin{table*}[t]",
        r"\centering",
        r"\small",
        rf"\caption{{CPU times (ms) of the hard {label} Bellman-based and linear Splits on the fifth permutation of each sampled instance, with increasing {setting_description}. The lower block omits the four smallest instances, matching the sampled-instance layout of the arXiv table. Faster times are bolded.}}",
        rf"\label{{tab:hard-{variant}-timings}}",
    ]
    lines.extend(
        table_block(rows, dimensions, variant, uses_time_windows, slice(0, 5))
    )
    lines.append(r"\vspace{0.8em}")
    lower_rows = {name: rows[name] for name in SAMPLED_INSTANCES[4:]}
    lines.extend(
        table_block(
            lower_rows,
            dimensions,
            variant,
            uses_time_windows,
            slice(5, 10),
        )
    )
    lines.append(r"\end{table*}")
    return lines


def main():
    options = arguments()
    dimensions = read_dimensions(options.instances_dir)
    with options.input.open(newline="", encoding="utf-8") as source:
        source_rows = {
            row["instance"]: row
            for row in csv.DictReader(source)
        }

    missing_rows = [name for name in SAMPLED_INSTANCES if name not in source_rows]
    missing_dimensions = [
        name for name in SAMPLED_INSTANCES if name not in dimensions
    ]
    if missing_rows:
        raise ValueError(f"Missing timing rows: {', '.join(missing_rows)}")
    if missing_dimensions:
        raise ValueError(
            f"Missing instance dimensions: {', '.join(missing_dimensions)}"
        )

    sampled_rows = {name: source_rows[name] for name in SAMPLED_INSTANCES}
    output_lines = [
        "% Generated by Experiments/generate_hard_timing_tables.py.",
        "% Requires \\usepackage{booktabs} and \\usepackage{graphicx}.",
        "% Boldface is selected from the unrounded timing measurements.",
        r"\providecommand{\hardtimingtablebreak}{}",
        "",
    ]
    for index, (variant, label, uses_time_windows) in enumerate(VARIANTS):
        if index:
            output_lines.extend(("", r"\hardtimingtablebreak", "", ""))
        output_lines.extend(
            make_table(
                sampled_rows,
                dimensions,
                variant,
                label,
                uses_time_windows,
            )
        )

    options.output.parent.mkdir(parents=True, exist_ok=True)
    options.output.write_text("\n".join(output_lines) + "\n", encoding="utf-8")
    print(f"LaTeX: {options.output}")


if __name__ == "__main__":
    main()
