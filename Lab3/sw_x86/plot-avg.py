import os
import sys
import re
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import Patch

def parse_perf_stat_avg(filename, target_stat):
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

    return sum(values) / len(values) if values else None

def parse_exec_time_avg(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()

    times = []
    pattern = r"execution time.*?([\d.]+)\s*sec"

    for line in lines:
        match = re.search(pattern, line)
        if match:
            times.append(float(match.group(1)))

    return sum(times) / len(times) if times else None

def plot_averages(file_avg_map, stat_name, output_path, input_desc):
    labels = list(file_avg_map.keys())
    values = list(file_avg_map.values())

    plt.figure(figsize=(12, 6))
    bars = plt.bar(labels, values, color='skyblue', width=0.6)  # Set bar width to 0.6

    plt.title(f'Average {"execution time" if stat_name == "exec-time" else stat_name} per version ({input_desc})', fontsize=16)
    plt.ylabel("Execution Time" if stat_name == "exec-time" else stat_name, fontsize=14)
    plt.xlabel('Version', fontsize=14)
    plt.xticks(rotation=45, ha='right', fontsize=12)

    # Determine the time unit for exec-time
    if stat_name == "exec-time":
        if all(v < 1 for v in values):  # All times are < 1 second
            unit = "µs"
            values = [v * 1e6 for v in values]  # Convert to microseconds
        else:
            unit = "ms"
            values = [v * 1e3 for v in values]  # Convert to milliseconds
    else:
        unit = stat_name  # For other stats, use the name as unit

    # Add value labels with better formatting
    for bar, value in zip(bars, values):
        height = bar.get_height()
        label = f"{value:.2f}" if stat_name != "exec-time" else f"{value:.2f} {unit}"
        plt.text(bar.get_x() + bar.get_width()/2, height, label,
                 ha='center', va='bottom', fontsize=12)

    # Speedup box
    sorted_items = sorted(file_avg_map.items(), key=lambda x: x[1])
    best_name, best_val = sorted_items[0]
    worst_val = sorted_items[-1][1]
    if best_val > 0 and worst_val > 0 and best_val != worst_val:
        speedup = worst_val / best_val
        info = f"Fastest: {best_name}\nSpeedup: {speedup:.2f}×"
    else:
        info = f"Fastest: {best_name}"

    ax = plt.gca()
    ax.text(0.98, 0.98, info, ha='right', va='top',
            transform=ax.transAxes,
            bbox=dict(boxstyle="round,pad=0.3", facecolor="lightyellow", edgecolor="gray"),
            fontsize=10)

    plt.tight_layout()
    plt.savefig(output_path)
    plt.close()
    print(f"✅ Saved plot to {output_path}")

def report_speedups(file_avg_map, stat_name):
    # Higher is better for timing or perf stats (slowest is baseline)
    sorted_items = sorted(file_avg_map.items(), key=lambda x: x[1], reverse=True)
    slowest_name, slowest_value = sorted_items[0]

    # Determine the time unit for exec-time
    if stat_name == "exec-time":
        if slowest_value < 1:
            unit = "µs"
            slowest_value *= 1e6  # Convert to microseconds
        else:
            unit = "ms"
            slowest_value *= 1e3  # Convert to milliseconds
    else:
        unit = stat_name  # For other stats, use the name as unit

    print(f"\n📊 Speedup report (higher is better for '{stat_name}'):")
    for name, val in sorted_items:
        if stat_name == "exec-time":
            if val < 1:
                val *= 1e6  # Convert to microseconds if needed
            else:
                val *= 1e3  # Convert to milliseconds if needed
        if val == slowest_value:
            print(f"  - {name}: baseline ({val:.4f} {unit})")
        else:
            speedup = slowest_value / val
            descriptor = "faster" if stat_name == "exec-time" else "better"
            print(f"  - {name}: {val:.4f} {unit}  ({speedup:.2f}× {descriptor} than {slowest_name})")


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

    file_avg_map = {}

    for filename in sorted(os.listdir(folder)):
        filepath = os.path.join(folder, filename)
        if not os.path.isfile(filepath):
            continue

        if stat_name == "exec-time":
            avg = parse_exec_time_avg(filepath)
        else:
            avg = parse_perf_stat_avg(filepath, stat_name)

        if avg is not None:
            base = os.path.splitext(filename)[0]
            file_avg_map[base] = avg

    if not file_avg_map:
        print(f"No data found for stat '{stat_name}' in directory '{folder}'")
        sys.exit(1)

    
    file_avg_map = dict(sorted(file_avg_map.items(), key=lambda x: x[1], reverse=True))
    os.makedirs("plots", exist_ok=True)
    sanitized_input = input_desc.replace(" ", "-")
    output_plot = os.path.join("plots", f"compare_{stat_name}_{sanitized_input}.png")   
    plot_averages(file_avg_map, stat_name, output_plot, input_desc)
    report_speedups(file_avg_map, stat_name)

if __name__ == "__main__":
    main()
