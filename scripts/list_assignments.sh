#!/usr/bin/env bash
set -euo pipefail

cat <<'LIST'
ECE340 Embedded Systems repository inventory

Lab1 — Verilog floating-point adder and FPGA I/O pipeline
  step1: single-cycle floating-point adder and testbench
  step2: pipelined floating-point adder and testbench
  step3: seven-segment display system integration
  step4: debounced/input-memory display system integration

Lab2 — Zynq ARM/Vivado/Vitis integration
  step1/simple_arm: bare-metal ARM GPIO example
  step1/interrupt_arm: interrupt-driven ARM GPIO example
  step2: timer utility and Vitis application support
  step3: floating-point adder custom IP and ARM integration

Lab3 — Software/HLS acceleration workflow
  sw_x86: Smith-Waterman/LSAL software variants and performance tooling
  sw_perf: captured x86/ARM performance data
  hw_baseline: baseline Vitis HLS/OpenCL kernel
  hw_antidiagonal*: optimized anti-diagonal HLS/OpenCL kernel variants
LIST
