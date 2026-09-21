#!/usr/bin/env python3

import argparse
import csv
import os
import re
import subprocess
import sys
import tempfile
from concurrent.futures import FIRST_COMPLETED, ThreadPoolExecutor, wait
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PROGRAM_DIRECTORY = ROOT / "Program"
SPLIT_EXECUTABLE = PROGRAM_DIRECTORY / "split"
INSTANCE_DIRECTORY = ROOT / "Instances-Full-vrpspdtw"
DEFAULT_RESULTS = Path(__file__).resolve().parent / "timing_results.csv"

INITIAL_SECONDS = 1
FALLBACK_SECONDS = 8
PENALTY_LOAD = 1.0
PENALTY_WARP = 1.0

# The lock-step settings reported in Table 1 and Figure 1 of the original
# 105-instance experiment campaign.
HARD_SETTINGS = (
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


@dataclass(frozen=True)
class Experiment:
    column: str
    solver: str
    arguments: tuple


@dataclass(frozen=True)
class Timing:
    milliseconds: float
    runs: int
    seconds: int


def hard_experiments():
    variants = (
        ("hard_cvrp", "BELLMAN_CVRP", "LINEAR_CVRP", False),
        ("hard_vrpspd", "BELLMAN_VRPSPD", "LINEAR_VRPSPD", False),
        ("hard_vrptw", "BELLMAN_VRPTW", "LINEAR_VRPTW", True),
        ("hard_vrpspdtw", "BELLMAN_VRPSPDTW", "LINEAR_VRPSPDTW", True),
    )

    experiments = []
    for variant, bellman, linear, uses_time_windows in variants:
        for capacity, window_factor in HARD_SETTINGS:
            setting = f"q{capacity}"
            arguments = ("-capacity", str(capacity))
            if uses_time_windows:
                setting += f"_wf{window_factor}"
                arguments += ("-window_factor", str(window_factor))

            experiments.append(Experiment(
                f"{variant}_{setting}_bellman_ms", bellman, arguments))
            experiments.append(Experiment(
                f"{variant}_{setting}_linear_ms", linear, arguments))
    return experiments


def soft_experiments():
    return [
        Experiment(
            "soft_cvrp_alpha1_bellman_ms",
            "BELLMAN_SOFT_CVRP",
            ("-alpha", str(PENALTY_LOAD)),
        ),
        Experiment(
            "soft_cvrp_alpha1_linear_ms",
            "LINEAR_SOFT_CVRP",
            ("-alpha", str(PENALTY_LOAD)),
        ),
        Experiment(
            "soft_vrpspd_alpha1_bellman_ms",
            "BELLMAN_SOFT_VRPSPD",
            ("-alpha", str(PENALTY_LOAD)),
        ),
        Experiment(
            "soft_vrpspd_alpha1_linear_ms",
            "LINEAR_SOFT_VRPSPD",
            ("-alpha", str(PENALTY_LOAD)),
        ),
        Experiment(
            "soft_vrptw_alpha1_beta1_bellman_ms",
            "BELLMAN_SOFT_VRPTW",
            ("-alpha", str(PENALTY_LOAD), "-beta", str(PENALTY_WARP)),
        ),
        Experiment(
            "soft_vrptw_alpha1_beta1_linear_ms",
            "LINEAR_SOFT_VRPTW",
            ("-alpha", str(PENALTY_LOAD), "-beta", str(PENALTY_WARP)),
        ),
        Experiment(
            "soft_vrpspdtw_alpha1_beta1_bellman_ms",
            "BELLMAN_SOFT_VRPSPDTW",
            ("-alpha", str(PENALTY_LOAD), "-beta", str(PENALTY_WARP)),
        ),
        Experiment(
            "soft_vrpspdtw_alpha1_beta1_linear_ms",
            "LINEAR_SOFT_VRPSPDTW",
            ("-alpha", str(PENALTY_LOAD), "-beta", str(PENALTY_WARP)),
        ),
    ]


def all_experiments():
    return hard_experiments() + soft_experiments()


def instance_name(path):
    suffix = "_05_vrpspdtw.gt"
    if not path.name.endswith(suffix):
        raise ValueError(f"Unexpected fifth-permutation filename: {path.name}")
    return path.name[:-len(suffix)]


def instance_dimension(path):
    with path.open("r", encoding="utf-8") as instance_file:
        for line in instance_file:
            match = re.fullmatch(r"DIMENSION\s*:\s*(\d+)\s*", line)
            if match:
                return int(match.group(1))
            if line.startswith("GIANT_TOUR_SECTION"):
                break
    raise RuntimeError(f"No DIMENSION found in {path}")


def discover_instances(selected_names):
    paths = sorted(INSTANCE_DIRECTORY.glob("*_05_vrpspdtw.gt"))
    if len(paths) != 105:
        raise RuntimeError(
            f"Expected 105 fifth permutations in {INSTANCE_DIRECTORY}, found {len(paths)}")

    named_paths = [(instance_name(path), path) for path in paths]
    if len({name for name, _ in named_paths}) != 105:
        raise RuntimeError("The fifth-permutation instance names are not unique")

    if selected_names:
        available = {name for name, _ in named_paths}
        missing = sorted(set(selected_names) - available)
        if missing:
            raise RuntimeError("Unknown instance names: " + ", ".join(missing))
        named_paths = [pair for pair in named_paths if pair[0] in selected_names]
    return named_paths


def initialize_results(path, instances, columns):
    expected_header = ["instance"] + columns
    if not path.exists():
        return {
            name: {column: "" for column in expected_header}
            for name, _ in instances
        }, expected_header

    with path.open("r", newline="", encoding="utf-8") as input_file:
        reader = csv.DictReader(input_file)
        if reader.fieldnames != expected_header:
            raise RuntimeError(
                f"Existing CSV header does not match this campaign: {path}")
        rows = list(reader)

    results = {}
    for row in rows:
        name = row["instance"]
        if not name or name in results:
            raise RuntimeError(f"Invalid or duplicate instance row in {path}: {name!r}")
        results[name] = row

    expected_names = {name for name, _ in discover_instances(set())}
    if set(results) != expected_names:
        raise RuntimeError(
            f"Existing CSV does not contain exactly the 105 expected instance rows: {path}")
    return results, expected_header


def save_results(path, results, header, ordered_names):
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=str(path.parent))
    try:
        with os.fdopen(descriptor, "w", newline="", encoding="utf-8") as output_file:
            writer = csv.DictWriter(output_file, fieldnames=header)
            writer.writeheader()
            for name in ordered_names:
                row = dict(results[name])
                row["instance"] = name
                writer.writerow(row)
        os.replace(temporary_name, path)
    except BaseException:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise


