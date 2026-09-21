# Standard VRP benchmark instances

Retrieved on 2026-09-20. The instance files are kept in the formats and with the
file names supplied by their source repositories; they have not been converted
to the library's giant-tour format.

The data is distributed as `VRPTW.zip`, `VRPSPD.zip`, and `VRPSPDTW.zip` in
this directory. Run `python3 extract_instances.py` from the repository root to
restore all benchmark and campaign instance directories. The extracted data
directories are ignored by Git.

`BenchmarkInstanceReader.h` and `BenchmarkInstanceReader.cpp` load all formats
in this directory directly. They parse the Solomon/Wang--Chen tabular format,
the coordinate-based Salhi--Nagy format, and the explicit full-distance-matrix
Dethloff and Montane--Galvao formats. For a requested problem variant, the
reader applies a customer permutation and returns a `Pb_Data` object compatible
with the existing Split solvers. Attributes not used by the requested variant
(pickups or time windows) are neutralized rather than rewritten on disk.

| Problem | Benchmark family | Directory | Instances |
|---|---|---|---:|
| VRPTW | Solomon | `VRPTW/Solomon/` | 56 |
| VRPTW | Gehring--Homberger | `VRPTW/Gehring-Homberger/` | 300 |
| VRPSPD | Salhi--Nagy, CMTX | `VRPSPD/Salhi-Nagy/CMTX/` | 14 |
| VRPSPD | Salhi--Nagy, CMTY | `VRPSPD/Salhi-Nagy/CMTY/` | 14 |
| VRPSPD | Dethloff, SCA/CON | `VRPSPD/Dethloff/` | 40 |
| VRPSPD | Montane--Galvao | `VRPSPD/Montane-Galvao/` | 18 |
| VRPSPDTW | Wang--Chen | `VRPSPDTW/Wang-Chen/` | 65 |
| | **Total** | | **507** |

## Sources and selection notes

### VRPTW

The Solomon and Gehring--Homberger files came from the CVRPLIB instance-set
archives. Only the `.txt` instance definitions were retained; the accompanying
`.sol` best-known-solution files in those archives were not copied.

- Solomon archive: <https://galgos.inf.puc-rio.br/cvrplib/index.php/en/download/instance-set/22>
  - downloaded SHA-256: `772d780da4d6856724edd31579beddf31ab7fa119008f30f7a2a7e8889c58015`
- Gehring--Homberger archive: <https://galgos.inf.puc-rio.br/cvrplib/index.php/en/download/instance-set/23>
  - downloaded SHA-256: `257db6fca7b2d89e97504ea51b5805f52d87fba52949a591214741be2fb18e0e`
- Benchmark descriptions and objective/distance conventions:
  - <https://www.sintef.no/projectweb/top/vrptw/solomon-benchmark/>
  - <https://www.sintef.no/projectweb/top/vrptw/homberger-benchmark/>

The Solomon directory contains the 56 standard 100-customer instances. The
Gehring--Homberger directory contains 60 instances at each of 200, 400, 600,
800, and 1,000 customers (300 total).

### VRPSPD

The three established VRPSPD families were copied from the `INSTANCES`
directory of the official FSPD repository:

<https://github.com/falcopt/FSPD/tree/main/INSTANCES>

Repository revision:
`de480ce4c59accc9f71c76c76f32b707ee7305ad`.

The Salhi--Nagy collection is the 28 CMT-derived instances split into its CMTX
and CMTY sets. The Dethloff collection is the 40 SCA/CON instances, and the
Montane--Galvao collection contains 18 instances with 100, 200, or 400
customers. Folder names use ASCII spelling, while the usual bibliographic
names are Salhi--Nagy and Montane--Galvao.

### VRPSPDTW

The Wang--Chen files were taken from the benchmark archive distributed with
VRPenstein/MATE:

<https://github.com/senshineL/VRPenstein>

Repository revision:
`c912c57aa387d31c20c036f11d0bca3c69ce93da`.

The archive contains 68 Wang--Chen `.txt` files, including three additional
5-customer toy files (`rcdp0501`, `rcdp0504`, and `rcdp0507`). The standard
published benchmark has 65 instances: nine small instances (three each with
10, 25, and 50 customers) and 56 instances with 100 customers. Therefore the
three 5-customer files and the archive's generated `explicit_*.vrpsdptw`
duplicates are intentionally not included here.

The MATE paper describing the 65-instance benchmark is available at
<https://arxiv.org/abs/2011.06331>.
