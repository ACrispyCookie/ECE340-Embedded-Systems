# ECE340 — Embedded Systems

![Verilog](https://img.shields.io/badge/Verilog-RTL-2F74C0)
![C](https://img.shields.io/badge/C-Embedded-00599C)
![C++](https://img.shields.io/badge/C++-HLS%2FHost-00599C)
![Vivado](https://img.shields.io/badge/Xilinx-Vivado-F15B2A)
![Vitis](https://img.shields.io/badge/Xilinx-Vitis-F15B2A)
![Zynq](https://img.shields.io/badge/Zynq-ARM%20%2B%20FPGA-7A3E9D)
![OpenCL/XRT](https://img.shields.io/badge/OpenCL-XRT-2E8B57)

Coursework repository for ECE340 Embedded Systems. The labs combine Verilog RTL, FPGA system integration, bare-metal ARM software, Vivado/Vitis projects, and HLS/OpenCL acceleration work.

## Highlights

- Designed and validated Verilog floating-point adder datapaths, including single-cycle, pipelined, and display-integrated variants.
- Built Zynq ARM/Vivado/Vitis projects for bare-metal GPIO and interrupt-driven embedded software.
- Packaged a custom floating-point adder as AXI-connected IP for FPGA/ARM integration.
- Designed multiple hardware acceleration versions for an LSAL/Smith-Waterman-style workload using C, OpenMP, Vitis HLS, and OpenCL/XRT.

## Requirements

For the lightweight validation checks:

- Linux environment
- `make`
- `gcc`
- `iverilog` for Verilog elaboration checks

For full hardware/project recreation:

- AMD/Xilinx Vivado and Vitis
- Vitis HLS / XRT for the Lab 3 hardware-acceleration projects
- Zynq/FPGA development board matching the course lab setup

On Debian-based systems, the lightweight tools can be installed with:

```bash
sudo apt update
sudo apt install build-essential make iverilog
```

## Build and validation commands

Run representative validation checks:

```bash
make check
```

Clean generated build, simulation, and validation-check files:

```bash
make clean
```

Print the lab inventory:

```bash
make list
```

The root commands are implemented in:

```text
scripts/check_all.sh
scripts/clean_all.sh
scripts/list_assignments.sh
```

## Lab overview

| Directory | Topic | Contents |
|---|---|---|
| `Lab1` | Verilog floating-point adder and FPGA I/O | Single-cycle and pipelined floating-point adder RTL, testbenches, seven-segment display integration, debounced input/memory logic, and XDC constraints. |
| `Lab2/step1/simple_arm` | Zynq bare-metal ARM basics | Vivado block-design project descriptors and a simple Vitis ARM application using GPIO-style board I/O. |
| `Lab2/step1/interrupt_arm` | Interrupt-driven ARM application | Zynq/Vitis application code for interrupt-based control flow. |
| `Lab2/step2` | Timer utility support | Shared C timer utilities and Vitis application support for ARM-side measurements. |
| `Lab2/step3` | Custom floating-point adder IP | Verilog adder/display sources, packaged AXI IP files, driver stubs, and Zynq integration project descriptors. |
| `Lab3/sw_x86` | Software optimization baseline | Multiple C implementations of the LSAL/Smith-Waterman-style workload, timing/perf scripts, plotting helpers, and assembly snapshots. |
| `Lab3/sw_perf` | Performance data | Captured x86 and ARM performance measurements for selected software variants. |
| `Lab3/hw_baseline` | Baseline HLS/OpenCL accelerator | Vitis HLS and host-code sources for the baseline hardware kernel. |
| `Lab3/hw_antidiagonal*` | Optimized HLS/OpenCL accelerators | Anti-diagonal hardware-kernel variants and host-code experiments. |

## Working with Vivado/Vitis projects

Open the Vivado and Vitis project files from the corresponding lab directories, then rebuild or export hardware platforms with the Xilinx tools as required by each lab handout.

## Validation-check scope

`make check` performs checks that are feasible without the full Xilinx toolchain:

- elaborates representative Lab 1 Verilog testbenches with Icarus Verilog,
- elaborates the Lab 2 floating-point-adder Verilog sources,
- skips Lab 2 ARM C checks when generated Xilinx BSP headers are not present, and
- compiles a representative Lab 3 x86 software target only on x86 hosts.

These checks are a quick local sanity check. They are not a replacement for full Vivado synthesis/implementation, Vitis application builds, board programming, or HLS cosimulation.
