#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$ROOT_DIR"

# Generated HDL/simulation/build outputs.
find . -type d \( \
  -name '.Xil' -o -name '*.cache' -o -name '*.gen' -o -name '*.hw' -o \
  -name '*.ip_user_files' -o -name '*.runs' -o -name '*.sim' -o -name '*.tmp' -o \
  -name '.metadata' -o -name 'RemoteSystemsTempFiles' -o -name 'Debug' -o -name 'Release' -o \
  -name 'target' -o -name 'build' -o -name 'dist' \
\) -prune -exec rm -rf {} +

find . -type f \( \
  -name '*.o' -o -name '*.a' -o -name '*.so' -o -name '*.elf' -o -name '*.exe' -o \
  -name '*.jou' -o -name '*.log' -o -name '*.pb' -o -name '*.wdb' -o -name '*.wcfg' -o \
  -name '*.rpt' -o -name '*.rpx' -o -name '*.dcp' -o -name '*.bit' -o \
  -name '.DS_Store' -o -name '._*' \
\) -delete

# Recreate placeholder directories used by Lab3/sw_x86.
mkdir -p Lab3/sw_x86/target Lab3/sw_x86/out Lab3/sw_x86/perf Lab3/sw_x86/plots
for placeholder in \
  Lab3/sw_x86/target/.gitignore \
  Lab3/sw_x86/out/.gitignore \
  Lab3/sw_x86/perf/.gitignore \
  Lab3/sw_x86/plots/.gitignore; do
  printf '*\n!.gitignore\n' > "$placeholder"
done

echo "Cleaned generated build, simulation, and validation-check outputs."
