#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(git rev-parse --show-toplevel)"
FILES=$(find "$REPO_ROOT/firmware" -type f \( -name '*.c' -o -name '*.h' \))

if [ -z "$FILES" ]; then
    echo "No source files found."
    exit 0
fi

if [ "${1:-check}" = "fix" ]; then
    echo "$FILES" | xargs clang-format -i
    echo "Done."
else
    echo "$FILES" | xargs clang-format --dry-run --Werror 2>&1
fi