def parse_timing(output, command):
    runs_match = re.search(r"Number of Runs\s*:\s*(\d+)", output)
    time_match = re.search(
        r"Average Millisecond Time \(Only Split\)\s*:\s*"
        r"([0-9]+(?:\.[0-9]*)?(?:[eE][+-]?\d+)?)",
        output,
    )
    if not runs_match or not time_match:
        raise RuntimeError(
            "Could not parse timing output from command:\n"
            + " ".join(command) + "\n\n" + output)
    return int(runs_match.group(1)), float(time_match.group(1))


def run_once(instance, experiment, seconds):
    command = [
        str(SPLIT_EXECUTABLE),
        str(instance),
        "-solver",
        experiment.solver,
        *experiment.arguments,
        "-seconds",
        str(seconds),
    ]
    completed = subprocess.run(
        command,
        cwd=PROGRAM_DIRECTORY,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            f"Timing command failed with exit code {completed.returncode}:\n"
            + " ".join(command) + "\n\n" + completed.stdout)
    runs, milliseconds = parse_timing(completed.stdout, command)
    return Timing(milliseconds, runs, seconds)


def measure(instance, experiment):
    initial = run_once(instance, experiment, INITIAL_SECONDS)
    if initial.runs >= 2:
        return initial

    fallback = run_once(instance, experiment, FALLBACK_SECONDS)
    if fallback.runs >= 2:
        return fallback

    # A Split taking longer than eight seconds can still produce only one run.
    # Repeat the eight-second measurement and combine the individual runs.
    second_fallback = run_once(instance, experiment, FALLBACK_SECONDS)
    total_runs = fallback.runs + second_fallback.runs
    if total_runs < 2:
        raise RuntimeError(
            f"Could not obtain two runs for {instance.name} with {experiment.solver}")
    milliseconds = (
        fallback.milliseconds * fallback.runs
        + second_fallback.milliseconds * second_fallback.runs
    ) / total_runs
    return Timing(milliseconds, total_runs, FALLBACK_SECONDS)


def validate_complete_results(path, header):
    with path.open("r", newline="", encoding="utf-8") as input_file:
        rows = list(csv.DictReader(input_file))
    if len(rows) != 105:
        raise RuntimeError(f"Expected 105 result rows, found {len(rows)}")

    missing = []
    invalid = []
    for row in rows:
        for column in header[1:]:
            value = row[column]
            if value == "":
                missing.append((row["instance"], column))
                continue
            try:
                numeric = float(value)
            except ValueError:
                invalid.append((row["instance"], column, value))
                continue
            if numeric <= 0.0:
                invalid.append((row["instance"], column, value))

    if missing or invalid:
        raise RuntimeError(
            f"CSV validation failed: {len(missing)} missing and "
            f"{len(invalid)} invalid measurements")


