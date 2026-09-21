#!/usr/bin/env python3

from pathlib import Path
from zipfile import ZipFile


ROOT = Path(__file__).resolve().parent
ARCHIVES = (
    (ROOT / "Instances-Full.zip", ROOT),
    (ROOT / "Instances-Full-vrpspdtw.zip", ROOT),
    (ROOT / "Benchmark-Instances" / "VRPTW.zip", ROOT / "Benchmark-Instances"),
    (ROOT / "Benchmark-Instances" / "VRPSPD.zip", ROOT / "Benchmark-Instances"),
    (ROOT / "Benchmark-Instances" / "VRPSPDTW.zip", ROOT / "Benchmark-Instances"),
)


def extract(archive_path, destination):
    if not archive_path.is_file():
        raise FileNotFoundError(f"Missing instance archive: {archive_path}")

    with ZipFile(archive_path) as archive:
        for member in archive.infolist():
            member_path = Path(member.filename)
            if member_path.is_absolute() or ".." in member_path.parts:
                raise ValueError(
                    f"Unsafe path in {archive_path.name}: {member.filename}"
                )
        archive.extractall(destination)

    print(f"Extracted {archive_path.relative_to(ROOT)}")


def main():
    for archive_path, destination in ARCHIVES:
        extract(archive_path, destination)


if __name__ == "__main__":
    main()
