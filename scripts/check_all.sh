#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$ROOT_DIR"

run() {
  local name=$1
  shift
  echo "==> $name"
  "$@"
}

check_iverilog() {
  if ! command -v iverilog >/dev/null 2>&1; then
    echo "WARN: iverilog not found; skipping Verilog validation checks" >&2
    return 0
  fi

  local tmpdir
  tmpdir=$(mktemp -d)

  run "Lab1 step1 floating-point adder testbench elaboration" \
    iverilog -g2012 -o "$tmpdir/lab1_step1.vvp" \
      Lab1/step1/test/testbench.v \
      Lab1/step1/fpadd_single_v3.v Lab1/step1/lzcounter.v Lab1/step1/pipeline.v

  run "Lab1 step2 pipelined floating-point adder testbench elaboration" \
    iverilog -g2012 -o "$tmpdir/lab1_step2.vvp" \
      Lab1/step2/test/testbench.v \
      Lab1/step2/fpadd_pipeline_v3.v Lab1/step1/lzcounter.v Lab1/step1/pipeline.v

  run "Lab1 step3 display system elaboration" \
    iverilog -g2012 -o "$tmpdir/lab1_step3.vvp" \
      Lab1/step3/char_decoder.v Lab1/step3/ssd_driver.v Lab1/step3/fpadd_system.v \
      Lab1/step2/fpadd_pipeline_v3.v Lab1/step1/lzcounter.v Lab1/step1/pipeline.v

  run "Lab1 step4 debounced-input system testbench elaboration" \
    iverilog -g2012 -o "$tmpdir/lab1_step4.vvp" \
      Lab1/step4/test/fpadd_system_tb.v \
      Lab1/step4/fpadd_system.v Lab1/step4/input_debouncer.v Lab1/step4/input_memory.v Lab1/step4/level_to_pulse.v \
      Lab1/step3/char_decoder.v Lab1/step3/ssd_driver.v \
      Lab1/step2/fpadd_pipeline_v3.v Lab1/step1/lzcounter.v Lab1/step1/pipeline.v

  run "Lab2 step3 custom floating-point adder IP elaboration" \
    iverilog -g2012 -o "$tmpdir/lab2_step3_fpadd.vvp" \
      Lab2/step3/fp_add/*.v
}

check_c_sources() {
  if ! command -v gcc >/dev/null 2>&1; then
    echo "WARN: gcc not found; skipping C syntax checks" >&2
    return 0
  fi

  if [[ -f Lab2/step1/simple_arm/VitisWorkspace/simple_arm_platform/ps7_cortexa9_0/standalone_domain/bsp/ps7_cortexa9_0/include/xparameters.h ]]; then
    run "Lab2 ARM application C syntax" \
      gcc -fsyntax-only \
        -I Lab2/step2 \
        -I Lab2/step1/simple_arm/VitisWorkspace/simple_arm_platform/ps7_cortexa9_0/standalone_domain/bsp/ps7_cortexa9_0/include \
        Lab2/step1/simple_arm/input/lab2.c \
        Lab2/step2/timer_util.c
  else
    echo "Skipping Lab2 ARM C syntax checks; they require generated Xilinx BSP headers such as xparameters.h."
  fi

  # Lab3/sw_x86 is intentionally x86/AVX/OpenMP-oriented, and several files
  # depend on x86 intrinsic headers. Compile one representative target only on
  # x86 hosts; other architectures can still validate the HDL sources above.
  local arch
  arch=$(uname -m)
  if [[ "$arch" == x86_64 || "$arch" == i?86 ]]; then
    run "Lab3 x86 software compile" make -C Lab3/sw_x86 TARGET=1-if clean all
  else
    echo "Skipping Lab3 x86 software compile on non-x86 host ($arch); sources use x86/AVX intrinsics."
  fi
}

check_iverilog
check_c_sources

echo "Validation checks completed."
