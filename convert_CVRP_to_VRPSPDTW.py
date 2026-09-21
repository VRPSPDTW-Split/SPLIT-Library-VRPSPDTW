import argparse
from pathlib import Path
import random


SERVICE_TIME = 40
DAY_ROUTE_CUSTOMERS = 20
RANDOM_SEED = 42


def parse_instance(file_path):
    with file_path.open("r", encoding="utf-8") as instance_file:
        lines = instance_file.readlines()

    if len(lines) < 6 or lines[4].strip() != "GIANT_TOUR_SECTION":
        raise ValueError("unexpected giant-tour instance header")

    header_lines = lines[:5]
    customers = []
    for line in lines[5:]:
        parts = line.strip().split()
        if not parts:
            continue
        if len(parts) != 4:
            raise ValueError(f"unexpected customer row: {line.rstrip()}")

        dnext = parts[3] if parts[3] == "EOF" else int(parts[3])
        customers.append(
            {
                "index": int(parts[0]),
                "demand": int(parts[1]),
                "dreturn": int(parts[2]),
                "dnext": dnext,
            }
        )

    dimension = int(header_lines[2].split(":", 1)[1])
    if len(customers) != dimension:
        raise ValueError(
            f"expected {dimension} customers but parsed {len(customers)}"
        )
    if customers[-1]["dnext"] != "EOF":
        raise ValueError("final customer row does not end with EOF")

    return header_lines, customers


def calculate_day_length(customers):
    """Implement the day-length construction described in Section 8.

    Time the route depot -> first 20 customers -> depot, including the
    constant service time at each customer. If necessary, raise the result so
    every depot -> customer -> depot singleton route is feasible.
    """
    sample_tour = customers[: min(DAY_ROUTE_CUSTOMERS, len(customers))]
    route_duration = (
        sample_tour[0]["dreturn"]
        + sum(customer["dnext"] for customer in sample_tour[:-1])
        + len(sample_tour) * SERVICE_TIME
        + sample_tour[-1]["dreturn"]
    )
    longest_singleton = max(
        2 * customer["dreturn"] + SERVICE_TIME for customer in customers
    )
    return max(route_duration, longest_singleton)


def assign_vrpspdtw_data(customers, day_length, rng):
    for customer in customers:
        travel_to_depot = customer["dreturn"]

        # Any opening time at or below this value permits service and a return
        # to the depot by the end of the day, even when the vehicle must wait.
        latest_open = day_length - travel_to_depot - SERVICE_TIME
        open_time = rng.randint(0, latest_open)

        # Keep the closing window at or after the earliest possible service
        # start, so the singleton route is feasible with respect to [a_i,b_i].
        earliest_service = max(travel_to_depot, open_time)
        close_time = rng.randint(earliest_service, day_length)

        pickup = rng.randint(0, customer["demand"])
        customer["pickup"] = pickup
        customer["delivery"] = customer["demand"] - pickup
        customer["tw_open"] = open_time
        customer["tw_close"] = close_time
        customer["service_time"] = SERVICE_TIME


def write_vrpspdtw_instance(output_path, header_lines, customers, day_length):
    with output_path.open("w", encoding="utf-8") as output_file:
        output_file.writelines(header_lines[:-1])
        output_file.write(f"DAY_LENGTH : {day_length}\n")
        output_file.write(header_lines[-1])
        for customer in customers:
            output_file.write(
                f"{customer['index']} {customer['delivery']} "
                f"{customer['pickup']} {customer['tw_open']} "
                f"{customer['tw_close']} {customer['service_time']} "
                f"{customer['dreturn']} {customer['dnext']}\n"
            )


def convert_directory(input_dir):
    output_dir = input_dir.with_name(input_dir.name + "-vrpspdtw")
    output_dir.mkdir(exist_ok=True)

    instance_paths = sorted(input_dir.glob("*.gt"))
    if not instance_paths:
        raise ValueError(f"no .gt instances found in {input_dir}")

    rng = random.Random(RANDOM_SEED)
    for file_path in instance_paths:
        header, customers = parse_instance(file_path)
        day_length = calculate_day_length(customers)
        assign_vrpspdtw_data(customers, day_length, rng)

        output_path = output_dir / f"{file_path.stem}_vrpspdtw{file_path.suffix}"
        write_vrpspdtw_instance(output_path, header, customers, day_length)

    return output_dir, len(instance_paths)


def main():
    parser = argparse.ArgumentParser(
        description="Batch convert CVRP giant tours to VRPSPDTW format."
    )
    parser.add_argument(
        "input_dir", help="Directory containing the CVRP .gt instance files."
    )
    args = parser.parse_args()

    output_dir, count = convert_directory(Path(args.input_dir))
    print(f"Converted {count} instances into {output_dir}")


if __name__ == "__main__":
    main()
