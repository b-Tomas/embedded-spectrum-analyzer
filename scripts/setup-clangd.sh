#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(git rev-parse --show-toplevel)"

cat > "$REPO_ROOT/.clangd" <<EOF
CompileFlags:
  Add:
    - -xc
    - -std=gnu11
    - -ffreestanding
    - --target=arm-none-eabi
    - -mcpu=cortex-m3
    - -mthumb
    - -D__USE_CMSIS=CMSISv2p00_LPC17xx
    - -D__CODE_RED
    - -DCORE_M3
    - -DDEBUG
    - -I${REPO_ROOT}/firmware/inc
    - -I${REPO_ROOT}/lib/CMSISv2p00_LPC17xx/CMSISv2p00_LPC17xx/inc
    - -I${REPO_ROOT}/lib/CMSISv2p00_LPC17xx/CMSISv2p00_LPC17xx/Drivers/inc
  Remove:
    - -mfpu=*
    - -mfloat-abi=*

Diagnostics:
  Suppress:
    - drv_unknown_argument
EOF

echo "Generated ${REPO_ROOT}/.clangd"
