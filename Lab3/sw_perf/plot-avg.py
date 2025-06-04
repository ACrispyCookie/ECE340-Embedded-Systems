import os
import sys
import re
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import Patch

def parse_exec_time_median_and_cv(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()

    times = []
    pattern = r"execution time.*?([\d.]+)\s*sec"

    for line in lines:
        match = re.search(pattern, line)
        if match:
            times.append(float(match.group(1)))

    if times:
        mean = np.mean(times)
        std_dev = np.std(times)
        median = np.median(times)
        cv = (std_dev / mean) * 100  # Coefficient of variation in percentage
        return median, mean, std_dev, cv
    else:
        return None, None, None, None

def parse_perf_stat_median_and_cv(filename, target_stat):
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
                try:
                    value = int(value_str.replace(',', ''))
                    values.append(value)
                except ValueError:
                    continue

    if values:
        mean = np.mean(values)
        std_dev = np.std(values)
        median = np.median(values)
        cv = (std_dev / mean) * 100  # Coefficient of variation in percentage
        return median, mean, std_dev, cv
    else:
        return None, None, None, None

def plot_medians(file_median_map, stat_name, output_path, input_desc):
    labels = list(file_median_map.keys())
    medians = [item[0] for item in file_median_map.values()]
    cvs = [item[3] for item in file_median_map.values()]

    # Determine the unit and convert the medians accordingly
    if stat_name == "exec-time":
        if all(v < 1 for v in medians):  # All times are < 1 second
            unit = "µs"
            medians = [v * 1e6 for v in medians]  # Convert to microseconds
        elif all(v < 1000 for v in medians):  # All times are < 1 second
            unit = "ms"
            medians = [v * 1e3 for v in medians]  # Convert to milliseconds
        else:
            unit = "s"  # Seconds as the unit
        ylabel = f"Execution Time ({unit})"
    else:
        unit = stat_name  # For other stats, use the name as the unit
        ylabel = f"{stat_name} ({unit})"

    plt.figure(figsize=(12, 6))
    bars = plt.bar(labels, medians, color='skyblue', width=0.6)

    plt.title(f'Median {ylabel} per version ({input_desc})', fontsize=16)
    plt.ylabel(ylabel, fontsize=14)
    plt.xlabel('Version', fontsize=14)
    plt.xticks(rotation=45, ha='right', fontsize=12)

    # Add value labels with CVs
    for bar, median, cv in zip(bars, medians, cvs):
        height = bar.get_height()
        label = f"{median:.2f}" if stat_name != "exec-time" else f"{median:.2f} {unit}"
        plt.text(bar.get_x() + bar.get_width()/2, height, f"{label}\nCV: {cv:.2f}%",
                 ha='center', va='bottom', fontsize=12)

    # Speedup box
    sorted_items = sorted(file_median_map.items(), key=lambda x: x[1][0])
    best_name, best_vals = sorted_items[0]
    worst_vals = sorted_items[-1][1][0]
    if best_vals[0] > 0 and worst_vals > 0 and best_vals[0] != worst_vals:
        speedup = worst_vals / best_vals[0]
        info = f"Fastest: {best_name}\nSpeedup: {speedup:.2f}×"
    else:
        info = f"Fastest: {best_name}"

    ax = plt.gca()
    ax.text(0.98, 0.98, info, ha='right', va='top',
            transform=ax.transAxes,
            bbox=dict(boxstyle="round,pad=0.3", facecolor="lightyellow", edgecolor="gray"),
            fontsize=10)

    # Adjust y-axis limit to give more space for text
    plt.ylim(bottom=0, top=max(medians) * 1.2)  # Increase the y-limit by 20% to add space

    # Adjust layout to prevent overlap
    plt.tight_layout()

    plt.savefig(output_path)
    plt.close()
    print(f"✅ Saved plot to {output_path}")


def report_speedups(file_median_map, stat_name):
    if stat_name == "exec-time":
        # Lower is better: fastest first
        sorted_items = sorted(file_median_map.items(), key=lambda x: x[1][0])
        fastest_name, fastest_vals = sorted_items[0]
    else:
        # Higher is better: best first
        sorted_items = sorted(file_median_map.items(), key=lambda x: x[1][0], reverse=True)
        fastest_name, fastest_vals = sorted_items[0]

    print(f"\n📊 Speedup report ({'lower is better' if stat_name == 'exec-time' else 'higher is better'} for '{stat_name}'): ")

    for name, vals in sorted_items:
        median, mean, std_dev, cv = vals
        if stat_name == "exec-time":
            # Convert time to readable units
            unit = "ms" if median >= 1 else "µs"
            median_converted = median * 1e3 if unit == "ms" else median * 1e6
            if median == fastest_vals[0]:
                print(f"  - {name}: baseline ({median_converted:.4f} {unit})")
            else:
                speedup = median / fastest_vals[0]
                print(f"  - {name}: {median_converted:.4f} {unit}  ({speedup:.2f}× slower than {fastest_name})")
        else:
            unit = stat_name
            if median == fastest_vals[0]:
                print(f"  - {name}: baseline ({median:.4f} {unit})")
            else:
                speedup = median / fastest_vals[0]
                print(f"  - {name}: {median:.4f} {unit}  ({speedup:.2f}× better than {fastest_name})")
        print(f"    CV: {cv:.2f}%")

def main():
    if len(sys.argv) != 4:
        print("Usage: python plot-avg.py <folder> <stat_name> <input_description>")
        sys.exit(1)

    folder = sys.argv[1]
    stat_name = sys.argv[2]
    input_desc = sys.argv[3]

    if not os.path.isdir(folder):
        print(f"Error: {folder} is not a directory.")
        sys.exit(1)

    file_median_map = {}

    for filename in sorted(os.listdir(folder)):
        filepath = os.path.join(folder, filename)
        if not os.path.isfile(filepath):
            continue

        if stat_name == "exec-time":
            median, mean, std_dev, cv = parse_exec_time_median_and_cv(filepath)
        else:
            median, mean, std_dev, cv = parse_perf_stat_median_and_cv(filepath, stat_name)

        if median is not None:
            base = os.path.splitext(filename)[0]
            file_median_map[base] = (median, mean, std_dev, cv)

    if not file_median_map:
        print(f"No data found for stat '{stat_name}' in directory '{folder}'")
        sys.exit(1)

    # Sort by median value
    file_median_map = dict(sorted(file_median_map.items(), key=lambda x: x[1][0], reverse=True))
    os.makedirs("plots", exist_ok=True)
    sanitized_input = input_desc.replace(" ", "-")
    output_plot = os.path.join("plots", f"compare_{stat_name}_{sanitized_input}.png")
    plot_medians(file_median_map, stat_name, output_plot, input_desc)
    report_speedups(file_median_map, stat_name)

if __name__ == "__main__":
    main()