def parse_arguments():
    parser = argparse.ArgumentParser(
        description=(
            "Run the 105-instance fifth-permutation Split timing campaign. "
            "The CSV is saved after every measurement and an interrupted run resumes."
        ))
    parser.add_argument(
        "--results", type=Path, default=DEFAULT_RESULTS,
        help=f"output CSV path (default: {DEFAULT_RESULTS})")
    parser.add_argument(
        "--instance", action="append", default=[],
        help="run only the named instance; may be repeated")
    parser.add_argument(
        "--max-measurements", type=int,
        help="stop after this many new measurements (for smoke testing)")
    parser.add_argument(
        "--jobs", type=int, default=4,
        help="independent Split processes to run concurrently (default: 4)")
    parser.add_argument(
        "--verify-only", action="store_true",
        help="verify that the existing CSV is complete and numeric")
    return parser.parse_args()


def main():
    arguments = parse_arguments()
    if arguments.max_measurements is not None and arguments.max_measurements <= 0:
        raise RuntimeError("--max-measurements must be positive")
    if arguments.jobs <= 0:
        raise RuntimeError("--jobs must be positive")
    if not SPLIT_EXECUTABLE.is_file():
        raise RuntimeError(f"Build the timing executable first: {SPLIT_EXECUTABLE}")

    experiments = all_experiments()
    columns = [experiment.column for experiment in experiments]
    if len(columns) != 88 or len(set(columns)) != 88:
        raise RuntimeError("Expected 88 unique timing columns")

    selected_names = set(arguments.instance)
    all_instances = discover_instances(set())
    ordered_names = [
        name for name, instance in sorted(
            all_instances,
            key=lambda pair: (instance_dimension(pair[1]), pair[0]),
        )
    ]
    instances = [
        pair for pair in all_instances
        if not selected_names or pair[0] in selected_names
    ]
    if selected_names:
        available = {name for name, _ in all_instances}
        missing = sorted(selected_names - available)
        if missing:
            raise RuntimeError("Unknown instance names: " + ", ".join(missing))
    results, header = initialize_results(
        arguments.results, all_instances, columns)

    if not arguments.results.exists():
        save_results(arguments.results, results, header, ordered_names)

    if arguments.verify_only:
        validate_complete_results(arguments.results, header)
        print(f"PASS: {arguments.results} contains 105 complete rows and 88 timings per row")
        return 0

    total = len(instances) * len(experiments)
    completed_count = sum(
        1 for name, _ in instances for experiment in experiments
        if results[name][experiment.column] != "")
    new_measurements = 0
    print(
        f"Campaign: {len(instances)} instances, {len(experiments)} timings each; "
        f"{completed_count}/{total} already complete",
        flush=True,
    )

    measurements = (
        (name, instance, experiment)
        for name, instance in instances
        for experiment in experiments
        if results[name][experiment.column] == ""
    )
    executor = ThreadPoolExecutor(max_workers=arguments.jobs)
    pending = {}

    def submit_next():
        try:
            name, instance, experiment = next(measurements)
        except StopIteration:
            return False
        future = executor.submit(measure, instance, experiment)
        pending[future] = (name, experiment)
        return True

    try:
        for _ in range(arguments.jobs):
            if not submit_next():
                break

        stop = False
        while pending and not stop:
            completed, _ = wait(pending, return_when=FIRST_COMPLETED)
            for future in completed:
                name, experiment = pending.pop(future)
                timing = future.result()
                results[name][experiment.column] = format(timing.milliseconds, ".12g")
                save_results(arguments.results, results, header, ordered_names)

                completed_count += 1
                new_measurements += 1
                if completed_count % 10 == 0 or timing.seconds == FALLBACK_SECONDS:
                    print(
                        f"[{completed_count}/{total}] {name} {experiment.column}: "
                        f"{timing.milliseconds:.6g} ms ({timing.runs} runs, "
                        f"{timing.seconds}-second timing window)",
                        flush=True,
                    )

                if (arguments.max_measurements is not None
                        and new_measurements >= arguments.max_measurements):
                    stop = True
                    break

                submit_next()
    finally:
        for future in pending:
            future.cancel()
        executor.shutdown(wait=True, cancel_futures=True)

    if arguments.max_measurements is not None and new_measurements >= arguments.max_measurements:
        print("Stopped at --max-measurements; rerun to resume.", flush=True)
        return 0

    if not selected_names:
        validate_complete_results(arguments.results, header)
        print(
            f"PASS: {arguments.results} contains 105 complete rows and "
            f"88 timings per row",
            flush=True,
        )
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        sys.exit(1)
