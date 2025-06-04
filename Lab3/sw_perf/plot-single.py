import os
import sys
import re
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator, MaxNLocator
import numpy as np

# Function to calculate standard deviation and coefficient of variation
def calculate_variability(values):
    if len(values) < 2:
        return None, None  # Cannot calculate variability with less than 2 points
    mean_value = np.mean(values)
    std_dev = np.std(values)
    coeff_of_variation = std_dev / mean_value if mean_value != 0 else None
    return std_dev, coeff_of_variation

def parse_perf_stat_values(filename, target_stat):
    values = []
    with open(filename, 'r') as f:
        for line in f:
            match = re.match(r"\s*([\d,]+)\s+(\S+)", line)
            if match:
                val_str, stat_name = match.groups()
                if stat_name == target_stat:
                    try:
                        values.append(int(val_str.replace(',', '')))
                    except ValueError:
                        continue
    return values

def parse_exec_times(filename):
    values = []
    pattern = r"execution time.*?([\d.]+)\s*sec"
    with open(filename, 'r') as f:
        for line in f:
            match = re.search(pattern, line)
            if match:
                try:
                    values.append(float(match.group(1)))
                except ValueError:
                    continue
    return values

def plot_stat_values(values, stat_name, filename, input_desc):
    if not values:
        print(f"No values found for stat '{stat_name}' in file '{filename}'")
        return

    avg_val = sum(values) / len(values)
    std_dev, coeff_of_variation = calculate_variability(values)
    median_val = np.median(values)  # Calculate the median
    base_name = os.path.splitext(os.path.basename(filename))[0]

    # Handle units for exec-time
    if stat_name == "exec-time":
        if all(v < 1 for v in values):
            unit = "µs"
            values = [v * 1e6 for v in values]
            avg_val *= 1e6
            median_val *= 1e6  # Convert median to the correct unit
        else:
            unit = "ms"
            values = [v * 1e3 for v in values]
            avg_val *= 1e3
            median_val *= 1e3  # Convert median to the correct unit
    else:
        unit = stat_name

    # Convert std_dev to the same unit as values
    if stat_name == "exec-time":
        if unit == "µs":
            std_dev *= 1e6
        elif unit == "ms":
            std_dev *= 1e3

    x = list(range(1, len(values) + 1))

    plt.figure(figsize=(14, 6))
    plt.plot(x, values, marker='o', linestyle='-', color='skyblue', label='Measurements')
    plt.axhline(avg_val, color='red', linestyle='--', label=f'Average: {avg_val:.2f} {unit}')
    plt.axhline(median_val, color='green', linestyle='-', label=f'Median: {median_val:.2f} {unit}')  # Add median line

    # Title and axis labels
    title_stat = "execution time" if stat_name == "exec-time" else stat_name
    plt.title(f'{title_stat.capitalize()} for {base_name} ({input_desc})', fontsize=16)
    plt.xlabel('Run Number', fontsize=14)
    plt.ylabel(unit, fontsize=14)

    # Use rounded x-axis ticks (e.g. every 10 runs)
    ax = plt.gca()
    ax.xaxis.set_major_locator(MultipleLocator(max(1, len(x) // 10)))
    ax.xaxis.set_major_formatter(plt.FuncFormatter(lambda val, _: f"{int(val)}"))
    plt.yticks(fontsize=12)

    # Annotated variability box
    if std_dev is not None and coeff_of_variation is not None:
        # Show higher precision for std_dev and CV
        if std_dev < 1e-6:  # If stdev is too small to be meaningful, don't show it
            variability_info = f"Coefficient of variation: {coeff_of_variation * 100:.2f}%"  # Display CV as percentage
        else:
            variability_info = f"Standard deviation: {std_dev:.6f} {unit}\nCoefficient of variation: {coeff_of_variation * 100:.2f}%"
        
        ax.text(0.99, 0.91, variability_info, ha='right', va='bottom',
                transform=ax.transAxes,
                bbox=dict(boxstyle="round,pad=0.3", facecolor="lightyellow", edgecolor="gray"),
                fontsize=10)

    plt.legend(fontsize=12)
    plt.tight_layout()

    os.makedirs("plots", exist_ok=True)
    sanitized_input = input_desc.replace(" ", "-")
    out_path = os.path.join("plots", f"{base_name}_{stat_name}_{sanitized_input}.png")
    plt.savefig(out_path)
    plt.close()
    print(f"✅ Saved plot to {out_path}")

def main():
    if len(sys.argv) != 4:
        print("Usage: python plot-single.py <file> <stat_name> <input_description>")
        sys.exit(1)

    filename = sys.argv[1]
    stat_name = sys.argv[2]
    input_desc = sys.argv[3]

    if not os.path.isfile(filename):
        print(f"Error: File '{filename}' does not exist.")
        sys.exit(1)

    if stat_name == "exec-time":
        values = parse_exec_times(filename)
    else:
        values = parse_perf_stat_values(filename, stat_name)

    plot_stat_values(values, stat_name, filename, input_desc)

if __name__ == "__main__":
    main()
