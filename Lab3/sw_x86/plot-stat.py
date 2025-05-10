import matplotlib.pyplot as plt
import sys
import re
import os

def parse_perf_stat(filename, target_stat):
    with open(filename, 'r') as file:
        lines = file.readlines()

    values = []
    for line in lines:
        if not line.strip():
            continue

        match = re.match(r"\s*([\d,]+)\s+(\S+)", line)
        if match:
            value_str, stat_name = match.groups()
            if stat_name == target_stat:
                value = int(value_str.replace(',', ''))
                values.append(value)

    return values

def parse_exec_times(output_file):
    with open(output_file, 'r') as f:
        lines = f.readlines()

    pattern = r"execution time.*?([\d.]+)\s*sec"
    times = []

    for line in lines:
        match = re.search(pattern, line)
        if match:
            times.append(float(match.group(1)))

    return times

def plot_stat(values, stat_name, output_filename):
    plt.figure(figsize=(10, 5))
    plt.plot(range(1, len(values) + 1), values, marker='o')
    plt.title(f'Statistic: {stat_name}')
    plt.xlabel('Run Number')
    plt.ylabel(stat_name)
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(output_filename)
    plt.close()
    print(f"Saved plot to {output_filename}")

def main():
    if len(sys.argv) != 3:
        print("Usage: python plot-stat.py <perf_stat_file|output_file> <stat_name>")
        sys.exit(1)

    input_file = sys.argv[1]
    stat_name = sys.argv[2]

    # Handle special case for execution time
    if stat_name == "exec-time":
        values = parse_exec_times(input_file)
    else:
        values = parse_perf_stat(input_file, stat_name)

    if not values:
        print(f"No data found for '{stat_name}' in file '{input_file}'.")
        sys.exit(1)

    os.makedirs("plots", exist_ok=True)
    base_name = os.path.splitext(os.path.basename(input_file))[0]
    output_path = os.path.join("plots", f"{base_name}_{stat_name}.png")

    plot_stat(values, stat_name, output_path)

if __name__ == '__main__':
    main()
