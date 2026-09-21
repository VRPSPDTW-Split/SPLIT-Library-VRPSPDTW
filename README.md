# Split Algorithm Extensions for Vehicle Routing Problems

This repository extends Thibaut Vidal's Split Algorithm Library with hard- and soft-constraint Split algorithms for the CVRP, VRPSPD, VRPTW, and VRPSPDTW.

## Summary

The repository contains Bellman-based reference algorithms and linear-time algorithms for four vehicle-routing variants:

- CVRP with hard or penalized capacity constraints
- VRPSPD with hard or penalized simultaneous pickup-and-delivery load constraints
- VRPTW with hard time windows or capacity-excess and time-warp penalties
- VRPSPDTW with hard combined constraints or load-excess and time-warp penalties

For a problem with `n` customers and an average of `B` feasible route extensions from each predecessor, the included command-line solvers are:

| Solver type | Split variant | Complexity |
|---|---|---:|
| `BELLMAN_CVRP` | CVRP with hard capacity constraints | `O(nB)` |
| `BELLMAN_VRPSPD` | VRPSPD with hard simultaneous pickup-and-delivery constraints | `O(nB)` |
| `BELLMAN_VRPTW` | VRPTW with hard capacity and time-window constraints | `O(nB)` |
| `BELLMAN_VRPSPDTW` | VRPSPDTW with hard load and time-window constraints | `O(nB)` |
| `BELLMAN_SOFT_CVRP` | CVRP with a capacity-excess penalty | `O(n^2)` |
| `BELLMAN_SOFT_VRPSPD` | VRPSPD with a load-excess penalty | `O(n^2)` |
| `BELLMAN_SOFT_VRPTW` | VRPTW with capacity-excess and time-warp penalties | `O(n^2)` |
| `BELLMAN_SOFT_VRPSPDTW` | VRPSPDTW with load-excess and time-warp penalties | `O(n^2)` |
| `LINEAR_CVRP` | CVRP with hard capacity constraints | `O(n)` |
| `LINEAR_VIDAL` | Original Vidal CVRP Split with hard capacity constraints | `O(n)` |
| `LINEAR_VRPSPD` | VRPSPD with hard simultaneous pickup-and-delivery constraints | `O(n)` |
| `LINEAR_VRPTW` | VRPTW with hard capacity and time-window constraints | `O(n)` |
| `LINEAR_VRPSPDTW` | VRPSPDTW with hard load and time-window constraints | `O(n)` |
| `LINEAR_SOFT_CVRP` | CVRP with a capacity-excess penalty | `O(n)` |
| `LINEAR_SOFT_VRPSPD` | VRPSPD with a load-excess penalty | `O(n)` |
| `LINEAR_SOFT_VRPTW` | VRPTW with capacity-excess and time-warp penalties | `O(n)` |
| `LINEAR_SOFT_VRPSPDTW` | VRPSPDTW with load-excess and time-warp penalties | `O(n)` |

The hard Bellman solver names use the common VRPSPDTW Bellman implementation after neutralizing constraints that do not belong to the selected variant.

## Attribution and license

- Original author: Thibaut Vidal
- Original date: August 15, 2015
- Original project: [Vidal's VRP resources](https://w.cba.ulaval.ca/vidalt/)
- Additional Split algorithms: *anonymized for review*
- Correspondence: *anonymized for review*
- Extension date: September 2026
- License: [GNU General Public License v3](LICENSE)

The soft CVRP solver pair was imported from [Vidal's official GPL-3.0 Split-Library](https://github.com/vidalt/Split-Library), revision `0005ac156a0d632cb214c5504ddd1e409bd340c9`.

## Installation and usage

On Unix-like systems, build both executables from the `Program` directory:

```sh
cd Program
make
```

On Windows, run `./build.ps1` from the `Program` directory instead.

Run the timing executable with:

```sh
./split INSTANCE_PATH
```

The default solver is `LINEAR_VRPSPDTW`. Select another solver with:

```sh
./split INSTANCE_PATH -solver SOLVER_TYPE
```

`SOLVER_TYPE` may be any solver listed in the table above. Soft solvers require `-alpha` for the capacity or load-excess penalty; soft time-window variants also require `-beta` for the time-warp penalty.

For example:

```sh
./split INSTANCE_PATH -solver LINEAR_SOFT_CVRP -alpha V -seconds Z
```

This runs Vidal's linear soft-capacity CVRP Split with a penalty of `V` per unit of capacity excess.

```sh
./split INSTANCE_PATH -solver LINEAR_SOFT_VRPTW -alpha V -beta W -capacity X -window_factor Y -seconds Z
```

This runs the linear soft VRPTW Split with capacity-excess penalty `V` and time-warp penalty `W`. It sets the vehicle capacity to `X`, multiplies customer and depot closing times by `Y`, and reports the average runtime over a timing window of at least `Z` seconds.

The same options can be used with `BELLMAN_SOFT_VRPSPDTW` or `LINEAR_SOFT_VRPSPDTW` to include simultaneous pickups and deliveries.

The timing output includes `Number of Runs`, the number of complete Split executions used to calculate the reported average.

## Timing campaign

`Experiments/run_timing_campaign.py` reproduces the 105-instance campaign using the fifth permutation of every
instance. It times hard and soft Bellman and linear CVRP, VRPSPD, VRPTW, and VRPSPDTW. The hard cases use the ten
lock-step capacity/time-window settings reported in the original manuscript. The soft cases use `alpha = beta = 1`
with each instance's original capacity and time windows.

Each measurement begins with a one-second timing window. If this completes only one Split execution, the script
repeats the measurement with an eight-second window. If necessary, it performs a second eight-second invocation so
the reported average always contains at least two completed Split executions. Results are saved after every
measurement and an interrupted campaign resumes from the existing CSV. Four independent Split processes run by
default to shorten wall-clock time; use `--jobs 1` for a fully sequential campaign.

Run from the project directory with:

```sh
python3 Experiments/run_timing_campaign.py
```

The output is written to `Experiments/timing_results.csv`. Use `--verify-only` to validate that the CSV contains
all 105 rows and all 88 positive numeric timing values per row. Rows are ordered by the `DIMENSION` field in each
instance file, from the smallest instance to the largest.

To recreate the manuscript-style hard VRPSPDTW speedup figure from that CSV, run:

```sh
python3 Experiments/plot_hard_vrpspdtw.py
```

This writes `Experiments/hard_vrpspdtw_speedup.png`. Each panel reports Bellman time divided by linear time for one
capacity/closing-window-factor pair, with logarithmic axes matching the original composite-plot convention.

To create matching hard VRPSPD, VRPTW, and VRPSPDTW figures with a shared speedup scale, run:

```sh
python3 Experiments/plot_hard_speedups.py
```

The PNG previews are written to `Experiments/`. Each figure is also written to its own vector PDF in `output/pdf/`:
`hard_vrpspd_speedup.pdf`, `hard_vrptw_speedup.pdf`, and `hard_vrpspdtw_speedup.pdf`.

To create the matching three-panel soft comparison figure, run:

```sh
python3 Experiments/plot_soft_speedups.py
```

The panels show Soft VRPTW, Soft VRPSPD, and Soft VRPSPDTW in that order, using `alpha = beta = 1` where applicable.
The PNG preview is written to `Experiments/soft_split_speedups.png`, and the vector version is written to
`output/pdf/soft_split_speedups.pdf`.

To generate the three arXiv-style hard timing tables for VRPTW, VRPSPD, and VRPSPDTW, run:

```sh
python3 Experiments/generate_hard_timing_tables.py
```

This writes `Experiments/hard_timing_tables.tex` using the twelve sampled instances from the arXiv table. Every
Bellman/linear pair is populated from `Experiments/timing_results.csv`, and the faster unrounded measurement is
bolded.

To compare the linear CVRP, VRPTW, VRPSPD, and VRPSPDTW running times directly, run:

```sh
python3 Experiments/plot_linear_relative_times.py
```

This writes separate hard and soft PNG previews to `Experiments/` and separate vector PDFs to
`output/pdf/hard_linear_relative_times.pdf` and `output/pdf/soft_linear_relative_times.pdf`. The horizontal axis
is logarithmic because the instance sizes span more than three orders of magnitude. Each hard point is the
arithmetic average of the ten ratios obtained by normalizing the variant timing against the hard linear CVRP timing
at the same capacity. The soft figure uses `alpha = beta = 1` where applicable and normalizes each observation by the soft
linear CVRP timing. All points are fully opaque, and the trend lines use a nine-instance moving average.

## Correctness checker

The normal `split` executable and `main.cpp` remain dedicated to timing. Building the project also creates a separate
`split_correctness` executable from `main_correctness.cpp`. It reads the native benchmark formats in
`Benchmark-Instances/`, generates reproducible random giant-tour permutations, and compares the selected linear
algorithm with the existing quadratic Bellman implementation.

Usage:

```sh
./split_correctness INSTANCE_PATH --variant VARIANT --num_permutations N [--alpha V] [--beta W]
```

The instance may instead be supplied as `--instance INSTANCE_PATH`. Supported variants are `CVRP`, `VRPSPD`, `VRPTW`,
`VRPSPDTW`, `Soft_CVRP`, `Soft_VRPSPD`, `Soft_VRPTW`, and `Soft_VRPSPDTW`. `Soft_CVRP` and `Soft_VRPSPD` require the capacity-excess penalty
`--alpha`. `Soft_VRPTW` and `Soft_VRPSPDTW` require both `--alpha` and the time-warp penalty `--beta`. The single-dash spellings
`-alpha` and `-beta` are also accepted to match the timing executable.

Examples:

```sh
./split_correctness ../Benchmark-Instances/VRPSPD/Salhi-Nagy/CMTX/CMT1H.vrp --variant VRPSPD --num_permutations 1000

./split_correctness ../Benchmark-Instances/VRPTW/Solomon/C101.txt --variant Soft_VRPTW --num_permutations 1000 --alpha 3 --beta 5
```

Permutations use a fixed random seed of 42 so failures are repeatable. Costs are compared with an absolute tolerance
of 1e-4 (plus a small relative floating-point allowance), matching the epsilon in Vidal's original soft-CVRP solver.
On a mismatch, the checker reports both costs, route counts, and the complete customer permutation, then exits with
status `1`. A successful run exits with status `0`.

## Benchmark instances

The instance data is distributed in five compressed archives so the repository remains compact. After cloning the
repository, extract every collection from the repository root with:

```sh
python3 extract_instances.py
```

This restores `Instances-Full/`, `Instances-Full-vrpspdtw/`, and the three problem-family directories under
`Benchmark-Instances/`. The extracted directories are ignored by Git; the ZIP archives are the repository copies.
The timing and correctness commands below expect the archives to have been extracted first.

The original benchmarks compiled by T. Vidal were extended into VRPSPDTW instances as described in the associated paper. 
The campaign directories retain the fifth permutation of each of the 105 CVRP giant-tour instances from Vidal's
2016 Split paper. The complete 1,050-file collection can be restored from the official archive at
[Vidal's official archive](https://w1.cirrelt.ca/~vidalt/resources/SPLIT-Instances.zip)
(SHA-256: `1173bb9cf9bc86a981df46821480c97a283fe64d4566fa322ec4cfb341e24592`).

The Python file `convert_CVRP_to_VRPSPDTW.py` can be run on a restored `Instances-Full/` collection to recreate the
corresponding `Instances-Full-vrpspdtw/` directory. The conversion follows Section 8 of the 2026 Split paper: each demand
is randomly divided into delivery and pickup quantities, service times are fixed at 40, travel times retain
the CVRP travel costs, and the day length is the larger of (a) the duration of the route through the first
20 customers and (b) the longest singleton-route duration. Time windows are then generated so every
singleton route is feasible. The random seed is fixed at 42 and files are processed in sorted order so the
generated instances are reproducible.

The standard literature benchmark collections for VRPTW (Solomon and Gehring--Homberger), VRPSPD
(Salhi--Nagy, Dethloff, and Montane--Galvao), and VRPSPDTW (Wang--Chen) are distributed as archives in
`Benchmark-Instances/`. See `Benchmark-Instances/README.md` for counts, sources, revisions, and format notes.
